<script>
  import { createEventDispatcher } from "svelte";

  export let actionsDisabled = false;
  export let statusMessage = "";
  export let statusTone = "";

  const dispatch = createEventDispatcher();

  function emitAction(type) {
    dispatch("action", { type });
  }
</script>

<div class="top">
  <div class="brand">
    <div class="t" data-i18n="title">MFCMouseEffect Settings</div>
    <div class="s" data-i18n="subtitle">
      Local UI · instant apply · saved to config.json
    </div>
    <div class="hint" data-i18n="hint_apply">
      Change values then click Apply.
    </div>
  </div>

  <div
    class="top-status"
    class:show={!!statusMessage}
    class:warn={statusTone === "warn"}
    class:ok={statusTone === "ok"}
    class:offline={statusTone === "offline"}
    id="status"
    aria-live="polite"
  >
    {statusMessage || ""}
  </div>

  <div class="btns">
    <a
      id="btnStar"
      class="btn-link btn-star"
      href="https://github.com/sqmw/MFCMouseEffect"
      target="_blank"
      rel="noopener"
      data-i18n-title="tip_star"
      title="Open project page"
    >
      &#9733; <span data-i18n="btn_star">Star</span>
    </a>
    <button
      id="btnReload"
      class="btn-soft"
      data-i18n="btn_reload"
      data-i18n-title="tip_reload"
      title="Reload config.json from disk"
      disabled={actionsDisabled}
      on:click={() => emitAction("reload")}
    >
      Reload
    </button>
    <button
      id="btnReset"
      class="btn-danger"
      data-i18n="btn_reset"
      data-i18n-title="tip_reset"
      title="Reset to defaults"
      disabled={actionsDisabled}
      on:click={() => emitAction("reset")}
    >
      Reset
    </button>
    <button
      id="btnStop"
      class="btn-soft"
      data-i18n="btn_stop"
      data-i18n-title="tip_stop"
      title="Stop server (reopen from tray)"
      disabled={actionsDisabled}
      on:click={() => emitAction("stop")}
    >
      Stop
    </button>
    <button
      id="btnSave"
      class="primary"
      data-i18n="btn_apply"
      data-i18n-title="tip_apply"
      title="Apply current form values"
      disabled={actionsDisabled}
      on:click={() => emitAction("save")}
    >
      Apply
    </button>
  </div>
</div>
