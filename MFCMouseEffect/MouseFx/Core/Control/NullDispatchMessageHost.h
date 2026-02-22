#pragma once

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

namespace mousefx {

class NullDispatchMessageHost final : public IDispatchMessageHost {
public:
    bool Create(IDispatchMessageHandler* /*handler*/) override { return false; }
    void Destroy() override {}
    bool IsCreated() const override { return false; }
    bool IsOwnerThread() const override { return false; }
    uintptr_t NativeHandle() const override { return 0; }
    uint32_t LastError() const override { return 0; }
    intptr_t SendSync(uint32_t /*msg*/, uintptr_t /*wParam*/, intptr_t /*lParam*/) override { return 0; }
    bool SetTimer(uintptr_t /*timerId*/, uint32_t /*intervalMs*/) override { return false; }
    void KillTimer(uintptr_t /*timerId*/) override {}
};

} // namespace mousefx
