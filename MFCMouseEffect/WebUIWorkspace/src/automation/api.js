import { createDefaultAutomationState } from './defaults.js';

function emptyValidationResult() {
  return { ok: true };
}

function fallbackReadResult() {
  return createDefaultAutomationState();
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
      target.$set(nextProps);
    },

    read() {
      return invoke('read', fallbackReadResult);
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
      target.$set(nextProps);
    },
  };
}
