import { syncMountedComponent } from './component-instance.js';

export function createLazyMountBridge(options) {
  const opts = options || {};
  const mountId = typeof opts.mountId === 'string' ? opts.mountId : '';
  const createComponent = typeof opts.createComponent === 'function'
    ? opts.createComponent
    : () => null;
  const initialProps = (opts.initialProps && typeof opts.initialProps === 'object')
    ? opts.initialProps
    : {};

  let latestProps = initialProps;
  let component = null;
  let observer = null;

  function stopObserver() {
    if (!observer) {
      return;
    }
    observer.disconnect();
    observer = null;
  }

  function mountIfNeeded() {
    if (component) {
      return component;
    }
    if (!mountId || typeof document === 'undefined') {
      return null;
    }

    const mountNode = document.getElementById(mountId);
    if (!mountNode) {
      return null;
    }

    try {
      component = createComponent(mountNode, latestProps);
    } catch (_error) {
      mountNode.dataset.mfxMountError = _error instanceof Error ? _error.message : String(_error);
      console.error(`[mfx lazy mount] ${mountId} failed`, _error);
      component = null;
      return null;
    }
    if (!component) {
      return null;
    }

    stopObserver();
    return component;
  }

  function observeMount() {
    if (component || observer || typeof MutationObserver !== 'function') {
      return;
    }
    if (typeof document === 'undefined') {
      return;
    }
    const root = document.body || document.documentElement;
    if (!root) {
      return;
    }
    observer = new MutationObserver(() => {
      mountIfNeeded();
    });
    observer.observe(root, { childList: true, subtree: true });
  }

  function updateProps(nextProps) {
    latestProps = (nextProps && typeof nextProps === 'object')
      ? nextProps
      : {};

    const target = mountIfNeeded();
    if (!target) {
      observeMount();
      return null;
    }
    component = syncMountedComponent(target, document.getElementById(mountId), createComponent, latestProps);
    return component;
  }

  mountIfNeeded();
  if (!component) {
    observeMount();
  }

  return {
    updateProps,
    ensureMounted: mountIfNeeded,
    getComponent: () => component,
  };
}
