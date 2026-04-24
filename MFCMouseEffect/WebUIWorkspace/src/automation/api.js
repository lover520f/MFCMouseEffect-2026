import { createDefaultAutomationState } from './defaults.js';
import { syncMountedComponent } from '../entries/component-instance.js';
import { mountLegacyComponent } from '../entries/legacy-component.js';

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

/**
 * AutomationApi — Svelte 5 compatible.
 *
 * Problem: Svelte 5 compiles `export function read/validate` in AutomationEditor
 * as private class fields. Accessing them via `instance[name]()` throws
 * "Cannot read from private field" in dev (ESM/HMR) mode.
 *
 * Fix: the component receives an `onReady(api)` callback prop. On mount it calls
 * onReady({ read, validate }) with plain JS function references, which are safe
 * to hold and call from outside without touching any private fields.
 */
export function createAutomationApi(Component, mountId) {
  let component = null;
  let mountObserver = null;
  // Externalized method references registered by the component via onReady.
  let registeredRead = null;
  let registeredValidate = null;

  let latestProps = {
    schema: {},
    payloadState: {},
    i18n: {},
  };

  function onReadyCallback(api) {
    registeredRead = (api && typeof api.read === 'function') ? api.read : null;
    registeredValidate = (api && typeof api.validate === 'function') ? api.validate : null;
  }

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

  function buildComponentProps(extraProps) {
    return {
      ...latestProps,
      ...(extraProps || {}),
      onReady: onReadyCallback,
    };
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
    component = mountLegacyComponent(Component, mount, buildComponentProps());
    stopMountObserver();
    return component;
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
      component = syncMountedComponent(
        target,
        mount,
        (mountNode, props) => mountLegacyComponent(Component, mountNode, buildComponentProps(props)),
        nextProps,
      );
    },

    read() {
      if (typeof registeredRead === 'function') {
        return registeredRead();
      }
      return fallbackReadResult(latestProps.payloadState);
    },

    validate() {
      if (typeof registeredValidate === 'function') {
        return registeredValidate();
      }
      return emptyValidationResult();
    },

    syncI18n(i18n) {
      const nextProps = { i18n: i18n || {} };
      const target = ensureComponent(nextProps);
      if (!target) {
        return;
      }
      const mount = document.getElementById(mountId);
      component = syncMountedComponent(
        target,
        mount,
        (mountNode, props) => mountLegacyComponent(Component, mountNode, buildComponentProps(props)),
        nextProps,
      );
    },
  };
}
