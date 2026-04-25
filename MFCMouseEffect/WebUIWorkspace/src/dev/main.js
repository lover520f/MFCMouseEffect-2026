import '../../../WebUI/styles.css';

import { loadDevRuntime, renderDevRuntimeError } from './runtime.js';

async function bootDevUi() {
  const runtime = await loadDevRuntime();
  if (!runtime.available) {
    throw new Error(runtime.reason || 'backend runtime not discovered');
  }

  await import('../entries/dialog-main.js');
  await import('../../../WebUI/automation-templates.js');
  await import('../entries/shell-main.js');
  await import('../entries/mouse-companion-main.js');
  await import('../entries/general-main.js');
  await import('../entries/effects-main.js');
  await import('../entries/text-main.js');
  await import('../entries/trail-main.js');
  await import('../entries/input-indicator-main.js');
  await import('../entries/automation-main.js');
  await import('../entries/wasm-main.js');
  await import('../entries/main.js');
  await import('../../../WebUI/i18n.js');
  await import('../../../WebUI/i18n-runtime.js');
  await import('../../../WebUI/web-api.js');
  await import('../../../WebUI/settings-form-input-indicator.js');
  await import('../../../WebUI/settings-form.js');
  await import('../../../WebUI/app-core.js');
  await import('../../../WebUI/app-gesture-debug.js');
  await import('../../../WebUI/app-actions.js');
  await import('../../../WebUI/app.js');
}

bootDevUi().catch((error) => {
  renderDevRuntimeError(error, window.__MFX_DEV_RUNTIME__);
  console.error(error);
});
