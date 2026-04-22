export function updateComponentProps(component, nextProps) {
  if (!component || !nextProps || typeof nextProps !== 'object') {
    return component;
  }
  if (typeof component.$set === 'function') {
    component.$set(nextProps);
    return component;
  }
  return component;
}

export function remountComponent(component, mountNode, createComponent, nextProps) {
  if (!mountNode || typeof createComponent !== 'function') {
    return component;
  }
  if (component && typeof component.$destroy === 'function') {
    try {
      component.$destroy();
    } catch (_error) {
      // Fall through and clear the mount node anyway.
    }
  }
  mountNode.replaceChildren();
  return createComponent(mountNode, nextProps || {});
}

export function syncMountedComponent(component, mountNode, createComponent, nextProps) {
  if (component && typeof component.$set === 'function') {
    component.$set(nextProps || {});
    return component;
  }
  return remountComponent(component, mountNode, createComponent, nextProps);
}
