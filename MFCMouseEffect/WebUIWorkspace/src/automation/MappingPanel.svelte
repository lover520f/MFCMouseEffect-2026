<script>
  import { createEventDispatcher, onDestroy } from 'svelte';
  import TriggerChainEditor from './TriggerChainEditor.svelte';
  import {
    pollShortcutCapture,
    startShortcutCapture,
    stopShortcutCapture,
  } from './shortcut-capture-remote.js';
  import { shortcutFromKeyboardEvent } from './shortcuts.js';
  import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

  export let kind = 'mouse';
  export let rows = [];
  export let options = [];
  export let templateValue = '';
  export let templateOptions = [];
  export let texts = {};

  const dispatch = createEventDispatcher();
  let recordingRowId = '';
  let remoteCaptureSessionId = '';
  let remoteCapturePollTimer = 0;

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
      <div class="automation-row" class:is-disabled={!row.enabled} class:is-conflict={row.hasConflict}>
        <input
          class="automation-toggle"
          type="checkbox"
          checked={row.enabled}
          title={texts.enabledTitle}
          on:change={(event) => emitRowChange(row.id, 'enabled', event.currentTarget.checked)}
        />
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
        <button class="btn-soft automation-remove" type="button" on:click={() => emitRemove(row.id)}>
          {texts.remove}
        </button>
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
</div>
