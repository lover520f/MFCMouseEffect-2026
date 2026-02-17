<script>
  import { createEventDispatcher, onDestroy } from 'svelte';
  import TriggerChainEditor from './TriggerChainEditor.svelte';
  import { filterCatalogEntries, normalizeCatalogEntries } from './app-catalog.js';
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
  export let templateValue = '';
  export let templateOptions = [];
  export let texts = {};

  const dispatch = createEventDispatcher();
  let recordingRowId = '';
  let remoteCaptureSessionId = '';
  let remoteCapturePollTimer = 0;
  let appCatalogEntries = [];
  let appCatalogLoaded = false;
  let appCatalogLoading = false;
  let appCatalogError = '';
  let scopeFilePicker = null;
  let scopeFileRowId = '';

  function isCapturing(rowId) {
    return recordingRowId === rowId;
  }

  function clearRemotePollTimer() {
    if (!remoteCapturePollTimer) {
      return;
    }
    window.clearTimeout(remoteCapturePollTimer);
    remoteCapturePollTimer = 0;
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

  function stopRecording(rowId, blurInput = false) {
    if (recordingRowId !== rowId) {
      return;
    }
    recordingRowId = '';
    if (blurInput) {
      const input = document.getElementById(shortcutInputId(rowId));
      if (input) {
        input.blur();
      }
    }
  }

  function scheduleRemotePoll(rowId, sessionId) {
    clearRemotePollTimer();
    remoteCapturePollTimer = window.setTimeout(() => {
      void pollRemoteCapture(rowId, sessionId);
    }, 120);
  }

  async function pollRemoteCapture(rowId, sessionId) {
    if (recordingRowId !== rowId || remoteCaptureSessionId !== sessionId) {
      return;
    }

    try {
      const result = await pollShortcutCapture(sessionId);
      if (recordingRowId !== rowId || remoteCaptureSessionId !== sessionId) {
        return;
      }

      if (result.status === 'captured' && result.shortcut) {
        emitRowChange(rowId, 'keys', result.shortcut);
        remoteCaptureSessionId = '';
        stopRecording(rowId, true);
        return;
      }

      if (result.status === 'expired' || result.status === 'invalid') {
        remoteCaptureSessionId = '';
        stopRecording(rowId, true);
        return;
      }

      scheduleRemotePoll(rowId, sessionId);
    } catch (_error) {
      // Fallback to local keydown capture if remote polling fails.
      remoteCaptureSessionId = '';
      clearRemotePollTimer();
    }
  }

  async function beginRemoteCapture(rowId) {
    try {
      const sessionId = await startShortcutCapture(10000);
      if (!sessionId || recordingRowId !== rowId) {
        return;
      }
      remoteCaptureSessionId = sessionId;
      scheduleRemotePoll(rowId, sessionId);
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
      recordingRowId = '';
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

  function focusShortcutInput(rowId) {
    const input = document.getElementById(shortcutInputId(rowId));
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

    if (!isCapturing(row.id)) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const lowered = `${event.key || ''}`.toLowerCase();
    if (lowered === 'escape') {
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
      emitRowChange(row.id, 'keys', '');
      stopRecording(row.id);
      void endRemoteCapture();
      return;
    }

    const shortcut = shortcutFromKeyboardEvent(event);
    if (!shortcut) {
      return;
    }

    emitRowChange(row.id, 'keys', shortcut);
    stopRecording(row.id);
    void endRemoteCapture();
  }

  function onShortcutInput(row, event) {
    if (isCapturing(row.id)) {
      return;
    }
    emitRowChange(row.id, 'keys', event.currentTarget.value);
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
      120);
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
      appCatalogEntries = normalizeCatalogEntries(apps);
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

  function toggleRecord(rowId) {
    if (recordingRowId === rowId) {
      recordingRowId = '';
      const input = document.getElementById(shortcutInputId(rowId));
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

    recordingRowId = rowId;
    focusShortcutInput(rowId);
    void beginRemoteCapture(rowId);
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

  $: {
    if (recordingRowId) {
      const activeRow = rows.find((item) => item.id === recordingRowId);
      if (activeRow && activeRow.enabled) {
        // Active capture row still valid.
      } else {
        const staleId = recordingRowId;
        recordingRowId = '';
        void endRemoteCapture();
        const input = document.getElementById(shortcutInputId(staleId));
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
    clearRemotePollTimer();
    if (remoteCaptureSessionId) {
      void endRemoteCapture(remoteCaptureSessionId);
    }
  });
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
            <span class="automation-row-head-main">{triggerSummaryForRow(row)}</span>
            <span class="automation-row-head-meta">{scopeSummaryForRow(row)}</span>
            <span class="automation-row-head-meta">{shortcutSummaryForRow(row)}</span>
          </summary>
          <div class="automation-row-body" class:is-scope-all={scopeModeForRow(row) !== 'selected'}>
            <div class="automation-chain-group automation-col">
              <div class="automation-shortcut-head">
                <input
                  id={shortcutInputId(row.id)}
                  class="automation-keys"
                  type="text"
                  disabled={!row.enabled}
                  readonly={isCapturing(row.id)}
                  value={row.keys}
                  placeholder={texts.shortcutPlaceholder}
                  on:keydown={(event) => onShortcutKeydown(row, event)}
                  on:input={(event) => onShortcutInput(row, event)}
                />
                <button
                  class="btn-soft automation-record"
                  class:is-recording={isCapturing(row.id)}
                  type="button"
                  disabled={!row.enabled}
                  on:click={() => toggleRecord(row.id)}
                >
                  {isCapturing(row.id) ? (texts.recordStop || texts.recording) : texts.record}
                </button>
              </div>
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

  <div class="automation-capture-hint" class:is-active={!!recordingRowId}>
    {recordingRowId ? texts.captureHintActive : texts.captureHint}
  </div>

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
    accept=".exe"
    multiple
    bind:this={scopeFilePicker}
    on:change={onScopeFileChange}
  />
</div>
