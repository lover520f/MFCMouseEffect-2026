#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

enum class MouseCompanionPetInputKind : uint8_t {
    Move = 0,
    Scroll,
    Click,
    ButtonDown,
    ButtonUp,
    HoverStart,
    HoverEnd,
    HoldStart,
    HoldUpdate,
    HoldEnd,
};

enum class MouseCompanionPetActionHint : uint8_t {
    None = 0,
    Idle,
    Follow,
    Click,
    Drag,
    Hold,
    Scroll,
};

struct MouseCompanionPetInputEvent {
    MouseCompanionPetInputKind kind{MouseCompanionPetInputKind::Move};
    MouseCompanionPetActionHint actionHint{MouseCompanionPetActionHint::None};
    ScreenPoint pt{};
    int button{0};
    int delta{0};
    uint32_t holdMs{0};
    uint64_t tickMs{0};
    float actionIntensity{0.0f};
    bool accepted{false};
};

struct MouseCompanionPetRuntimeConfig {
    bool enabled{false};
    bool useTestProfile{false};
    bool facePointerEnabled{false};
    int sizePx{0};
    int offsetX{0};
    int offsetY{0};
    int absoluteX{0};
    int absoluteY{0};
    int pressLiftPx{0};
    int smoothingPercent{0};
    int followThresholdPx{0};
    int releaseHoldMs{0};
    int clickStreakBreakMs{0};
    float headTintPerClick{0.0f};
    float headTintMax{0.0f};
    float headTintDecayPerSecond{0.0f};
    std::string positionMode;
    std::string targetMonitor;
    std::string edgeClampMode;
};

struct MouseCompanionPetPoseSample {
    int boneIndex{-1};
    float position[3]{0.0f, 0.0f, 0.0f};
    float rotation[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float scale[3]{1.0f, 1.0f, 1.0f};
};

struct MouseCompanionPetPoseFrame {
    uint64_t sampleTickMs{0};
    std::string actionName{"idle"};
    float actionIntensity{0.0f};
    float headTintAmount{0.0f};
    std::vector<MouseCompanionPetPoseSample> samples;

    void Clear() {
        sampleTickMs = 0;
        actionName = "idle";
        actionIntensity = 0.0f;
        headTintAmount = 0.0f;
        samples.clear();
    }
};

struct MouseCompanionPluginV1Diagnostics {
    bool hostReady{false};
    std::string hostPhase;
    std::string activePluginId;
    std::string activePluginVersion;
    std::string engineApiVersion;
    std::string compatibilityStatus;
    std::string fallbackReason;
    std::string lastEventName;
    std::string lastActionName;
    uint64_t lastEventTickMs{0};
    uint64_t lastTickMs{0};
    uint64_t lastPoseSampleTickMs{0};
    uint64_t eventCount{0};
    uint64_t tickCount{0};
    uint64_t poseSampleCount{0};
};

} // namespace mousefx
