const CURSOR_DECORATION_STATE_EVENT = 'mfx:cursor-decoration-state';
const CURSOR_DECORATION_CHANGE_EVENT = 'mfx:cursor-decoration-change';

function isBrowser() {
  return typeof window !== 'undefined' && typeof window.addEventListener === 'function';
}

export function emitCursorDecorationState(detail) {
  if (!isBrowser()) {
    return;
  }
  window.dispatchEvent(new CustomEvent(CURSOR_DECORATION_STATE_EVENT, {
    detail: detail || {},
  }));
}

export function subscribeCursorDecorationState(listener) {
  if (!isBrowser() || typeof listener !== 'function') {
    return () => {};
  }
  const handler = (event) => listener(event?.detail || {});
  window.addEventListener(CURSOR_DECORATION_STATE_EVENT, handler);
  return () => window.removeEventListener(CURSOR_DECORATION_STATE_EVENT, handler);
}

export function emitCursorDecorationChange(detail) {
  if (!isBrowser()) {
    return;
  }
  window.dispatchEvent(new CustomEvent(CURSOR_DECORATION_CHANGE_EVENT, {
    detail: detail || {},
  }));
}

export function subscribeCursorDecorationChange(listener) {
  if (!isBrowser() || typeof listener !== 'function') {
    return () => {};
  }
  const handler = (event) => listener(event?.detail || {});
  window.addEventListener(CURSOR_DECORATION_CHANGE_EVENT, handler);
  return () => window.removeEventListener(CURSOR_DECORATION_CHANGE_EVENT, handler);
}
