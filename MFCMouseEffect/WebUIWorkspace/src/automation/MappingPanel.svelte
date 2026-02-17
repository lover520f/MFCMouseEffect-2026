<script>
  import { createEventDispatcher } from 'svelte';
  import TriggerChainEditor from './TriggerChainEditor.svelte';
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

  function isCapturing(rowId) {
    return recordingRowId === rowId;
  }

  function emitRowChange(rowId, key, value) {
    dispatch('rowchange', { kind, rowId, key, value });
  }

  function emitRemove(rowId) {
    if (recordingRowId === rowId) {
      recordingRowId = '';
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

  function onShortcutBlur(rowId) {
    if (recordingRowId === rowId) {
      recordingRowId = '';
    }
  }

  function onShortcutKeydown(row, event) {
    if (!row.enabled) {
      return;
    }

    const lowered = `${event.key || ''}`.toLowerCase();
    if (lowered === 'escape') {
      event.preventDefault();
      event.currentTarget.blur();
      if (recordingRowId === row.id) {
        recordingRowId = '';
      }
      return;
    }

    if ((lowered === 'backspace' || lowered === 'delete') &&
      !event.ctrlKey &&
      !event.altKey &&
      !event.shiftKey &&
      !event.metaKey) {
      event.preventDefault();
      event.stopPropagation();
      emitRowChange(row.id, 'keys', '');
      if (recordingRowId === row.id) {
        recordingRowId = '';
      }
      return;
    }

    const shortcut = shortcutFromKeyboardEvent(event);
    if (!shortcut) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();
    emitRowChange(row.id, 'keys', shortcut);
    if (recordingRowId === row.id) {
      recordingRowId = '';
    }
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
      return;
    }

    recordingRowId = rowId;
    focusShortcutInput(rowId);
  }

  function shortcutInputId(rowId) {
    return `auto_keys_${kind}_${rowId}`;
  }
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
          value={row.keys}
          placeholder={texts.shortcutPlaceholder}
          on:blur={() => onShortcutBlur(row.id)}
          on:keydown={(event) => onShortcutKeydown(row, event)}
          on:input={(event) => emitRowChange(row.id, 'keys', event.currentTarget.value)}
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
