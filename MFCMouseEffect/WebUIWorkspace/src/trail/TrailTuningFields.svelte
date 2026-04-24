<script>
  import { createEventDispatcher } from 'svelte';

  export let trail = {};
  export let onChangeState = null;

  const dispatch = createEventDispatcher();

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
      p_streamer_duration: toNumber(profile.streamer?.duration_ms, 0),
      p_streamer_max: toNumber(profile.streamer?.max_points, 0),
      p_electric_duration: toNumber(profile.electric?.duration_ms, 0),
      p_electric_max: toNumber(profile.electric?.max_points, 0),
      p_meteor_duration: toNumber(profile.meteor?.duration_ms, 0),
      p_meteor_max: toNumber(profile.meteor?.max_points, 0),
      p_tubes_duration: toNumber(profile.tubes?.duration_ms, 0),
      p_tubes_max: toNumber(profile.tubes?.max_points, 0),
      p_line_duration: toNumber(profile.line?.duration_ms, 0),
      p_line_max: toNumber(profile.line?.max_points, 0),
      k_streamer_glow: toNumber(params.streamer?.glow_width_scale, 0),
      k_streamer_core: toNumber(params.streamer?.core_width_scale, 0),
      k_streamer_head: toNumber(params.streamer?.head_power, 0),
      k_electric_amp: toNumber(params.electric?.amplitude_scale, 0),
      k_electric_fork: toNumber(params.electric?.fork_chance, 0),
      k_meteor_rate: toNumber(params.meteor?.spark_rate_scale, 0),
      k_meteor_speed: toNumber(params.meteor?.spark_speed_scale, 0),
      k_idle_fade_start: toNumber(params.idle_fade_start_ms, 0),
      k_idle_fade_end: toNumber(params.idle_fade_end_ms, 0),
    };
  }

  function toSnapshot(form) {
    const value = form || normalizeTrail({});
    return {
      trail_style: value.trail_style || 'default',
      trail_line_width: toNumber(value.trail_line_width, 0),
      trail_profiles: {
        line: { duration_ms: toNumber(value.p_line_duration, 0), max_points: toNumber(value.p_line_max, 0) },
        streamer: { duration_ms: toNumber(value.p_streamer_duration, 0), max_points: toNumber(value.p_streamer_max, 0) },
        electric: { duration_ms: toNumber(value.p_electric_duration, 0), max_points: toNumber(value.p_electric_max, 0) },
        meteor: { duration_ms: toNumber(value.p_meteor_duration, 0), max_points: toNumber(value.p_meteor_max, 0) },
        tubes: { duration_ms: toNumber(value.p_tubes_duration, 0), max_points: toNumber(value.p_tubes_max, 0) },
      },
      trail_params: {
        streamer: {
          glow_width_scale: toNumber(value.k_streamer_glow, 0),
          core_width_scale: toNumber(value.k_streamer_core, 0),
          head_power: toNumber(value.k_streamer_head, 0),
        },
        electric: {
          amplitude_scale: toNumber(value.k_electric_amp, 0),
          fork_chance: toNumber(value.k_electric_fork, 0),
        },
        meteor: {
          spark_rate_scale: toNumber(value.k_meteor_rate, 0),
          spark_speed_scale: toNumber(value.k_meteor_speed, 0),
        },
        idle_fade_start_ms: toNumber(value.k_idle_fade_start, 0),
        idle_fade_end_ms: toNumber(value.k_idle_fade_end, 0),
      },
    };
  }

  let form = normalizeTrail(trail);
  let lastTrailRef = trail;

  $: if (trail !== lastTrailRef) {
    lastTrailRef = trail;
    form = normalizeTrail(trail);
  }

  $: {
    const snapshot = toSnapshot(form);
    if (typeof onChangeState === 'function') {
      onChangeState(snapshot);
    }
    dispatch('change', snapshot);
  }
</script>

<div>
  <div class="grid">
    <label for="trail_style" data-i18n="label_style_preset">Style preset</label>
    <select id="trail_style" bind:value={form.trail_style}>
      <option value="default">default</option>
      <option value="snappy">snappy</option>
      <option value="long">long</option>
      <option value="cinematic">cinematic</option>
      <option value="custom">custom</option>
    </select>

    <div class="hint span2" data-i18n="hint_trail_preset">Preset name only; values below are what actually apply.</div>

    <label for="trail_line_width" data-i18n="label_trail_line_width">Line width (px)</label>
    <input id="trail_line_width" type="number" step="0.5" min="1" max="18" bind:value={form.trail_line_width} />

    <div class="hint span2" data-i18n="hint_trail_line_width">Applies to line trail thickness (non-particle).</div>

    <label for="k_idle_fade_start" data-i18n="label_idle_fade">Idle fade start/end (ms)</label>
    <div class="pair">
      <input id="k_idle_fade_start" type="number" min="0" max="3000" bind:value={form.k_idle_fade_start} />
      <input id="k_idle_fade_end" type="number" min="0" max="6000" bind:value={form.k_idle_fade_end} />
    </div>

    <div class="hint span2" data-i18n="hint_idle_fade">Controls how fast the trail converges after the mouse stops.</div>
  </div>

  <div class="grid grid-offset-top">
    <label for="p_streamer_duration" class="mono" data-i18n="label_streamer_profile">streamer duration/max</label>
    <div class="pair">
      <input id="p_streamer_duration" type="number" min="80" max="2000" bind:value={form.p_streamer_duration} />
      <input id="p_streamer_max" type="number" min="2" max="240" bind:value={form.p_streamer_max} />
    </div>

    <label for="p_electric_duration" class="mono" data-i18n="label_electric_profile">electric duration/max</label>
    <div class="pair">
      <input id="p_electric_duration" type="number" min="80" max="2000" bind:value={form.p_electric_duration} />
      <input id="p_electric_max" type="number" min="2" max="240" bind:value={form.p_electric_max} />
    </div>

    <label for="p_meteor_duration" class="mono" data-i18n="label_meteor_profile">meteor duration/max</label>
    <div class="pair">
      <input id="p_meteor_duration" type="number" min="80" max="2000" bind:value={form.p_meteor_duration} />
      <input id="p_meteor_max" type="number" min="2" max="240" bind:value={form.p_meteor_max} />
    </div>

    <label for="p_tubes_duration" class="mono" data-i18n="label_tubes_profile">tubes duration/max</label>
    <div class="pair">
      <input id="p_tubes_duration" type="number" min="80" max="2000" bind:value={form.p_tubes_duration} />
      <input id="p_tubes_max" type="number" min="2" max="240" bind:value={form.p_tubes_max} />
    </div>

    <label for="p_line_duration" class="mono" data-i18n="label_line_profile">line duration/max</label>
    <div class="pair">
      <input id="p_line_duration" type="number" min="80" max="2000" bind:value={form.p_line_duration} />
      <input id="p_line_max" type="number" min="2" max="240" bind:value={form.p_line_max} />
    </div>

    <div class="sep"></div>

    <label for="k_streamer_glow" class="mono" data-i18n="label_streamer_params">streamer glow/core/head</label>
    <div class="triple">
      <input id="k_streamer_glow" type="number" step="0.05" min="0.5" max="4" bind:value={form.k_streamer_glow} />
      <input id="k_streamer_core" type="number" step="0.05" min="0.2" max="2" bind:value={form.k_streamer_core} />
      <input id="k_streamer_head" type="number" step="0.05" min="0.8" max="3" bind:value={form.k_streamer_head} />
    </div>

    <label for="k_electric_amp" class="mono" data-i18n="label_electric_params">electric amp/fork</label>
    <div class="pair">
      <input id="k_electric_amp" type="number" step="0.05" min="0.2" max="3" bind:value={form.k_electric_amp} />
      <input id="k_electric_fork" type="number" step="0.01" min="0" max="0.5" bind:value={form.k_electric_fork} />
    </div>

    <label for="k_meteor_rate" class="mono" data-i18n="label_meteor_params">meteor rate/speed</label>
    <div class="pair">
      <input id="k_meteor_rate" type="number" step="0.01" min="0.2" max="4" bind:value={form.k_meteor_rate} />
      <input id="k_meteor_speed" type="number" step="0.01" min="0.2" max="4" bind:value={form.k_meteor_speed} />
    </div>

    <div class="hint span2" data-i18n="hint_clamp">Values are clamped to safe ranges when applied.</div>
  </div>
</div>
