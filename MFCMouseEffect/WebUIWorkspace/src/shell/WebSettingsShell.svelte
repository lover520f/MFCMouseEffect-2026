<script>
  import TopBar from "./TopBar.svelte";
  import SettingsGrid from "./SettingsGrid.svelte";

  export let statusMessage = "";
  export let statusTone = "";
  export let actionsDisabled = false;
  export let onAction = () => {};

  function handleAction(event) {
    const type = event?.detail?.type;
    if (!type) return;
    onAction(type);
  }
</script>

<div class="page-bg" aria-hidden="true">
  <div class="shape shape-a"></div>
  <div class="shape shape-b"></div>
  <div class="shape shape-c"></div>
</div>

<div class="wrap">
  <TopBar
    {actionsDisabled}
    {statusMessage}
    {statusTone}
    on:action={handleAction}
  />

  <div class="settings-shell">
    <aside class="workspace-sidebar" id="workspace_sidebar_mount"></aside>

    <main class="workspace-main">
      <div id="workspace_context_mount"></div>
      <SettingsGrid />
    </main>
  </div>
</div>
