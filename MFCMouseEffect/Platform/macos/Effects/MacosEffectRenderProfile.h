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
    double normalDurationSec = 0.32;
    double textDurationSec = 0.36;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
    ClickButtonColorProfile leftButton{};
    ClickButtonColorProfile rightButton{0x50FFB74Du, 0xFFFF6F00u, 0x55FF6F00u};
    ClickButtonColorProfile middleButton{0x5033D17Au, 0xFF0B8043u, 0x550B8043u};
};

struct TrailRenderProfile {
    int normalSizePx = 64;
    int particleSizePx = 48;
    double durationSec = 0.22;
    int closePaddingMs = 40;
    double baseOpacity = 0.95;
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
    DirectionColor horizontalPositive{};
    DirectionColor horizontalNegative{0x3D9ECCFFu, 0xF59ECCFFu};
    DirectionColor verticalPositive{0x3D6BEA8Fu, 0xF56BEA8Fu};
    DirectionColor verticalNegative{0x3DFF9157u, 0xF5FF9157u};
};

struct HoldRenderProfile {
    int sizePx = 188;
    int progressFullMs = 1400;
    double breatheDurationSec = 0.9;
    double rotateDurationSec = 2.2;
    double rotateDurationFastSec = 1.5;
    double baseOpacity = 0.92;
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
    ColorProfile colors{};
};

ClickRenderProfile ResolveClickRenderProfile(const EffectConfig& config);
TrailRenderProfile ResolveTrailRenderProfile(const EffectConfig& config, const std::string& trailType);
TrailThrottleProfile ResolveTrailThrottleProfile(const EffectConfig& config, const std::string& trailType);
ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config);
HoldRenderProfile ResolveHoldRenderProfile(const EffectConfig& config);
HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config);

ClickRenderProfile DefaultClickRenderProfile();
TrailRenderProfile DefaultTrailRenderProfile(const std::string& trailType);
TrailThrottleProfile DefaultTrailThrottleProfile(const std::string& trailType);
ScrollRenderProfile DefaultScrollRenderProfile();
HoldRenderProfile DefaultHoldRenderProfile();
HoverRenderProfile DefaultHoverRenderProfile();

} // namespace mousefx::macos_effect_profile
