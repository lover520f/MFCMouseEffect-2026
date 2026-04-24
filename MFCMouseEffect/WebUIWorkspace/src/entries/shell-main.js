import WebSettingsShell from '../shell/WebSettingsShell.svelte';
import { mountLegacyComponent } from './legacy-component.js';

const mountNode = document.getElementById('web_settings_shell_mount');
const actionListeners = [];
const shellState = {
  statusMessage: '',
  statusTone: '',
  actionsDisabled: false,
};

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
function createShellComponent(target, props) {
  return mountLegacyComponent(WebSettingsShell, target, props);
}

function syncShellDom() {
  const statusNode = document.getElementById('status');
  const statusTextNode = statusNode?.querySelector('.top-status__text') || null;
  if (statusNode) {
    const show = !!shellState.statusMessage;
    statusNode.className = `top-status${show ? ' show' : ''}${shellState.statusTone ? ` ${shellState.statusTone}` : ''}`;
  }
  if (statusTextNode) {
    statusTextNode.textContent = shellState.statusMessage || '';
  }
  for (const id of ['btnReset', 'btnStop', 'btnReload', 'btnSave']) {
    const node = document.getElementById(id);
    if (node) {
      node.disabled = !!shellState.actionsDisabled;
    }
  }
}
if (mountNode) {
  component = createShellComponent(mountNode, {
    statusMessage: '',
    statusTone: '',
    actionsDisabled: false,
    onAction: notifyAction,
  });
}

function setStatus(message, tone) {
  if (!component) return;
  shellState.statusMessage = message || '';
  shellState.statusTone = tone || '';
  if (typeof component.$set === 'function') {
    component.$set({
      statusMessage: shellState.statusMessage,
      statusTone: shellState.statusTone,
    });
    return;
  }
  syncShellDom();
}

function setActionsEnabled(enabled) {
  if (!component) return;
  shellState.actionsDisabled = !enabled;
  if (typeof component.$set === 'function') {
    component.$set({
      actionsDisabled: shellState.actionsDisabled,
    });
    return;
  }
  syncShellDom();
}

window.MfxWebShell = {
  onAction: addActionListener,
  setStatus,
  setActionsEnabled,
};
