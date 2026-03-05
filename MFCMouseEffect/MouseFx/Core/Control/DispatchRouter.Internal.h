#pragma once

#include "AppController.h"
#include "DispatchMessage.h"

namespace mousefx::dispatch_router_detail {

inline ScreenPoint MessagePoint(const DispatchMessage& message) {
    ScreenPoint pt{};
    pt.x = message.x;
    pt.y = message.y;
    return pt;
}

inline bool IsKnownTimerId(uintptr_t timerId) {
    if (timerId == AppController::HoverTimerId()) {
        return true;
    }
    if (timerId == AppController::HoldTimerId()) {
        return true;
    }
    if (timerId == AppController::HoldUpdateTimerId()) {
        return true;
    }
#ifdef _DEBUG
    static constexpr uintptr_t kSelfTestTimerId = 0x4D46;
    if (timerId == kSelfTestTimerId) {
        return true;
    }
#endif
    return false;
}

} // namespace mousefx::dispatch_router_detail
