#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"

#include <cstdint>
#include <string>

namespace mousefx::macos_effect_profile {

struct ClickButtonColorProfile {
    uint32_t fillArgb = 0x594FC3F7u;
    uint32_t strokeArgb = 0xFF0288D1u;
    uint32_t glowArgb = 0x660288D1u;
};

struct ClickRenderProfile {
    int normalSizePx = 138;
    int textSizePx = 152;
    double textFontSizePx = 24.0;
    double textFloatDistancePx = 60.0;
    double normalDurationSec = 0.32;
    double textDurationSec = 0.36;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
    double rippleStartRadiusPx = 0.0;
    double rippleEndRadiusPx = 40.0;
    double rippleStrokeWidthPx = 2.5;
    double starStartRadiusPx = 5.0;
    double starEndRadiusPx = 35.0;
    double starStrokeWidthPx = 2.0;
    ClickButtonColorProfile leftButton{};
    ClickButtonColorProfile rightButton{0x50FFB74Du, 0xFFFF6F00u, 0x55FF6F00u};
    ClickButtonColorProfile middleButton{0x5033D17Au, 0xFF0B8043u, 0x550B8043u};
};

struct TrailRenderProfile {
    struct TypeTempoProfile {
        double durationScale = 1.0;
        double sizeScale = 1.0;
    };

    struct TypeColorProfile {
        uint32_t fillArgb = 0x3D66C2FFu;
        uint32_t strokeArgb = 0xF566C2FFu;
    };

    int normalSizePx = 64;
    int particleSizePx = 48;
    double lineWidthPx = 4.0;
    double durationSec = 0.22;
    int closePaddingMs = 40;
    double baseOpacity = 0.95;
    TypeColorProfile line{};
    TypeColorProfile streamer{0x3D52F2EBu, 0xF552F2EBu};
    TypeColorProfile electric{0x3D94BAFFu, 0xF594BAFFu};
    TypeColorProfile meteor{0x3DFFA44Du, 0xF5FFA44Du};
    TypeColorProfile tubes{0x3D6EDB84u, 0xF56EDB84u};
    TypeColorProfile particle{0x3DFFD657u, 0xF5FFD657u};
    TypeTempoProfile lineTempo{};
    TypeTempoProfile streamerTempo{0.94, 1.0};
    TypeTempoProfile electricTempo{0.82, 0.92};
    TypeTempoProfile meteorTempo{1.18, 1.08};
    TypeTempoProfile tubesTempo{1.06, 1.05};
    TypeTempoProfile particleTempo{0.78, 0.86};
};

struct TrailThrottleProfile {
    uint64_t minIntervalMs = 18;
    double minDistancePx = 8.0;
};

struct ScrollRenderProfile {
    struct DirectionColor {
        uint32_t fillArgb = 0x3D5AE0F2u;
        uint32_t strokeArgb = 0xF556E0F2u;
    };

    int verticalSizePx = 138;
    int horizontalSizePx = 148;
    double baseDurationSec = 0.28;
    double perStrengthStepSec = 0.018;
    int closePaddingMs = 90;
    double baseOpacity = 0.96;
    double defaultDurationScale = 1.0;
    double helixDurationScale = 1.14;
    double twinkleDurationScale = 0.88;
    double defaultSizeScale = 1.0;
    double helixSizeScale = 1.06;
    double twinkleSizeScale = 0.94;
    DirectionColor horizontalPositive{};
    DirectionColor horizontalNegative{0x3D9ECCFFu, 0xF59ECCFFu};
    DirectionColor verticalPositive{0x3D6BEA8Fu, 0xF56BEA8Fu};
    DirectionColor verticalNegative{0x3DFF9157u, 0xF5FF9157u};
};

struct HoldRenderProfile {
    struct ColorProfile {
        uint32_t leftBaseStrokeArgb = 0xF542BDFFu;
        uint32_t rightBaseStrokeArgb = 0xF5FF9E42u;
        uint32_t middleBaseStrokeArgb = 0xF56ADB89u;
        uint32_t lightningStrokeArgb = 0xF58FB9FFu;
        uint32_t hexStrokeArgb = 0xF570E59Au;
        uint32_t hologramStrokeArgb = 0xF56CF2E8u;
        uint32_t quantumHaloStrokeArgb = 0xF5A8B3FFu;
        uint32_t fluxFieldStrokeArgb = 0xF573F2A0u;
        uint32_t techNeonStrokeArgb = 0xF580C7FFu;
    };

    int sizePx = 188;
    int progressFullMs = 1400;
    double breatheDurationSec = 0.9;
    double rotateDurationSec = 2.2;
    double rotateDurationFastSec = 1.5;
    double baseOpacity = 0.92;
    ColorProfile colors{};
};

struct HoverRenderProfile {
    struct ColorProfile {
        uint32_t glowFillArgb = 0x1A40B3FFu;
        uint32_t glowStrokeArgb = 0xF240B3FFu;
        uint32_t tubesStrokeArgb = 0xF278E6A1u;
    };

    int sizePx = 172;
    double breatheDurationSec = 0.85;
    double spinDurationSec = 1.6;
    double baseOpacity = 0.9;
    double glowSizeScale = 0.96;
    double tubesSizeScale = 1.08;
    double glowBreatheScale = 0.92;
    double tubesBreatheScale = 1.15;
    double tubesSpinScale = 0.82;
    ColorProfile colors{};
};

struct TestProfileTuning {
    double durationScale = 1.0;
    double sizeScale = 1.0;
    double opacityScale = 1.0;
    double trailThrottleScale = 1.0;
    bool durationOverridden = false;
    bool sizeOverridden = false;
    bool opacityOverridden = false;
    bool trailThrottleOverridden = false;
};

ClickRenderProfile ResolveClickRenderProfile(const EffectConfig& config);
TrailRenderProfile ResolveTrailRenderProfile(const EffectConfig& config, const std::string& trailType);
TrailThrottleProfile ResolveTrailThrottleProfile(const EffectConfig& config, const std::string& trailType);
ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config);
HoldRenderProfile ResolveHoldRenderProfile(const EffectConfig& config);
HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config);
TestProfileTuning ResolveTestProfileTuning();

ClickRenderProfile DefaultClickRenderProfile();
TrailRenderProfile DefaultTrailRenderProfile(const std::string& trailType);
TrailThrottleProfile DefaultTrailThrottleProfile(const std::string& trailType);
ScrollRenderProfile DefaultScrollRenderProfile();
HoldRenderProfile DefaultHoldRenderProfile();
HoverRenderProfile DefaultHoverRenderProfile();

} // namespace mousefx::macos_effect_profile
