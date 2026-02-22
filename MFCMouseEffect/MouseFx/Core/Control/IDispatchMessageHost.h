#pragma once

#include <cstdint>

namespace mousefx {

class IDispatchMessageHandler;

// Platform-specific dispatch host (message-only window on Win32).
class IDispatchMessageHost {
public:
    virtual ~IDispatchMessageHost() = default;

    virtual bool Create(IDispatchMessageHandler* handler) = 0;
    virtual void Destroy() = 0;
    virtual bool IsCreated() const = 0;
    virtual bool IsOwnerThread() const = 0;
    virtual uintptr_t NativeHandle() const = 0;
    virtual uint32_t LastError() const = 0;
    virtual intptr_t SendSync(uint32_t msg, uintptr_t wParam, intptr_t lParam) = 0;
    virtual bool SetTimer(uintptr_t timerId, uint32_t intervalMs) = 0;
    virtual void KillTimer(uintptr_t timerId) = 0;
};

} // namespace mousefx
