#pragma once

#include <cstdint>
#include <string>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

enum class DispatchMessageKind : uint8_t {
    Unknown = 0,
    Click,
    Move,
    Scroll,
    Key,
    ButtonDown,
    ButtonUp,
    Timer,
    ExecCmd,
    GetConfig,
};

// Platform-independent message payload passed to DispatchRouter.
struct DispatchMessage {
    DispatchMessageKind kind = DispatchMessageKind::Unknown;
    uintptr_t sourceHandle = 0;

    int32_t x = 0;
    int32_t y = 0;
    int32_t delta = 0;
    uint32_t button = 0;
    uint32_t timerId = 0;

    ClickEvent* clickEvent = nullptr;
    KeyEvent* keyEvent = nullptr;
    const std::string* commandJson = nullptr;
    EffectConfig* configOut = nullptr;

    uint32_t nativeMsg = 0;
    uintptr_t nativeWParam = 0;
    intptr_t nativeLParam = 0;
};

} // namespace mousefx
