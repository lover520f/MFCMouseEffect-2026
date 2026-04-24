function canAssignComponentProp(component, key) {
  if (!component || !key) {
    return false;
  }
  if (key in component) {
    return true;
  }
  let proto = Object.getPrototypeOf(component);
  while (proto) {
    const descriptor = Object.getOwnPropertyDescriptor(proto, key);
    if (descriptor) {
      return typeof descriptor.set === 'function' || descriptor.writable === true;
    }
    proto = Object.getPrototypeOf(proto);
  }
  return false;
}

export function updateComponentProps(component, nextProps) {
  if (!component || !nextProps || typeof nextProps !== 'object') {
    return false;
  }
  if (typeof component.$set === 'function') {
    component.$set(nextProps);
    return true;
  }

  let assigned = false;
  for (const [key, value] of Object.entries(nextProps)) {
    if (!canAssignComponentProp(component, key)) {
      continue;
    }
    try {
      component[key] = value;
      assigned = true;
    } catch (_error) {
      // Fall through and allow remaining props to sync.
    }
  }
  return assigned;
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
  if (!mountNode || typeof createComponent !== 'function') {
    return component;
  }
  if (!component || mountNode.childElementCount <= 0) {
    return remountComponent(component, mountNode, createComponent, nextProps);
  }
  if (updateComponentProps(component, nextProps || {})) {
    return component;
  }
  return remountComponent(component, mountNode, createComponent, nextProps);
}
