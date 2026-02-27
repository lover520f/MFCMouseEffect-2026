#include "pch.h"

#include "SettingsStateMapper.EffectsProfileStateBuilder.Macos.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

namespace mousefx {

nlohmann::json BuildMacosEffectsProfileStateJson(const EffectConfig& cfg) {
    const auto clickProfile = macos_effect_profile::ResolveClickRenderProfile(cfg);
    const auto trailProfile = macos_effect_profile::ResolveTrailRenderProfile(cfg, cfg.active.trail);
    const auto trailThrottle = macos_effect_profile::ResolveTrailThrottleProfile(cfg, cfg.active.trail);
    const auto scrollProfile = macos_effect_profile::ResolveScrollRenderProfile(cfg);
    const auto holdProfile = macos_effect_profile::ResolveHoldRenderProfile(cfg);
    const auto hoverProfile = macos_effect_profile::ResolveHoverRenderProfile(cfg);
    const auto testTuning = macos_effect_profile::ResolveTestProfileTuning();

    nlohmann::json out = nlohmann::json::object();
    out["platform"] = "macos";
    out["config_basis"] = {
        {"ripple_duration_ms", cfg.ripple.durationMs},
        {"ripple_window_size", cfg.ripple.windowSize},
        {"text_duration_ms", cfg.textClick.durationMs},
        {"trail_profile_duration_ms", cfg.GetTrailHistoryProfile(cfg.active.trail).durationMs},
        {"trail_profile_max_points", cfg.GetTrailHistoryProfile(cfg.active.trail).maxPoints},
        {"test_tuning", {
            {"duration_scale", testTuning.durationScale},
            {"size_scale", testTuning.sizeScale},
            {"trail_throttle_scale", testTuning.trailThrottleScale},
            {"duration_overridden", testTuning.durationOverridden},
            {"size_overridden", testTuning.sizeOverridden},
            {"trail_throttle_overridden", testTuning.trailThrottleOverridden},
        }},
    };
    out["click"] = {
        {"normal_size_px", clickProfile.normalSizePx},
        {"text_size_px", clickProfile.textSizePx},
        {"normal_duration_sec", clickProfile.normalDurationSec},
        {"text_duration_sec", clickProfile.textDurationSec},
        {"close_padding_ms", clickProfile.closePaddingMs},
        {"base_opacity", clickProfile.baseOpacity},
    };
    out["trail"] = {
        {"normal_size_px", trailProfile.normalSizePx},
        {"particle_size_px", trailProfile.particleSizePx},
        {"duration_sec", trailProfile.durationSec},
        {"close_padding_ms", trailProfile.closePaddingMs},
        {"base_opacity", trailProfile.baseOpacity},
        {"line_duration_scale", trailProfile.lineTempo.durationScale},
        {"line_size_scale", trailProfile.lineTempo.sizeScale},
        {"streamer_duration_scale", trailProfile.streamerTempo.durationScale},
        {"streamer_size_scale", trailProfile.streamerTempo.sizeScale},
        {"electric_duration_scale", trailProfile.electricTempo.durationScale},
        {"electric_size_scale", trailProfile.electricTempo.sizeScale},
        {"meteor_duration_scale", trailProfile.meteorTempo.durationScale},
        {"meteor_size_scale", trailProfile.meteorTempo.sizeScale},
        {"tubes_duration_scale", trailProfile.tubesTempo.durationScale},
        {"tubes_size_scale", trailProfile.tubesTempo.sizeScale},
        {"particle_duration_scale", trailProfile.particleTempo.durationScale},
        {"particle_size_scale", trailProfile.particleTempo.sizeScale},
        {"line_stroke_argb", trailProfile.line.strokeArgb},
        {"line_fill_argb", trailProfile.line.fillArgb},
        {"streamer_stroke_argb", trailProfile.streamer.strokeArgb},
        {"streamer_fill_argb", trailProfile.streamer.fillArgb},
        {"electric_stroke_argb", trailProfile.electric.strokeArgb},
        {"electric_fill_argb", trailProfile.electric.fillArgb},
        {"meteor_stroke_argb", trailProfile.meteor.strokeArgb},
        {"meteor_fill_argb", trailProfile.meteor.fillArgb},
        {"tubes_stroke_argb", trailProfile.tubes.strokeArgb},
        {"tubes_fill_argb", trailProfile.tubes.fillArgb},
        {"particle_stroke_argb", trailProfile.particle.strokeArgb},
        {"particle_fill_argb", trailProfile.particle.fillArgb},
    };
    out["trail_throttle"] = {
        {"min_interval_ms", trailThrottle.minIntervalMs},
        {"min_distance_px", trailThrottle.minDistancePx},
    };
    out["scroll"] = {
        {"vertical_size_px", scrollProfile.verticalSizePx},
        {"horizontal_size_px", scrollProfile.horizontalSizePx},
        {"base_duration_sec", scrollProfile.baseDurationSec},
        {"per_strength_step_sec", scrollProfile.perStrengthStepSec},
        {"close_padding_ms", scrollProfile.closePaddingMs},
        {"base_opacity", scrollProfile.baseOpacity},
        {"default_duration_scale", scrollProfile.defaultDurationScale},
        {"helix_duration_scale", scrollProfile.helixDurationScale},
        {"twinkle_duration_scale", scrollProfile.twinkleDurationScale},
        {"default_size_scale", scrollProfile.defaultSizeScale},
        {"helix_size_scale", scrollProfile.helixSizeScale},
        {"twinkle_size_scale", scrollProfile.twinkleSizeScale},
        {"horizontal_positive_stroke_argb", scrollProfile.horizontalPositive.strokeArgb},
        {"horizontal_positive_fill_argb", scrollProfile.horizontalPositive.fillArgb},
        {"horizontal_negative_stroke_argb", scrollProfile.horizontalNegative.strokeArgb},
        {"horizontal_negative_fill_argb", scrollProfile.horizontalNegative.fillArgb},
        {"vertical_positive_stroke_argb", scrollProfile.verticalPositive.strokeArgb},
        {"vertical_positive_fill_argb", scrollProfile.verticalPositive.fillArgb},
        {"vertical_negative_stroke_argb", scrollProfile.verticalNegative.strokeArgb},
        {"vertical_negative_fill_argb", scrollProfile.verticalNegative.fillArgb},
    };
    out["hold"] = {
        {"size_px", holdProfile.sizePx},
        {"progress_full_ms", holdProfile.progressFullMs},
        {"breathe_duration_sec", holdProfile.breatheDurationSec},
        {"rotate_duration_sec", holdProfile.rotateDurationSec},
        {"rotate_duration_fast_sec", holdProfile.rotateDurationFastSec},
        {"base_opacity", holdProfile.baseOpacity},
        {"left_base_stroke_argb", holdProfile.colors.leftBaseStrokeArgb},
        {"right_base_stroke_argb", holdProfile.colors.rightBaseStrokeArgb},
        {"middle_base_stroke_argb", holdProfile.colors.middleBaseStrokeArgb},
        {"lightning_stroke_argb", holdProfile.colors.lightningStrokeArgb},
        {"hex_stroke_argb", holdProfile.colors.hexStrokeArgb},
        {"hologram_stroke_argb", holdProfile.colors.hologramStrokeArgb},
        {"quantum_halo_stroke_argb", holdProfile.colors.quantumHaloStrokeArgb},
        {"flux_field_stroke_argb", holdProfile.colors.fluxFieldStrokeArgb},
        {"tech_neon_stroke_argb", holdProfile.colors.techNeonStrokeArgb},
    };
    out["hover"] = {
        {"size_px", hoverProfile.sizePx},
        {"breathe_duration_sec", hoverProfile.breatheDurationSec},
        {"spin_duration_sec", hoverProfile.spinDurationSec},
        {"base_opacity", hoverProfile.baseOpacity},
        {"glow_size_scale", hoverProfile.glowSizeScale},
        {"tubes_size_scale", hoverProfile.tubesSizeScale},
        {"glow_breathe_scale", hoverProfile.glowBreatheScale},
        {"tubes_breathe_scale", hoverProfile.tubesBreatheScale},
        {"tubes_spin_scale", hoverProfile.tubesSpinScale},
        {"glow_fill_argb", hoverProfile.colors.glowFillArgb},
        {"glow_stroke_argb", hoverProfile.colors.glowStrokeArgb},
        {"tubes_stroke_argb", hoverProfile.colors.tubesStrokeArgb},
    };
    return out;
}

} // namespace mousefx
