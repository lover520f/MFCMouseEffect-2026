#pragma once

#include "MouseFx/Core/System/IGlobalMouseHook.h"

namespace mousefx {

// No-op fallback hook for unsupported platforms.
class NullGlobalMouseHook final : public IGlobalMouseHook {
public:
    bool Start(uintptr_t /*dispatchHandle*/) override { return false; }
    void Stop() override {}
    uint32_t LastError() const override { return 0; }
    bool ConsumeLatestMove(ScreenPoint& /*outPt*/) override { return false; }
    void SetKeyboardCaptureExclusive(bool /*enabled*/) override {}
};

} // namespace mousefx
