import { createClassComponent } from 'svelte/legacy';

export function mountLegacyComponent(Component, target, props) {
  return createClassComponent({
    component: Component,
    target,
    props: props || {},
  });
}
