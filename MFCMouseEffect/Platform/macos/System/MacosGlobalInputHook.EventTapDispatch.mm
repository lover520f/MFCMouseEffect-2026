#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

namespace mousefx {

void MacosGlobalInputHook::HandleTapDisabledEvent() {
#if defined(__APPLE__)
    const bool trusted = AXIsProcessTrusted();
    if (!trusted) {
        lastError_.store(kErrorPermissionDenied, std::memory_order_release);
        return;
    }

    CFMachPortRef tap = nullptr;
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        tap = static_cast<CFMachPortRef>(tapRef_);
    }
    if (tap) {
        CGEventTapEnable(tap, true);
        lastError_.store(kErrorSuccess, std::memory_order_release);
    }
#endif
}

} // namespace mousefx
