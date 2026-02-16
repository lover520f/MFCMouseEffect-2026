<script>
  import { createEventDispatcher, tick } from 'svelte';

  export let open = false;
  export let mode = 'notice';
  export let title = '';
  export let message = '';
  export let okText = 'OK';
  export let cancelText = 'Cancel';

  const dispatch = createEventDispatcher();
  let okButtonEl;
  let wasOpen = false;

  $: if (open && !wasOpen) {
    wasOpen = true;
    focusPrimaryButton();
  } else if (!open && wasOpen) {
    wasOpen = false;
  }

  async function focusPrimaryButton() {
    await tick();
    if (okButtonEl) okButtonEl.focus();
  }

  function onMaskMouseDown(event) {
    if (event.target === event.currentTarget) {
      dispatch('mask');
    }
  }

  function onCancel() {
    dispatch('cancel');
  }

  function onConfirm() {
    dispatch('confirm');
  }
</script>

{#if open}
  <div class="mfx-modal-mask" role="presentation" on:mousedown={onMaskMouseDown}>
    <div
      class="mfx-modal-card"
      role="dialog"
      aria-modal="true"
      aria-label={title || (mode === 'confirm' ? 'Confirm' : 'Notice')}
    >
      <h4 class="mfx-modal-title">{title || (mode === 'confirm' ? 'Confirm' : 'Notice')}</h4>
      <p class="mfx-modal-text">{message || ''}</p>

      <div class="mfx-modal-actions">
        {#if mode === 'confirm'}
          <button type="button" on:click={onCancel}>{cancelText || 'Cancel'}</button>
        {/if}
        <button class="primary" type="button" bind:this={okButtonEl} on:click={onConfirm}>{okText || 'OK'}</button>
      </div>
    </div>
  </div>
{/if}
