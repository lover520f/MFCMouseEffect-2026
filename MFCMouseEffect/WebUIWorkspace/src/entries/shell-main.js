import WebSettingsShell from '../shell/WebSettingsShell.svelte';

const mountNode = document.getElementById('web_settings_shell_mount');
const actionListeners = [];

function notifyAction(type) {
  if (!type) return;
  for (const listener of actionListeners) {
    try {
      listener(type);
    } catch (_error) {
      // Keep notifying remaining listeners.
    }
  }
}

function addActionListener(listener) {
  if (typeof listener !== 'function') return () => {};
  actionListeners.push(listener);
  return () => {
    const index = actionListeners.indexOf(listener);
    if (index >= 0) actionListeners.splice(index, 1);
  };
}

let component = null;
if (mountNode) {
  component = new WebSettingsShell({
    target: mountNode,
    props: {
      statusMessage: '',
      statusTone: '',
      actionsDisabled: false,
      onAction: notifyAction,
    },
  });
}

function setStatus(message, tone) {
  if (!component) return;
  component.$set({
    statusMessage: message || '',
    statusTone: tone || '',
  });
}

function setActionsEnabled(enabled) {
  if (!component) return;
  component.$set({
    actionsDisabled: !enabled,
  });
}

window.MfxWebShell = {
  onAction: addActionListener,
  setStatus,
  setActionsEnabled,
};
