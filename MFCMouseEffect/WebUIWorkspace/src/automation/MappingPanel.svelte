<script>
  import { createEventDispatcher } from 'svelte';

  export let kind = 'mouse';
  export let rows = [];
  export let options = [];
  export let templateValue = '';
  export let templateOptions = [];
  export let recordingKey = '';
  export let texts = {};

  const dispatch = createEventDispatcher();

  function rowRecordingKey(rowId) {
    return `${kind}:${rowId}`;
  }

  function emitRowChange(rowId, key, value) {
    dispatch('rowchange', { kind, rowId, key, value });
  }

  function emitRemove(rowId) {
    dispatch('remove', { kind, rowId });
  }

  function emitRecord(rowId) {
    dispatch('record', { kind, rowId });
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
        <select
          class="automation-trigger"
          disabled={!row.enabled}
          value={row.trigger}
          on:change={(event) => emitRowChange(row.id, 'trigger', event.currentTarget.value)}
        >
          {#each options as option (option.value)}
            <option value={option.value}>{option.label}</option>
          {/each}
        </select>
        <input
          class="automation-keys"
          type="text"
          disabled={!row.enabled}
          value={row.keys}
          placeholder={texts.shortcutPlaceholder}
          on:input={(event) => emitRowChange(row.id, 'keys', event.currentTarget.value)}
        />
        <button
          class="btn-soft automation-record"
          class:is-recording={recordingKey === rowRecordingKey(row.id)}
          type="button"
          disabled={!row.enabled}
          on:click={() => emitRecord(row.id)}
        >
          {recordingKey === rowRecordingKey(row.id) ? texts.recording : texts.record}
        </button>
        <button class="btn-soft automation-remove" type="button" on:click={() => emitRemove(row.id)}>
          {texts.remove}
        </button>
        <div class="automation-note" class:is-visible={!!row.note}>{row.note}</div>
      </div>
    {/each}
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
