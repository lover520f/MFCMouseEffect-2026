#include "pch.h"

#include "SettingsStateMapper.EffectsProfileStateBuilder.Macos.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosTrailPulseEmissionPlanner.h"
#include "Settings/SettingsOptions.h"

#include <unordered_set>

namespace mousefx {
namespace {

nlohmann::json BuildClickCommandJson(const ClickEffectRenderCommand& command) {
    return {
        {"normalized_type", command.normalizedType},
        {"size_px", command.sizePx},
        {"text_font_size_px", command.textFontSizePx},
        {"text_float_distance_px", command.textFloatDistancePx},
        {"text_font_family_utf8", command.textFontFamilyUtf8},
        {"text_emoji", command.textEmoji},
        {"duration_sec", command.animationDurationSec},
        {"close_padding_ms", command.closePaddingMs},
        {"base_opacity", command.baseOpacity},
        {"fill_argb", command.fillArgb},
        {"stroke_argb", command.strokeArgb},
        {"glow_argb", command.glowArgb},
        {"text_label", command.textLabel},
    };
}

nlohmann::json BuildTrailCommandJson(const TrailEffectRenderCommand& command) {
    return {
        {"emit", command.emit},
        {"normalized_type", command.normalizedType},
        {"size_px", command.sizePx},
        {"line_width_px", command.lineWidthPx},
        {"duration_sec", command.durationSec},
        {"close_after_ms", command.closeAfterMs},
        {"base_opacity", command.baseOpacity},
        {"fill_argb", command.fillArgb},
        {"stroke_argb", command.strokeArgb},
        {"delta_x", command.deltaX},
        {"delta_y", command.deltaY},
        {"speed_px", command.speedPx},
        {"intensity", command.intensity},
        {"tubes_mode", command.tubesMode},
        {"particle_mode", command.particleMode},
        {"glow_mode", command.glowMode},
    };
}

nlohmann::json BuildScrollCommandJson(const ScrollEffectRenderCommand& command) {
    return {
        {"emit", command.emit},
        {"normalized_type", command.normalizedType},
        {"horizontal", command.horizontal},
        {"delta", command.delta},
        {"strength_level", command.strengthLevel},
        {"strength_scalar", command.strengthScalar},
        {"intensity", command.intensity},
        {"size_px", command.sizePx},
        {"duration_sec", command.durationSec},
        {"close_after_ms", command.closeAfterMs},
        {"base_opacity", command.baseOpacity},
        {"fill_argb", command.fillArgb},
        {"stroke_argb", command.strokeArgb},
        {"helix_mode", command.helixMode},
        {"twinkle_mode", command.twinkleMode},
    };
}

nlohmann::json BuildHoverCommandJson(const HoverEffectRenderCommand& command) {
    return {
        {"normalized_type", command.normalizedType},
        {"size_px", command.sizePx},
        {"breathe_duration_sec", command.breatheDurationSec},
        {"tubes_spin_duration_sec", command.tubesSpinDurationSec},
        {"base_opacity", command.baseOpacity},
        {"glow_fill_argb", command.glowFillArgb},
        {"glow_stroke_argb", command.glowStrokeArgb},
        {"tubes_stroke_argb", command.tubesStrokeArgb},
        {"tubes_mode", command.tubesMode},
    };
}

nlohmann::json BuildHoldStartCommandJson(const HoldEffectStartCommand& command) {
    return {
        {"normalized_type", command.normalizedType},
        {"size_px", command.sizePx},
        {"progress_full_ms", command.progressFullMs},
        {"breathe_duration_sec", command.breatheDurationSec},
        {"rotate_duration_sec", command.rotateDurationSec},
        {"rotate_duration_fast_sec", command.rotateDurationFastSec},
        {"base_opacity", command.baseOpacity},
        {"left_base_stroke_argb", command.colors.leftBaseStrokeArgb},
        {"right_base_stroke_argb", command.colors.rightBaseStrokeArgb},
        {"middle_base_stroke_argb", command.colors.middleBaseStrokeArgb},
        {"flux_field_stroke_argb", command.colors.fluxFieldStrokeArgb},
        {"tech_neon_stroke_argb", command.colors.techNeonStrokeArgb},
    };
}

nlohmann::json BuildHoldUpdateCommandJson(const HoldEffectUpdateCommand& command) {
    return {
        {"emit", command.emit},
        {"hold_ms", command.holdMs},
        {"overlay_x", command.overlayPoint.x},
        {"overlay_y", command.overlayPoint.y},
    };
}

template <typename NormalizeFn>
nlohmann::json BuildCanonicalNormalizationMatrix(
    const EffectOption* (*metadataFn)(size_t&),
    NormalizeFn normalizeFn) {
    nlohmann::json matrix = nlohmann::json::array();
    size_t count = 0;
    const EffectOption* options = metadataFn(count);
    if (options == nullptr || count == 0) {
        return matrix;
    }

    std::unordered_set<std::string> seenInputs;
    for (size_t i = 0; i < count; ++i) {
        const EffectOption& option = options[i];
        if (option.value != nullptr && seenInputs.insert(option.value).second) {
            matrix.push_back({
                {"input", option.value},
                {"normalized", normalizeFn(option.value)},
            });
        }
        if (option.secondType != nullptr && seenInputs.insert(option.secondType).second) {
            matrix.push_back({
                {"input", option.secondType},
                {"normalized", normalizeFn(option.secondType)},
            });
        }
    }
    return matrix;
}

nlohmann::json BuildMetadataCatalogValues(const EffectOption* (*metadataFn)(size_t&)) {
    nlohmann::json values = nlohmann::json::array();
    size_t count = 0;
    const EffectOption* options = metadataFn(count);
    if (options == nullptr || count == 0) {
        return values;
    }

    for (size_t i = 0; i < count; ++i) {
        const EffectOption& option = options[i];
        if (option.value != nullptr) {
            values.push_back(option.value);
        }
    }
    return values;
}

nlohmann::json BuildAliasMatrixJson() {
    nlohmann::json click = BuildCanonicalNormalizationMatrix(
        ClickMetadata,
        [](const std::string& value) { return NormalizeClickEffectType(value); });
    click.push_back({{"input", "TEXT"}, {"normalized", NormalizeClickEffectType("TEXT")}});
    click.push_back({{"input", "icon"}, {"normalized", NormalizeClickEffectType("icon")}});
    click.push_back({{"input", "textclick"}, {"normalized", NormalizeClickEffectType("textclick")}});
    click.push_back({{"input", "none"}, {"normalized", NormalizeClickEffectType("none")}});
    click.push_back({{"input", "star"}, {"normalized", NormalizeClickEffectType("star")}});
    click.push_back({{"input", "ripple_custom"}, {"normalized", NormalizeClickEffectType("ripple_custom")}});

    nlohmann::json trail = BuildCanonicalNormalizationMatrix(
        TrailMetadata,
        [](const std::string& value) { return NormalizeTrailEffectType(value); });
    trail.push_back({{"input", "scifi"}, {"normalized", NormalizeTrailEffectType("scifi")}});
    trail.push_back({{"input", "stream"}, {"normalized", NormalizeTrailEffectType("stream")}});
    trail.push_back({{"input", "neon"}, {"normalized", NormalizeTrailEffectType("neon")}});
    trail.push_back({{"input", "suspension"}, {"normalized", NormalizeTrailEffectType("suspension")}});
    trail.push_back({{"input", "none"}, {"normalized", NormalizeTrailEffectType("none")}});
    trail.push_back({{"input", "default"}, {"normalized", NormalizeTrailEffectType("default")}});
    trail.push_back({{"input", "spark"}, {"normalized", NormalizeTrailEffectType("spark")}});

    nlohmann::json scroll = BuildCanonicalNormalizationMatrix(
        ScrollMetadata,
        [](const std::string& value) { return NormalizeScrollEffectType(value); });
    scroll.push_back({{"input", "stardust"}, {"normalized", NormalizeScrollEffectType("stardust")}});
    scroll.push_back({{"input", "helix"}, {"normalized", NormalizeScrollEffectType("helix")}});
    scroll.push_back({{"input", "none"}, {"normalized", NormalizeScrollEffectType("none")}});
    scroll.push_back({{"input", "arrow"}, {"normalized", NormalizeScrollEffectType("arrow")}});

    nlohmann::json hover = BuildCanonicalNormalizationMatrix(
        HoverMetadata,
        [](const std::string& value) { return NormalizeHoverEffectType(value); });
    hover.push_back({{"input", "suspension"}, {"normalized", NormalizeHoverEffectType("suspension")}});
    hover.push_back({{"input", "helix"}, {"normalized", NormalizeHoverEffectType("helix")}});
    hover.push_back({{"input", "none"}, {"normalized", NormalizeHoverEffectType("none")}});
    hover.push_back({{"input", "glow"}, {"normalized", NormalizeHoverEffectType("glow")}});

    nlohmann::json hold = BuildCanonicalNormalizationMatrix(
        HoldMetadata,
        [](const std::string& value) { return NormalizeHoldEffectType(value); });
    hold.push_back({{"input", "hold_neon3d_gpu_v2"}, {"normalized", NormalizeHoldEffectType("hold_neon3d_gpu_v2")}});
    hold.push_back({{"input", "hold_fluxfield_gpu_v2"}, {"normalized", NormalizeHoldEffectType("hold_fluxfield_gpu_v2")}});
    hold.push_back({{"input", "scifi3d"}, {"normalized", NormalizeHoldEffectType("scifi3d")}});
    hold.push_back({{"input", "neon3d"}, {"normalized", NormalizeHoldEffectType("neon3d")}});
    hold.push_back({{"input", "charge"}, {"normalized", NormalizeHoldEffectType("charge")}});
    const nlohmann::json holdFollowMode = nlohmann::json::array({
        {{"input", "precise"}, {"normalized", config_internal::NormalizeHoldFollowMode("precise")}},
        {{"input", "cursor_priority"}, {"normalized", config_internal::NormalizeHoldFollowMode("cursor_priority")}},
        {{"input", "performance_first"}, {"normalized", config_internal::NormalizeHoldFollowMode("performance_first")}},
        {{"input", "cpu_saver"}, {"normalized", config_internal::NormalizeHoldFollowMode("cpu_saver")}},
    });

    return {
        {"click", click},
        {"trail", trail},
        {"scroll", scroll},
        {"hover", hover},
        {"hold", hold},
        {"hold_follow_mode", holdFollowMode},
        {"catalog_values", {
            {"click", BuildMetadataCatalogValues(ClickMetadata)},
            {"trail", BuildMetadataCatalogValues(TrailMetadata)},
            {"scroll", BuildMetadataCatalogValues(ScrollMetadata)},
            {"hold", BuildMetadataCatalogValues(HoldMetadata)},
            {"hover", BuildMetadataCatalogValues(HoverMetadata)},
        }},
    };
}

} // namespace

nlohmann::json BuildMacosEffectsProfileStateJson(const EffectConfig& cfg) {
    const auto clickProfile = macos_effect_profile::ResolveClickRenderProfile(cfg);
    const auto trailProfile = macos_effect_profile::ResolveTrailRenderProfile(cfg, cfg.active.trail);
    const auto trailThrottle = macos_effect_profile::ResolveTrailThrottleProfile(cfg, cfg.active.trail);
    const auto scrollProfile = macos_effect_profile::ResolveScrollRenderProfile(cfg);
    const auto holdProfile = macos_effect_profile::ResolveHoldRenderProfile(cfg);
    const auto hoverProfile = macos_effect_profile::ResolveHoverRenderProfile(cfg);
    const auto testTuning = macos_effect_profile::ResolveTestProfileTuning();
    const auto trailPlannerConfig = macos_trail_pulse::ResolveTrailPulseEmissionPlannerConfig();

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
            {"opacity_scale", testTuning.opacityScale},
            {"trail_throttle_scale", testTuning.trailThrottleScale},
            {"duration_overridden", testTuning.durationOverridden},
            {"size_overridden", testTuning.sizeOverridden},
            {"opacity_overridden", testTuning.opacityOverridden},
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
        {"line_width_px", trailProfile.lineWidthPx},
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
    out["trail_emission_planner"] = {
        {"teleport_skip_distance_px", trailPlannerConfig.teleportSkipDistancePx},
        {"max_segments", trailPlannerConfig.maxSegments},
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

nlohmann::json BuildMacosEffectRenderCommandSamplesJson(const EffectConfig& cfg) {
    constexpr ScreenPoint kSamplePoint{640, 360};
    constexpr ScreenPoint kSampleLastPoint{624, 352};
    constexpr uint64_t kSampleNowMs = 1000;
    constexpr uint64_t kSampleLastEmitTickMs = 960;
    constexpr double kSampleTrailDeltaX = 22.0;
    constexpr double kSampleTrailDeltaY = 14.0;
    constexpr int kSampleScrollDelta = 120;
    constexpr uint32_t kSampleHoldDurationMs = 420;
    constexpr uint64_t kSampleHoldNowMs = 2000;

    const auto clickProfile = macos_effect_profile::ResolveClickRenderProfile(cfg);
    const auto trailProfile = macos_effect_profile::ResolveTrailRenderProfile(cfg, cfg.active.trail);
    const auto trailThrottle = macos_effect_profile::ResolveTrailThrottleProfile(cfg, cfg.active.trail);
    const auto scrollProfile = macos_effect_profile::ResolveScrollRenderProfile(cfg);
    const auto holdProfile = macos_effect_profile::ResolveHoldRenderProfile(cfg);
    const auto hoverProfile = macos_effect_profile::ResolveHoverRenderProfile(cfg);
    const auto trailPlannerConfig = macos_trail_pulse::ResolveTrailPulseEmissionPlannerConfig();

    const ClickEffectRenderCommand clickCommand = ComputeClickEffectRenderCommand(
        kSamplePoint,
        MouseButton::Left,
        cfg.active.click,
        macos_effect_compute_profile::BuildClickProfile(clickProfile));
    const TrailEffectRenderCommand trailCommand = ComputeTrailEffectRenderCommand(
        kSamplePoint,
        kSampleTrailDeltaX,
        kSampleTrailDeltaY,
        cfg.active.trail,
        macos_effect_compute_profile::BuildTrailProfile(trailProfile));
    const TrailEffectEmissionResult trailEmission = ComputeTrailEffectEmission(
        kSamplePoint,
        kSampleLastPoint,
        kSampleNowMs,
        kSampleLastEmitTickMs,
        macos_effect_compute_profile::BuildTrailThrottleProfile(trailThrottle));
    const ScrollEffectRenderCommand scrollCommand = ComputeScrollEffectRenderCommand(
        kSamplePoint,
        false,
        kSampleScrollDelta,
        cfg.active.scroll,
        macos_effect_compute_profile::BuildScrollProfile(scrollProfile));
    const ScrollEffectInputShaperProfile scrollShaper = ResolveScrollInputShaperProfile(cfg.active.scroll);
    const HoverEffectRenderCommand hoverCommand = ComputeHoverEffectRenderCommand(
        kSamplePoint,
        cfg.active.hover,
        macos_effect_compute_profile::BuildHoverProfile(hoverProfile));
    const HoldEffectStartCommand holdStartCommand = ComputeHoldEffectStartCommand(
        kSamplePoint,
        MouseButton::Left,
        cfg.active.hold,
        macos_effect_compute_profile::BuildHoldProfile(holdProfile));
    HoldEffectFollowState holdFollowState{};
    const HoldEffectUpdateCommand holdUpdateCommand = ComputeHoldEffectUpdateCommand(
        kSamplePoint,
        kSampleHoldDurationMs,
        kSampleHoldNowMs,
        ParseHoldEffectFollowMode(cfg.holdFollowMode),
        &holdFollowState);

    constexpr ScreenPoint kSmoothFirstPoint{600, 340};
    constexpr ScreenPoint kSmoothSecondPoint{700, 420};
    HoldEffectFollowState smoothFollowState{};
    const HoldEffectUpdateCommand holdSmoothFirstCommand = ComputeHoldEffectUpdateCommand(
        kSmoothFirstPoint,
        420,
        3000,
        HoldEffectFollowMode::Smooth,
        &smoothFollowState);
    const HoldEffectUpdateCommand holdSmoothSecondCommand = ComputeHoldEffectUpdateCommand(
        kSmoothSecondPoint,
        440,
        3016,
        HoldEffectFollowMode::Smooth,
        &smoothFollowState);

    constexpr ScreenPoint kEfficientFirstPoint{640, 360};
    constexpr ScreenPoint kEfficientSuppressedPoint{664, 376};
    constexpr ScreenPoint kEfficientResumedPoint{688, 392};
    HoldEffectFollowState efficientFollowState{};
    const HoldEffectUpdateCommand holdEfficientFirstCommand = ComputeHoldEffectUpdateCommand(
        kEfficientFirstPoint,
        420,
        4000,
        HoldEffectFollowMode::Efficient,
        &efficientFollowState);
    const HoldEffectUpdateCommand holdEfficientSuppressedCommand = ComputeHoldEffectUpdateCommand(
        kEfficientSuppressedPoint,
        430,
        4010,
        HoldEffectFollowMode::Efficient,
        &efficientFollowState);
    const HoldEffectUpdateCommand holdEfficientResumedCommand = ComputeHoldEffectUpdateCommand(
        kEfficientResumedPoint,
        450,
        4035,
        HoldEffectFollowMode::Efficient,
        &efficientFollowState);

    nlohmann::json out = nlohmann::json::object();
    out["sample_input"] = {
        {"point", {{"x", kSamplePoint.x}, {"y", kSamplePoint.y}}},
        {"trail_delta", {{"x", kSampleTrailDeltaX}, {"y", kSampleTrailDeltaY}}},
        {"scroll_delta", kSampleScrollDelta},
        {"hold_duration_ms", kSampleHoldDurationMs},
        {"hold_follow_mode", cfg.holdFollowMode},
        {"hold_follow_mode_normalized", config_internal::NormalizeHoldFollowMode(cfg.holdFollowMode)},
        {"active_raw", {
            {"click", cfg.active.click},
            {"trail", cfg.active.trail},
            {"scroll", cfg.active.scroll},
            {"hold", cfg.active.hold},
            {"hover", cfg.active.hover},
        }},
        {"active_normalized", {
            {"click", NormalizeClickEffectType(cfg.active.click)},
            {"trail", NormalizeTrailEffectType(cfg.active.trail)},
            {"scroll", NormalizeScrollEffectType(cfg.active.scroll)},
            {"hold", NormalizeHoldEffectType(cfg.active.hold)},
            {"hover", NormalizeHoverEffectType(cfg.active.hover)},
        }},
    };
    out["active_resolved"] = {
        {"click", clickCommand.normalizedType},
        {"trail", trailCommand.normalizedType},
        {"scroll", scrollCommand.normalizedType},
        {"hold", holdStartCommand.normalizedType},
        {"hover", hoverCommand.normalizedType},
    };
    out["click"] = BuildClickCommandJson(clickCommand);
    out["trail"] = BuildTrailCommandJson(trailCommand);
    out["trail_emission"] = {
        {"should_emit", trailEmission.shouldEmit},
        {"delta_x", trailEmission.deltaX},
        {"delta_y", trailEmission.deltaY},
        {"distance_px", trailEmission.distancePx},
        {"throttle_min_interval_ms", trailThrottle.minIntervalMs},
        {"throttle_min_distance_px", trailThrottle.minDistancePx},
    };
    out["scroll"] = BuildScrollCommandJson(scrollCommand);
    out["hover"] = BuildHoverCommandJson(hoverCommand);
    out["hold"] = {
        {"start", BuildHoldStartCommandJson(holdStartCommand)},
        {"update", BuildHoldUpdateCommandJson(holdUpdateCommand)},
    };
    out["hold_follow_mode_samples"] = {
        {"smooth_first", BuildHoldUpdateCommandJson(holdSmoothFirstCommand)},
        {"smooth_second", BuildHoldUpdateCommandJson(holdSmoothSecondCommand)},
        {"efficient_first", BuildHoldUpdateCommandJson(holdEfficientFirstCommand)},
        {"efficient_suppressed", BuildHoldUpdateCommandJson(holdEfficientSuppressedCommand)},
        {"efficient_resumed", BuildHoldUpdateCommandJson(holdEfficientResumedCommand)},
    };
    out["alias_matrix"] = BuildAliasMatrixJson();
    out["effective_timing"] = {
        {"click_duration_sec", clickCommand.animationDurationSec},
        {"click_text_font_size_px", clickCommand.textFontSizePx},
        {"click_text_float_distance_px", clickCommand.textFloatDistancePx},
        {"trail_duration_sec", trailCommand.durationSec},
        {"trail_command_line_width_px", trailCommand.lineWidthPx},
        {"trail_profile_line_width_px", trailProfile.lineWidthPx},
        {"scroll_duration_sec", scrollCommand.durationSec},
        {"trail_planner_teleport_skip_distance_px", trailPlannerConfig.teleportSkipDistancePx},
        {"trail_planner_max_segments", trailPlannerConfig.maxSegments},
        {"scroll_emit_interval_ms", scrollShaper.emitIntervalMs},
        {"scroll_max_duration_ms", scrollShaper.maxDurationMs},
        {"hover_breathe_duration_sec", hoverCommand.breatheDurationSec},
        {"hover_spin_duration_sec", hoverCommand.tubesSpinDurationSec},
        {"hold_progress_full_ms", holdStartCommand.progressFullMs},
        {"hold_breathe_duration_sec", holdStartCommand.breatheDurationSec},
    };
    return out;
}

} // namespace mousefx
