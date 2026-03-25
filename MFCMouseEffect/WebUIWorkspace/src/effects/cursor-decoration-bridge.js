const CURSOR_DECORATION_STATE_EVENT = 'mfx:cursor-decoration-state';
const CURSOR_DECORATION_CHANGE_EVENT = 'mfx:cursor-decoration-change';

function hasWindow() {
  return typeof window !== 'undefined';
}

export function emitCursorDecorationState(detail) {
  if (!hasWindow()) {
    return;
  }
  window.dispatchEvent(new CustomEvent(CURSOR_DECORATION_STATE_EVENT, {
    detail: detail || {},
  }));
}

export function subscribeCursorDecorationState(listener) {
  if (!hasWindow() || typeof listener !== 'function') {
    return () => {};
  }
  const handler = (event) => listener(event?.detail || {});
  window.addEventListener(CURSOR_DECORATION_STATE_EVENT, handler);
  return () => window.removeEventListener(CURSOR_DECORATION_STATE_EVENT, handler);
}

export function emitCursorDecorationChange(detail) {
  if (!hasWindow()) {
    return;
  }
  window.dispatchEvent(new CustomEvent(CURSOR_DECORATION_CHANGE_EVENT, {
    detail: detail || {},
  }));
}

export function subscribeCursorDecorationChange(listener) {
  if (!hasWindow() || typeof listener !== 'function') {
    return () => {};
  }
  const handler = (event) => listener(event?.detail || {});
  window.addEventListener(CURSOR_DECORATION_CHANGE_EVENT, handler);
  return () => window.removeEventListener(CURSOR_DECORATION_CHANGE_EVENT, handler);
}
