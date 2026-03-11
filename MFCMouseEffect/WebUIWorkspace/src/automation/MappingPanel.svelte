<script>
  import { createEventDispatcher, onDestroy } from 'svelte';
  import GesturePatternEditor from './GesturePatternEditor.svelte';
  import TriggerChainEditor from './TriggerChainEditor.svelte';
  import { filterCatalogEntries, normalizeCatalogEntries } from './app-catalog.js';
  import { isMacosPlatform, isWindowsPlatform, normalizeRuntimePlatform } from './platform.js';
  import {
    pollShortcutCapture,
    readAutomationAppCatalog,
    startShortcutCapture,
    stopShortcutCapture,
  } from './shortcut-capture-remote.js';
  import { shortcutFromKeyboardEvent } from './shortcuts.js';
  import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

  export let kind = 'mouse';
  export let rows = [];
  export let options = [];
  export let scopeOptions = [];
  export let gestureButtonOptions = [];
  export let templateValue = '';
  export let templateOptions = [];
  export let texts = {};
  export let platform = 'windows';

  const CAPTURE_TARGET_KEYS = 'keys';
  const CAPTURE_TARGET_MODIFIERS = 'modifiers';
  const dispatch = createEventDispatcher();
  let recordingRowId = '';
  let recordingTarget = CAPTURE_TARGET_KEYS;
  let remoteCaptureSessionId = '';
  let remoteCapturePollTimer = 0;
  let localCaptureCommitTimer = 0;
  let localCapturePending = null;
  let globalShortcutSuppressionBound = false;
  let appCatalogEntries = [];
  let appCatalogLoaded = false;
  let appCatalogLoading = false;
  let appCatalogError = '';
  let scopeFilePicker = null;
  let scopeFileRowId = '';

  function normalizedPlatform() {
    return normalizeRuntimePlatform(platform);
  }

  function scopeFileAccept() {
    if (isMacosPlatform(platform)) {
      return '.app';
    }
    if (isWindowsPlatform(platform)) {
      return '.exe';
    }
    return '';
  }

  function isCapturing(rowId, target = CAPTURE_TARGET_KEYS) {
    return recordingRowId === rowId && recordingTarget === target;
  }

  function clearRemotePollTimer() {
    if (!remoteCapturePollTimer) {
      return;
    }
    window.clearTimeout(remoteCapturePollTimer);
    remoteCapturePollTimer = 0;
  }

  function clearLocalCapturePending() {
    if (localCaptureCommitTimer) {
      window.clearTimeout(localCaptureCommitTimer);
      localCaptureCommitTimer = 0;
    }
    localCapturePending = null;
  }

  function onGlobalCaptureShortcut(event) {
    if (!recordingRowId) {
      return;
    }
    if (!event) {
      return;
    }
    event.preventDefault();
    if (typeof event.stopImmediatePropagation === 'function') {
      event.stopImmediatePropagation();
    }
    if (typeof event.stopPropagation === 'function') {
      event.stopPropagation();
    }
  }

  function bindGlobalShortcutSuppression() {
    if (globalShortcutSuppressionBound || typeof window === 'undefined') {
      return;
    }
    window.addEventListener('keydown', onGlobalCaptureShortcut, true);
    window.addEventListener('keyup', onGlobalCaptureShortcut, true);
    window.addEventListener('keypress', onGlobalCaptureShortcut, true);
    globalShortcutSuppressionBound = true;
  }

  function unbindGlobalShortcutSuppression() {
    if (!globalShortcutSuppressionBound || typeof window === 'undefined') {
      return;
    }
    window.removeEventListener('keydown', onGlobalCaptureShortcut, true);
    window.removeEventListener('keyup', onGlobalCaptureShortcut, true);
    window.removeEventListener('keypress', onGlobalCaptureShortcut, true);
    globalShortcutSuppressionBound = false;
  }

  function isModifierToken(token) {
    const text = `${token || ''}`.trim().toLowerCase();
    return text === 'ctrl' || text === 'shift' || text === 'alt' || text === 'option' || text === 'cmd' || text === 'command' || text === 'win' || text === 'meta' || text === 'os' || text === 'super';
  }

  function isModifierOnlyShortcut(shortcut) {
    const tokens = `${shortcut || ''}`.split('+').map((item) => `${item || ''}`.trim()).filter((item) => !!item);
    if (tokens.length === 0) {
      return false;
    }
    return tokens.every((token) => isModifierToken(token));
  }

  async function endRemoteCapture(sessionId = remoteCaptureSessionId) {
    clearRemotePollTimer();
    if (!sessionId) {
      return;
    }
    if (remoteCaptureSessionId === sessionId) {
      remoteCaptureSessionId = '';
    }
    try {
      await stopShortcutCapture(sessionId);
    } catch (_error) {
      // Keep UI responsive even if server session already ended.
    }
  }

  function activeCaptureInputId(rowId, target = recordingTarget) {
    if (target === CAPTURE_TARGET_MODIFIERS) {
      return modifierInputId(rowId);
    }
    return shortcutInputId(rowId);
  }

  function stopRecording(rowId, blurInput = false) {
    if (recordingRowId !== rowId) {
      return;
    }
    clearLocalCapturePending();
    const target = recordingTarget;
    recordingRowId = '';
    recordingTarget = CAPTURE_TARGET_KEYS;
    if (blurInput) {
      const input = document.getElementById(activeCaptureInputId(rowId, target));
      if (input) {
        input.blur();
      }
    }
  }

  function scheduleRemotePoll(rowId, target, sessionId) {
    clearRemotePollTimer();
    remoteCapturePollTimer = window.setTimeout(() => {
      void pollRemoteCapture(rowId, target, sessionId);
    }, 120);
  }

  async function pollRemoteCapture(rowId, target, sessionId) {
    if (recordingRowId !== rowId || recordingTarget !== target || remoteCaptureSessionId !== sessionId) {
      return;
    }

    try {
      const result = await pollShortcutCapture(sessionId);
      if (recordingRowId !== rowId || recordingTarget !== target || remoteCaptureSessionId !== sessionId) {
        return;
      }

      if (result.status === 'captured' && result.shortcut) {
        clearLocalCapturePending();
        if (target === CAPTURE_TARGET_MODIFIERS) {
          applyTriggerModifierShortcut(rowId, result.shortcut);
        } else {
          emitRowChange(rowId, 'keys', result.shortcut);
        }
        remoteCaptureSessionId = '';
        stopRecording(rowId, true);
        return;
      }

      if (result.status === 'expired' || result.status === 'invalid') {
        remoteCaptureSessionId = '';
        stopRecording(rowId, true);
        return;
      }

      scheduleRemotePoll(rowId, target, sessionId);
    } catch (_error) {
      // Fallback to local keydown capture if remote polling fails.
      remoteCaptureSessionId = '';
      clearRemotePollTimer();
    }
  }

  async function beginRemoteCapture(rowId, target = CAPTURE_TARGET_KEYS) {
    try {
      const sessionId = await startShortcutCapture(10000);
      if (!sessionId || recordingRowId !== rowId || recordingTarget !== target) {
        return;
      }
      remoteCaptureSessionId = sessionId;
      scheduleRemotePoll(rowId, target, sessionId);
    } catch (_error) {
      // Server capture unavailable. Keep local keydown capture active.
      remoteCaptureSessionId = '';
      clearRemotePollTimer();
    }
  }

  function emitRowChange(rowId, key, value) {
    dispatch('rowchange', { kind, rowId, key, value });
  }

  function emitRemove(rowId) {
    if (recordingRowId === rowId) {
      clearLocalCapturePending();
      recordingRowId = '';
      recordingTarget = CAPTURE_TARGET_KEYS;
      void endRemoteCapture();
    }
    dispatch('remove', { kind, rowId });
  }

  function emitAdd() {
    dispatch('add', { kind });
  }

  function emitTemplateChange(value) {
    dispatch('templatechange', { kind, value });
  }

  function emitApplyTemplate() {
    dispatch('applytemplate', { kind });
  }

  function focusCaptureInput(rowId, target = CAPTURE_TARGET_KEYS) {
    const input = document.getElementById(activeCaptureInputId(rowId, target));
    if (!input) {
      return;
    }
    input.focus();
    input.select();
  }

  function onShortcutKeydown(row, event) {
    if (!row.enabled) {
      return;
    }

    if (!isCapturing(row.id, CAPTURE_TARGET_KEYS)) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const lowered = `${event.key || ''}`.toLowerCase();
    if (lowered === 'escape') {
      clearLocalCapturePending();
      event.currentTarget.blur();
      stopRecording(row.id);
      void endRemoteCapture();
      return;
    }

    if ((lowered === 'backspace' || lowered === 'delete') &&
      !event.ctrlKey &&
      !event.altKey &&
      !event.shiftKey &&
      !event.metaKey) {
      clearLocalCapturePending();
      emitRowChange(row.id, 'keys', '');
      stopRecording(row.id);
      void endRemoteCapture();
      return;
    }

    const shortcut = shortcutFromKeyboardEvent(event, platform);
    if (!shortcut) {
      return;
    }

    if (isModifierOnlyShortcut(shortcut)) {
      clearLocalCapturePending();
      localCapturePending = { rowId: row.id, target: CAPTURE_TARGET_KEYS, shortcut };
      localCaptureCommitTimer = window.setTimeout(() => {
        if (!localCapturePending) {
          return;
        }
        const pending = localCapturePending;
        clearLocalCapturePending();
        emitRowChange(pending.rowId, 'keys', pending.shortcut);
        stopRecording(pending.rowId);
        void endRemoteCapture();
      }, 240);
      return;
    }

    clearLocalCapturePending();
    emitRowChange(row.id, 'keys', shortcut);
    stopRecording(row.id);
    void endRemoteCapture();
  }

  function onShortcutInput(row, event) {
    if (isCapturing(row.id, CAPTURE_TARGET_KEYS)) {
      return;
    }
    emitRowChange(row.id, 'keys', event.currentTarget.value);
  }

  function modifierInputId(rowId) {
    return `auto_trigger_modifiers_${kind}_${rowId}`;
  }

  function parseModifierFlagsFromShortcut(shortcutText) {
    const text = `${shortcutText || ''}`.trim();
    if (!text) {
      return { primary: false, shift: false, alt: false };
    }
    const tokens = text.split('+').map((item) => `${item || ''}`.trim().toLowerCase());
    let primary = false;
    let shift = false;
    let alt = false;
    for (const token of tokens) {
      if (!token) {
        continue;
      }
      if (token === 'ctrl' || token === 'control' || token === 'cmd' || token === 'command' || token === 'meta' || token === 'win' || token === 'windows' || token === 'os' || token === 'super') {
        primary = true;
      } else if (token === 'shift') {
        shift = true;
      } else if (token === 'alt' || token === 'option' || token === 'opt') {
        alt = true;
      }
    }
    return { primary, shift, alt };
  }

  function applyTriggerModifierFlags(rowId, shortcut, emptyMode = 'any') {
    const flags = parseModifierFlagsFromShortcut(shortcut);
    if (flags.primary || flags.shift || flags.alt) {
      emitRowChange(rowId, 'modifiers', {
        mode: 'exact',
        primary: flags.primary,
        shift: flags.shift,
        alt: flags.alt,
      });
      return;
    }
    emitRowChange(rowId, 'modifiers', {
      mode: emptyMode === 'none' ? 'none' : 'any',
      primary: false,
      shift: false,
      alt: false,
    });
  }

  function applyTriggerModifierShortcut(rowId, shortcut) {
    applyTriggerModifierFlags(rowId, shortcut, 'none');
  }

  function normalizedModifiersForRow(row) {
    const source = row?.modifiers || {};
    const mode = `${source.mode || 'any'}`.trim().toLowerCase();
    return {
      mode: mode === 'none' || mode === 'exact' ? mode : 'any',
      primary: !!source.primary,
      shift: !!source.shift,
      alt: !!source.alt,
    };
  }

  function modifierPrimaryLabel() {
    return texts.modifierPrimary || (isMacosPlatform(platform) ? 'Cmd' : 'Ctrl');
  }

  function modifierAltLabel() {
    return texts.modifierAlt || (isMacosPlatform(platform) ? 'Option' : 'Alt');
  }

  function modifierShortcutTextForRow(row) {
    const modifiers = normalizedModifiersForRow(row);
    if (modifiers.mode === 'any') {
      return texts.gestureTriggerModifiersAny || 'Any modifier';
    }
    if (modifiers.mode === 'none') {
      return texts.gestureTriggerModifiersNone || 'No modifier';
    }
    const parts = [];
    if (modifiers.primary) {
      parts.push(modifierPrimaryLabel());
    }
    if (modifiers.shift) {
      parts.push(texts.modifierShift || 'Shift');
    }
    if (modifiers.alt) {
      parts.push(modifierAltLabel());
    }
    return parts.length > 0
      ? parts.join('+')
      : (texts.gestureTriggerModifiersAny || 'Any modifier');
  }

  function modifierInputValueForRow(row) {
    const modifiers = normalizedModifiersForRow(row);
    if (modifiers.mode !== 'exact') {
      return '';
    }
    return modifierShortcutTextForRow(row);
  }

  function modifierInputPlaceholderForRow(row) {
    const modifiers = normalizedModifiersForRow(row);
    if (modifiers.mode === 'none') {
      return texts.gestureTriggerModifiersNonePlaceholder || 'Optional (empty means no modifier)';
    }
    if (modifiers.mode === 'any') {
      return texts.gestureTriggerModifiersAny || 'Any modifier';
    }
    return texts.gestureTriggerShortcutPlaceholder || 'Cmd / Ctrl+Shift';
  }

  function onTriggerModifierKeydown(row, event) {
    if (!row.enabled) {
      return;
    }
    if (!isCapturing(row.id, CAPTURE_TARGET_MODIFIERS)) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const lowered = `${event.key || ''}`.toLowerCase();
    if (lowered === 'escape') {
      clearLocalCapturePending();
      event.currentTarget.blur();
      stopRecording(row.id);
      void endRemoteCapture();
      return;
    }

    if ((lowered === 'backspace' || lowered === 'delete') &&
      !event.ctrlKey &&
      !event.altKey &&
      !event.shiftKey &&
      !event.metaKey) {
      clearLocalCapturePending();
      emitRowChange(row.id, 'modifiers', {
        mode: 'none',
        primary: false,
        shift: false,
        alt: false,
      });
      stopRecording(row.id);
      void endRemoteCapture();
      return;
    }

    const shortcut = shortcutFromKeyboardEvent(event, platform);
    if (!shortcut) {
      return;
    }

    if (isModifierOnlyShortcut(shortcut)) {
      clearLocalCapturePending();
      localCapturePending = { rowId: row.id, target: CAPTURE_TARGET_MODIFIERS, shortcut };
      localCaptureCommitTimer = window.setTimeout(() => {
        if (!localCapturePending) {
          return;
        }
        const pending = localCapturePending;
        clearLocalCapturePending();
        applyTriggerModifierShortcut(pending.rowId, pending.shortcut);
        stopRecording(pending.rowId);
        void endRemoteCapture();
      }, 240);
      return;
    }

    clearLocalCapturePending();
    applyTriggerModifierShortcut(row.id, shortcut);
    stopRecording(row.id);
    void endRemoteCapture();
  }

  function onTriggerModifierInput(row, event) {
    if (isCapturing(row.id, CAPTURE_TARGET_MODIFIERS)) {
      return;
    }
    applyTriggerModifierFlags(row.id, event.currentTarget.value, 'none');
  }

  function chainForRow(row) {
    return normalizeTriggerChain(row?.triggerChain || row?.trigger || '', options, options[0]?.value || '');
  }

  function triggerValueFromEvent(event, fallbackTrigger) {
    const detail = event?.detail || {};
    const fallback = `${fallbackTrigger || options[0]?.value || ''}`;

    if (detail.value !== undefined && detail.value !== null) {
      return serializeTriggerChain(detail.value, options, fallback);
    }
    if (detail.chain !== undefined && detail.chain !== null) {
      return serializeTriggerChain(detail.chain, options, fallback);
    }

    if (event?.target && event.target.value !== undefined) {
      return serializeTriggerChain(event.target.value, options, fallback);
    }

    return '';
  }

  function onChainChange(row, event) {
    const nextTrigger = triggerValueFromEvent(event, row?.trigger || row?.triggerChain || '');
    if (!nextTrigger) {
      return;
    }
    emitRowChange(row.id, 'triggerChain', nextTrigger);
  }

  function scopeModeForRow(row) {
    const mode = `${row?.appScopeMode || row?.appScopeType || 'all'}`.trim().toLowerCase();
    return mode === 'selected' || mode === 'process' ? 'selected' : 'all';
  }

  function scopeAppsForRow(row) {
    return Array.isArray(row?.appScopeApps) ? row.appScopeApps : [];
  }

  function scopeDraftForRow(row) {
    return `${row?.appScopeDraft || ''}`;
  }

  function onScopeModeChange(row, event) {
    emitRowChange(row.id, 'appScopeMode', event.currentTarget.value);
  }

  function onScopeDraftInput(row, event) {
    emitRowChange(row.id, 'appScopeDraft', event.currentTarget.value);
  }

  function onScopeDraftKeydown(row, event) {
    if (event.key !== 'Enter') {
      return;
    }
    event.preventDefault();
    const entries = catalogEntriesForRow(row);
    if (entries.length === 0) {
      return;
    }
    addCatalogScopeApp(row, entries[0].exe);
    emitRowChange(row.id, 'appScopeDraft', '');
  }

  function removeScopeApp(row, app) {
    emitRowChange(row.id, 'appScopeRemove', app);
  }

  function catalogEntriesForRow(row) {
    return filterCatalogEntries(
      appCatalogEntries,
      scopeDraftForRow(row),
      scopeAppsForRow(row),
      120,
      normalizedPlatform());
  }

  function catalogMetaText(entry) {
    const exe = `${entry?.exe || ''}`.trim();
    const source = `${entry?.source || ''}`.trim().toLowerCase();
    if (!source) {
      return exe;
    }
    return `${exe} (${source})`;
  }

  function addCatalogScopeApp(row, processName) {
    if (!row || !row.id || !row.enabled) {
      return;
    }
    emitRowChange(row.id, 'appScopeAdd', processName);
  }

  function onScopeFilePick(row) {
    if (!row || !row.id || !row.enabled || !scopeFilePicker) {
      return;
    }
    scopeFileRowId = row.id;
    scopeFilePicker.value = '';
    scopeFilePicker.click();
  }

  function onScopeFileChange(event) {
    const input = event.currentTarget;
    const rowId = scopeFileRowId;
    scopeFileRowId = '';
    const files = Array.from(input?.files || []);
    if (!rowId || files.length === 0) {
      if (input) {
        input.value = '';
      }
      return;
    }

    for (const file of files) {
      const name = `${file?.name || ''}`.trim();
      if (!name) {
        continue;
      }
      emitRowChange(rowId, 'appScopeAdd', name);
    }
    if (input) {
      input.value = '';
    }
  }

  async function loadAppCatalog(force = false) {
    if (appCatalogLoading) {
      return;
    }
    appCatalogLoading = true;
    appCatalogError = '';
    try {
      const apps = await readAutomationAppCatalog(force);
      appCatalogEntries = normalizeCatalogEntries(apps, normalizedPlatform());
      appCatalogLoaded = true;
    } catch (_error) {
      appCatalogError = texts.scopeCatalogLoadFailed || 'Failed to load app list.';
      if (!appCatalogLoaded) {
        appCatalogEntries = [];
      }
    } finally {
      appCatalogLoading = false;
    }
  }

  function onRefreshAppCatalog() {
    void loadAppCatalog(true);
  }

  function toggleRecord(rowId, target = CAPTURE_TARGET_KEYS) {
    if (recordingRowId === rowId && recordingTarget === target) {
      clearLocalCapturePending();
      recordingRowId = '';
      recordingTarget = CAPTURE_TARGET_KEYS;
      const input = document.getElementById(activeCaptureInputId(rowId, target));
      if (input) {
        input.blur();
      }
      void endRemoteCapture();
      return;
    }

    if (recordingRowId && recordingRowId !== rowId) {
      stopRecording(recordingRowId, true);
    }
    if (remoteCaptureSessionId) {
      void endRemoteCapture(remoteCaptureSessionId);
    }

    clearLocalCapturePending();
    recordingRowId = rowId;
    recordingTarget = target;
    focusCaptureInput(rowId, target);
    void beginRemoteCapture(rowId, target);
  }

  function shortcutInputId(rowId) {
    return `auto_keys_${kind}_${rowId}`;
  }

  function onRowToggle(rowId, event) {
    const details = event?.currentTarget;
    if (!details || details.open) {
      return;
    }
    if (recordingRowId === rowId) {
      clearLocalCapturePending();
      stopRecording(rowId, true);
      void endRemoteCapture();
    }
  }

  function triggerLabel(value) {
    const normalized = `${value || ''}`.trim();
    if (!normalized) {
      return '';
    }
    for (const option of options || []) {
      if (`${option?.value || ''}`.trim() === normalized) {
        return `${option?.label || normalized}`.trim() || normalized;
      }
    }
    return normalized;
  }

  function triggerSummaryForRow(row) {
    const labels = chainForRow(row)
      .map((value) => triggerLabel(value))
      .filter((value) => !!value);
    if (labels.length === 0) {
      return '-';
    }
    return labels.join(' -> ');
  }

  function scopeSummaryForRow(row) {
    if (scopeModeForRow(row) === 'all') {
      return texts.scopeAllLabel || 'All Apps';
    }
    const apps = scopeAppsForRow(row);
    if (apps.length === 0) {
      return texts.scopeSelectedEmpty || 'No app selected';
    }
    if (apps.length === 1) {
      return apps[0];
    }
    return `${apps[0]} +${apps.length - 1}`;
  }

  function shortcutSummaryForRow(row) {
    const keys = `${row?.keys || ''}`.trim();
    if (keys) {
      return keys;
    }
    return texts.shortcutEmpty || 'No shortcut';
  }

  function gestureButtonForRow(row) {
    const value = `${row?.triggerButton || 'left'}`.trim().toLowerCase();
    if (value === 'left' || value === 'middle' || value === 'right') {
      return value;
    }
    return 'left';
  }

  function gestureButtonLabel(value) {
    const normalized = `${value || ''}`.trim();
    if (!normalized) {
      return '';
    }
    for (const option of gestureButtonOptions || []) {
      if (`${option?.value || ''}`.trim() === normalized) {
        return `${option?.label || normalized}`.trim() || normalized;
      }
    }
    return normalized;
  }

  function gestureSummaryForRow(row) {
    if (kind !== 'gesture') {
      return triggerSummaryForRow(row);
    }
    const mode = `${row?.gesturePattern?.mode || 'preset'}`.trim().toLowerCase();
    if (mode === 'custom') {
      const thresholdRaw = row?.gesturePattern?.matchThresholdPercent !== undefined
        ? row.gesturePattern.matchThresholdPercent
        : row?.gesturePattern?.match_threshold_percent;
      const threshold = Number(thresholdRaw);
      const percent = Number.isFinite(threshold)
        ? Math.max(50, Math.min(95, Math.round(threshold)))
        : 75;
      return `${gestureButtonLabel(gestureButtonForRow(row))} · ${texts.gestureModeCustom || 'Custom Draw'} ${percent}%`;
    }
    return `${gestureButtonLabel(gestureButtonForRow(row))} · ${triggerSummaryForRow(row)}`;
  }

  $: {
    if (recordingRowId) {
      const activeRow = rows.find((item) => item.id === recordingRowId);
      if (activeRow && activeRow.enabled) {
        // Active capture row still valid.
      } else {
        const staleId = recordingRowId;
        const staleTarget = recordingTarget;
        clearLocalCapturePending();
        recordingRowId = '';
        recordingTarget = CAPTURE_TARGET_KEYS;
        void endRemoteCapture();
        const input = document.getElementById(activeCaptureInputId(staleId, staleTarget));
        if (input) {
          input.blur();
        }
      }
    }
  }

  $: {
    const hasSelectedScopeRow = rows.some((row) => scopeModeForRow(row) === 'selected');
    if (hasSelectedScopeRow && !appCatalogLoaded && !appCatalogLoading) {
      void loadAppCatalog(false);
    }
  }

  onDestroy(() => {
    clearLocalCapturePending();
    unbindGlobalShortcutSuppression();
    clearRemotePollTimer();
    if (remoteCaptureSessionId) {
      void endRemoteCapture(remoteCaptureSessionId);
    }
  });

  $: {
    if (recordingRowId) {
      bindGlobalShortcutSuppression();
    } else {
      unbindGlobalShortcutSuppression();
    }
  }
</script>

<div class="automation-panel">
  <div class="automation-list">
    {#if rows.length === 0}
      <div class="automation-empty">{texts.empty}</div>
    {/if}
    {#each rows as row (row.id)}
      <div
        class="automation-row"
        class:is-disabled={!row.enabled}
        class:is-conflict={row.hasConflict}
      >
        <button
          class="btn-soft automation-remove-corner"
          type="button"
          on:click={() => emitRemove(row.id)}
          title={texts.remove}
          aria-label={texts.remove}
        >
          x
        </button>
        <input
          class="automation-toggle"
          type="checkbox"
          checked={row.enabled}
          title={texts.enabledTitle}
          on:change={(event) => emitRowChange(row.id, 'enabled', event.currentTarget.checked)}
        />
        <details class="automation-collapse" on:toggle={(event) => onRowToggle(row.id, event)}>
          <summary
            class="automation-row-head"
            title={texts.expand || 'Expand'}
            aria-label={texts.expand || 'Expand'}
          >
            <span class="automation-row-head-icon" aria-hidden="true"></span>
            <span class="automation-row-head-main">{gestureSummaryForRow(row)}</span>
            <span class="automation-row-head-badges">
              <span class="automation-row-head-meta">{scopeSummaryForRow(row)}</span>
              {#if kind === 'gesture'}
                <span class="automation-row-head-meta">{modifierShortcutTextForRow(row)}</span>
              {/if}
              <span class="automation-row-head-meta">{shortcutSummaryForRow(row)}</span>
            </span>
          </summary>
          <div class="automation-row-body" class:is-scope-all={scopeModeForRow(row) !== 'selected'}>
            <div class="automation-chain-group automation-col">
              {#if kind === 'gesture'}
                <div class="automation-shortcut-label">{texts.gestureTriggerShortcutLabel}</div>
                <div class="automation-shortcut-head">
                  <input
                    id={modifierInputId(row.id)}
                    class="automation-keys"
                    type="text"
                    disabled={!row.enabled}
                    readonly={isCapturing(row.id, CAPTURE_TARGET_MODIFIERS)}
                    value={modifierInputValueForRow(row)}
                    placeholder={modifierInputPlaceholderForRow(row)}
                    on:keydown={(event) => onTriggerModifierKeydown(row, event)}
                    on:input={(event) => onTriggerModifierInput(row, event)}
                  />
                  <button
                    class="btn-soft automation-record"
                    class:is-recording={isCapturing(row.id, CAPTURE_TARGET_MODIFIERS)}
                    type="button"
                    disabled={!row.enabled}
                    on:click={() => toggleRecord(row.id, CAPTURE_TARGET_MODIFIERS)}
                  >
                    {isCapturing(row.id, CAPTURE_TARGET_MODIFIERS) ? (texts.recordStop || texts.recording) : texts.record}
                  </button>
                </div>
                <div class="automation-shortcut-label">{texts.gestureOutputShortcutLabel || texts.shortcutLabel}</div>
              {/if}
              <div class="automation-shortcut-head">
                <input
                  id={shortcutInputId(row.id)}
                  class="automation-keys"
                  type="text"
                  disabled={!row.enabled}
                  readonly={isCapturing(row.id, CAPTURE_TARGET_KEYS)}
                  value={row.keys}
                  placeholder={texts.shortcutPlaceholder}
                  on:keydown={(event) => onShortcutKeydown(row, event)}
                  on:input={(event) => onShortcutInput(row, event)}
                />
                <button
                  class="btn-soft automation-record"
                  class:is-recording={isCapturing(row.id, CAPTURE_TARGET_KEYS)}
                  type="button"
                  disabled={!row.enabled}
                  on:click={() => toggleRecord(row.id, CAPTURE_TARGET_KEYS)}
                >
                  {isCapturing(row.id, CAPTURE_TARGET_KEYS) ? (texts.recordStop || texts.recording) : texts.record}
                </button>
              </div>
              {#if kind === 'gesture'}
                <div class="automation-gesture-trigger-group">
                  <div class="automation-modifier-label">{texts.gestureTriggerButtonLabel}</div>
                  <select
                    class="automation-modifier-mode"
                    disabled={!row.enabled}
                    value={gestureButtonForRow(row)}
                    on:change={(event) => emitRowChange(row.id, 'triggerButton', event.currentTarget.value)}
                  >
                    {#each gestureButtonOptions as option (option.value)}
                      <option value={option.value}>{option.label}</option>
                    {/each}
                  </select>
                </div>
                <GesturePatternEditor
                  {row}
                  options={options}
                  disabled={!row.enabled}
                  texts={{
                    title: texts.gesturePatternTitle,
                    modePreset: texts.gestureModePreset,
                    modeCustom: texts.gestureModeCustom,
                    preset: texts.gesturePatternPreset,
                    custom: texts.gesturePatternCustom,
                    threshold: texts.gestureThreshold,
                    canvasEmpty: texts.gestureCanvasEmpty,
                    canvasClear: texts.gestureCanvasClear,
                    canvasUndo: texts.gestureCanvasUndo,
                    canvasGuide: texts.gestureCanvasGuide,
                    canvasLimitHint: texts.gestureCanvasLimitHint,
                    canvasLimitBadge: texts.gestureCanvasLimitBadge,
                    canvasLimitReached: texts.gestureCanvasLimitReached,
                    canvasStrokeCount: texts.gestureCanvasStrokeCount,
                    canvasPointUnit: texts.gestureCanvasPointUnit,
                    canvasNoDirection: texts.gestureCanvasNoDirection,
                  }}
                  on:triggerchange={(event) => emitRowChange(row.id, 'triggerChain', event.detail?.trigger)}
                  on:change={(event) => emitRowChange(row.id, 'gesturePattern', event.detail?.gesturePattern)}
                />
              {:else}
                <TriggerChainEditor
                  value={chainForRow(row)}
                  options={options}
                  disabled={!row.enabled}
                  texts={{
                    addNode: texts.addChainNode,
                    removeNode: texts.removeChainNode,
                    chainJoiner: texts.chainJoiner,
                  }}
                  on:chainchange={(event) => onChainChange(row, event)}
                />
              {/if}
            </div>
            <div class="automation-scope-group automation-col">
              <select
                class="automation-scope-select"
                disabled={!row.enabled}
                value={scopeModeForRow(row)}
                on:change={(event) => onScopeModeChange(row, event)}
              >
                {#each scopeOptions as option (option.value)}
                  <option value={option.value}>{option.label}</option>
                {/each}
              </select>
              {#if scopeModeForRow(row) === 'selected'}
                <div class="automation-scope-chip-list">
                  {#each scopeAppsForRow(row) as app (app)}
                    <span class="automation-scope-chip">
                      <span>{app}</span>
                      <button
                        type="button"
                        class="automation-scope-chip-remove"
                        disabled={!row.enabled}
                        on:click={() => removeScopeApp(row, app)}
                      >
                        x
                      </button>
                    </span>
                  {/each}
                </div>
              {/if}
            </div>
            {#if scopeModeForRow(row) === 'selected'}
              <div class="automation-shortcut-pane automation-col">
                <input
                  class="automation-scope-app"
                  type="text"
                  disabled={!row.enabled}
                  value={scopeDraftForRow(row)}
                  placeholder={texts.scopeSearchPlaceholder || texts.scopeAppPlaceholder}
                  on:keydown={(event) => onScopeDraftKeydown(row, event)}
                  on:input={(event) => onScopeDraftInput(row, event)}
                />
                <div class="automation-shortcut-scope">
                  <div class="automation-scope-tools">
                    <button
                      class="btn-soft automation-scope-refresh"
                      type="button"
                      disabled={!row.enabled || appCatalogLoading}
                      on:click={onRefreshAppCatalog}
                    >
                      {appCatalogLoading ? texts.scopeRefreshingCatalog : texts.scopeRefreshCatalog}
                    </button>
                    <button
                      class="btn-soft automation-scope-file"
                      type="button"
                      disabled={!row.enabled}
                      on:click={() => onScopeFilePick(row)}
                    >
                      {texts.scopePickFromFile}
                    </button>
                  </div>
                  <div class="automation-scope-catalog">
                    {#if appCatalogLoading}
                      <div class="automation-scope-catalog-state">{texts.scopeCatalogLoading}</div>
                    {:else if appCatalogError}
                      <div class="automation-scope-catalog-state is-error">{appCatalogError}</div>
                    {:else}
                      {#if catalogEntriesForRow(row).length === 0}
                        <div class="automation-scope-catalog-state">{texts.scopeCatalogEmpty}</div>
                      {:else}
                        {#each catalogEntriesForRow(row) as app (app.exe)}
                          <button
                            type="button"
                            class="automation-scope-catalog-item"
                            disabled={!row.enabled}
                            on:click={() => addCatalogScopeApp(row, app.exe)}
                          >
                            <span class="automation-scope-catalog-label">{app.label}</span>
                            <span
                              class="automation-scope-catalog-meta"
                              title={catalogMetaText(app)}
                              aria-label={catalogMetaText(app)}
                            >
                              i
                            </span>
                          </button>
                        {/each}
                      {/if}
                    {/if}
                  </div>
                </div>
              </div>
            {/if}
          </div>
        </details>
        <div class="automation-note" class:is-visible={!!row.note}>{row.note}</div>
      </div>
    {/each}
  </div>

  {#if recordingRowId}
    <div class="automation-capture-hint is-active">
      {recordingTarget === CAPTURE_TARGET_MODIFIERS
        ? (texts.captureHintModifierActive || texts.captureHintActive)
        : texts.captureHintActive}
    </div>
  {/if}

  <div class="automation-actions">
    <button class="btn-soft" type="button" on:click={emitAdd}>
      {texts.add}
    </button>
    <select
      class="automation-template-select"
      value={templateValue}
      title={texts.templateTitle}
      on:change={(event) => emitTemplateChange(event.currentTarget.value)}
    >
      <option value="">{texts.templateNone}</option>
      {#each templateOptions as option (option.id)}
        <option value={option.id}>{option.label}</option>
      {/each}
    </select>
    <button class="btn-soft" type="button" disabled={!templateValue} on:click={emitApplyTemplate}>
      {texts.applyTemplate}
    </button>
  </div>

  <input
    class="automation-scope-file-input"
    type="file"
    accept={scopeFileAccept()}
    multiple
    bind:this={scopeFilePicker}
    on:change={onScopeFileChange}
  />
</div>
