import { createDefaultAutomationState } from './defaults.js';
import { syncMountedComponent } from '../entries/component-instance.js';

function emptyValidationResult() {
  return { ok: true };
}

function fallbackReadResult(payloadState) {
  const fallback = createDefaultAutomationState();
  if (!payloadState || typeof payloadState !== 'object') {
    return fallback;
  }

  const gesture = (payloadState.gesture && typeof payloadState.gesture === 'object')
    ? payloadState.gesture
    : {};

  return {
    enabled: payloadState.enabled === true,
    mouse_mappings: Array.isArray(payloadState.mouse_mappings) ? payloadState.mouse_mappings : [],
    gesture: {
      ...fallback.gesture,
      ...gesture,
      mappings: Array.isArray(gesture.mappings) ? gesture.mappings : [],
    },
  };
}

export function createAutomationApi(Component, mountId) {
  let component = null;
  let mountObserver = null;
  let latestProps = {
    schema: {},
    payloadState: {},
    i18n: {},
  };

  function stopMountObserver() {
    if (!mountObserver) {
      return;
    }
    mountObserver.disconnect();
    mountObserver = null;
  }

  function observeMount() {
    if (component || mountObserver || typeof MutationObserver !== 'function') {
      return;
    }
    const root = document.body || document.documentElement;
    if (!root) {
      return;
    }
    mountObserver = new MutationObserver(() => {
      ensureComponent();
    });
    mountObserver.observe(root, { childList: true, subtree: true });
  }

  function ensureComponent(initialProps) {
    if (initialProps && typeof initialProps === 'object') {
      latestProps = {
        ...latestProps,
        ...initialProps,
      };
    }

    if (component) {
      return component;
    }
    const mount = document.getElementById(mountId);
    if (!mount) {
      observeMount();
      return null;
    }
    component = new Component({
      target: mount,
      props: latestProps,
    });
    stopMountObserver();
    return component;
  }

  function invoke(name, fallback) {
    const target = ensureComponent();
    if (!target || typeof target[name] !== 'function') {
      return fallback();
    }
    return target[name]();
  }

  return {
    render(payload) {
      const nextProps = {
        schema: payload?.schema || {},
        payloadState: payload?.state || {},
        i18n: payload?.i18n || {},
      };
      const target = ensureComponent(nextProps);
      if (!target) {
        return;
      }
      const mount = document.getElementById(mountId);
      component = syncMountedComponent(target, mount, (mountNode, props) => new Component({
        target: mountNode,
        props,
      }), nextProps);
    },

    read() {
      return invoke('read', () => fallbackReadResult(latestProps.payloadState));
    },

    validate() {
      return invoke('validate', emptyValidationResult);
    },

    syncI18n(i18n) {
      const nextProps = { i18n: i18n || {} };
      const target = ensureComponent(nextProps);
      if (!target) {
        return;
      }
      const mount = document.getElementById(mountId);
      component = syncMountedComponent(target, mount, (mountNode, props) => new Component({
        target: mountNode,
        props,
      }), nextProps);
    },
  };
}
