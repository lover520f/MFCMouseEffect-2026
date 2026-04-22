(() => {
  const GESTURE_DEBUG_POLL_MS_IDLE = 180;
  const GESTURE_DEBUG_POLL_MS_ACTIVE = 66;
  const GESTURE_DEBUG_POLL_MS_IDLE_DEV = 1200;
  const GESTURE_DEBUG_POLL_MS_ACTIVE_DEV = 800;

  let core = null;
  let gestureDebugPollTimer = 0;
  let gestureDebugPollInFlight = false;

  function init(appCore) {
    core = appCore;
    return api;
  }

  function ensureCore() {
    if (!core) {
      throw new Error('app-core unavailable');
    }
    return core;
  }

  function isSourceDevMode() {
    return !!(typeof window !== 'undefined' && window.__MFX_DEV_RUNTIME__);
  }

  function syncWorkspaceGestureRouteStatus(routeStatus) {
    if (!window.MfxSectionWorkspace || typeof window.MfxSectionWorkspace.syncRuntimeState !== 'function') {
      return;
    }
    try {
      window.MfxSectionWorkspace.syncRuntimeState({
        platform: ensureCore().currentRuntimePlatform(),
        input_automation_gesture_route_status: ensureCore().isObject(routeStatus) ? routeStatus : null,
      });
    } catch (_error) {
      // Keep runtime state sync best-effort during diagnostics polling.
    }
  }

  function hasGestureRouteDiagnosticsPayload() {
    const state = ensureCore().getCachedState();
    return ensureCore().isObject(state?.input_automation_gesture_route_status);
  }

  function shouldRunGestureDebugPolling() {
    return (
      ensureCore().getConnectionState() === 'online' &&
      ensureCore().getHasRenderedSettings() &&
      hasGestureRouteDiagnosticsPayload());
  }

  function clear() {
    if (!gestureDebugPollTimer) {
      return;
    }
    window.clearTimeout(gestureDebugPollTimer);
    gestureDebugPollTimer = 0;
  }

  function isAutomationSectionActive() {
    if (window.MfxSectionWorkspace &&
        typeof window.MfxSectionWorkspace.isAutomationSectionActive === 'function') {
      return window.MfxSectionWorkspace.isAutomationSectionActive();
    }
    return `${location.hash || ''}`.trim().toLowerCase() === '#automation';
  }

  async function pollGestureDebugStateOnce() {
    if (!shouldRunGestureDebugPolling() || !isAutomationSectionActive() || gestureDebugPollInFlight) {
      return;
    }
    gestureDebugPollInFlight = true;
    try {
      const latest = await ensureCore().apiGet('/api/state');
      if (!ensureCore().isObject(latest)) {
        return;
      }
      const routeStatus = latest.input_automation_gesture_route_status;
      if (!ensureCore().isObject(routeStatus)) {
        ensureCore().mergeCachedState({ input_automation_gesture_route_status: null });
        syncWorkspaceGestureRouteStatus(null);
        return;
      }
      ensureCore().mergeCachedState({ input_automation_gesture_route_status: routeStatus });
      syncWorkspaceGestureRouteStatus(routeStatus);
    } catch (_error) {
      // Diagnostics polling is best-effort; fallback to normal reload flow.
    } finally {
      gestureDebugPollInFlight = false;
    }
  }

  async function runGestureDebugPollLoop() {
    await pollGestureDebugStateOnce();
    if (!shouldRunGestureDebugPolling()) {
      clear();
      return;
    }
    const routeStatus = ensureCore().getCachedState()?.input_automation_gesture_route_status;
    const stage = `${(routeStatus && routeStatus.last_stage) || ''}`.trim();
    const activeStage =
      stage === 'gesture_drag_snapshot' ||
      stage === 'buttonless_arm' ||
      stage === 'buttonless_snapshot' ||
      stage === 'gesture_trigger' ||
      stage === 'custom_trigger';
    const idleDelay = isSourceDevMode() ? GESTURE_DEBUG_POLL_MS_IDLE_DEV : GESTURE_DEBUG_POLL_MS_IDLE;
    const activeDelay = isSourceDevMode() ? GESTURE_DEBUG_POLL_MS_ACTIVE_DEV : GESTURE_DEBUG_POLL_MS_ACTIVE;
    gestureDebugPollTimer = window.setTimeout(
      runGestureDebugPollLoop,
      activeStage ? activeDelay : idleDelay,
    );
  }

  function refresh() {
    if (!shouldRunGestureDebugPolling()) {
      clear();
      return;
    }
    if (gestureDebugPollTimer) {
      return;
    }
    const initialDelay = isSourceDevMode() ? GESTURE_DEBUG_POLL_MS_IDLE_DEV : GESTURE_DEBUG_POLL_MS_IDLE;
    gestureDebugPollTimer = window.setTimeout(runGestureDebugPollLoop, initialDelay);
  }

  function onStateRendered(stateSnapshot) {
    syncWorkspaceGestureRouteStatus(stateSnapshot?.input_automation_gesture_route_status || null);
    if (window.MfxSectionWorkspace && typeof window.MfxSectionWorkspace.refresh === 'function') {
      window.MfxSectionWorkspace.refresh();
    }
    refresh();
  }

  const api = {
    clear,
    init,
    onStateRendered,
    refresh,
  };

  window.MfxGestureDebug = api;
})();
