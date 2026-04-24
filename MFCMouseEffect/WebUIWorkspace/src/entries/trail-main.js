import TrailTuningFields from '../trail/TrailTuningFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';
import { mountLegacyComponent } from './legacy-component.js';

let currentState = {
  trail_style: 'default',
  trail_line_width: 0,
  trail_profiles: {
    line: { duration_ms: 0, max_points: 0 },
    streamer: { duration_ms: 0, max_points: 0 },
    electric: { duration_ms: 0, max_points: 0 },
    meteor: { duration_ms: 0, max_points: 0 },
    tubes: { duration_ms: 0, max_points: 0 },
  },
  trail_params: {
    streamer: { glow_width_scale: 0, core_width_scale: 0, head_power: 0 },
    electric: { amplitude_scale: 0, fork_chance: 0 },
    meteor: { spark_rate_scale: 0, spark_speed_scale: 0 },
    idle_fade_start_ms: 0,
    idle_fade_end_ms: 0,
  },
};

function toNumber(value, fallback) {
  const parsed = Number(value);
  if (Number.isFinite(parsed)) return parsed;
  return fallback;
}

function normalizeTrail(input) {
  const value = input || {};
  const profile = value.trail_profiles || {};
  const params = value.trail_params || {};
  return {
    trail_style: value.trail_style || 'default',
    trail_line_width: toNumber(value.trail_line_width, 0),
    trail_profiles: {
      line: {
        duration_ms: toNumber(profile.line?.duration_ms, 0),
        max_points: toNumber(profile.line?.max_points, 0),
      },
      streamer: {
        duration_ms: toNumber(profile.streamer?.duration_ms, 0),
        max_points: toNumber(profile.streamer?.max_points, 0),
      },
      electric: {
        duration_ms: toNumber(profile.electric?.duration_ms, 0),
        max_points: toNumber(profile.electric?.max_points, 0),
      },
      meteor: {
        duration_ms: toNumber(profile.meteor?.duration_ms, 0),
        max_points: toNumber(profile.meteor?.max_points, 0),
      },
      tubes: {
        duration_ms: toNumber(profile.tubes?.duration_ms, 0),
        max_points: toNumber(profile.tubes?.max_points, 0),
      },
    },
    trail_params: {
      streamer: {
        glow_width_scale: toNumber(params.streamer?.glow_width_scale, 0),
        core_width_scale: toNumber(params.streamer?.core_width_scale, 0),
        head_power: toNumber(params.streamer?.head_power, 0),
      },
      electric: {
        amplitude_scale: toNumber(params.electric?.amplitude_scale, 0),
        fork_chance: toNumber(params.electric?.fork_chance, 0),
      },
      meteor: {
        spark_rate_scale: toNumber(params.meteor?.spark_rate_scale, 0),
        spark_speed_scale: toNumber(params.meteor?.spark_speed_scale, 0),
      },
      idle_fade_start_ms: toNumber(params.idle_fade_start_ms, 0),
      idle_fade_end_ms: toNumber(params.idle_fade_end_ms, 0),
    },
  };
}

const bridge = createLazyMountBridge({
  mountId: 'trail_settings_mount',
  initialProps: {
    trail: currentState,
  },
  createComponent: (mountNode, props) => {
    return mountLegacyComponent(TrailTuningFields, mountNode, {
      ...props,
      onChangeState: (detail) => {
        currentState = normalizeTrail(detail);
      },
    });
  },
});

function render(payload) {
  const appState = payload?.state || {};
  const trail = normalizeTrail(appState);
  currentState = trail;
  bridge.updateProps({ trail });
}

function read() {
  return normalizeTrail(currentState);
}

window.MfxTrailSection = {
  render,
  read,
};
