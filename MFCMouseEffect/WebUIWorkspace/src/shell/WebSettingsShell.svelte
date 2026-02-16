<script>
  import TopBar from './TopBar.svelte';
  import SettingsGrid from './SettingsGrid.svelte';

  export let statusMessage = '';
  export let statusTone = '';
  export let actionsDisabled = false;
  export let onAction = () => {};

  function buildStatusClass(message, tone) {
    let cls = 'status';
    if (!message) return cls;
    cls += ' show';
    if (tone === 'warn') cls += ' warn';
    if (tone === 'ok') cls += ' ok';
    if (tone === 'offline') cls += ' offline';
    return cls;
  }

  function handleAction(event) {
    const type = event?.detail?.type;
    if (!type) return;
    onAction(type);
  }

  $: statusClass = buildStatusClass(statusMessage, statusTone);
</script>

<div class="page-bg" aria-hidden="true">
  <div class="shape shape-a"></div>
  <div class="shape shape-b"></div>
  <div class="shape shape-c"></div>
</div>

<div class={statusClass} id="status" aria-live="polite">{statusMessage || ''}</div>

<div class="wrap">
  <TopBar {actionsDisabled} on:action={handleAction} />

  <div class="settings-shell">
    <aside class="workspace-sidebar" id="workspace_sidebar_mount"></aside>

    <main class="workspace-main">
      <SettingsGrid />
    </main>
  </div>
</div>
