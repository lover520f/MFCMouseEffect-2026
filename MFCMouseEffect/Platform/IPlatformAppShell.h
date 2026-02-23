#pragma once

namespace mousefx::platform {

// Minimal app-shell lifecycle used by the process entrypoint.
class IPlatformAppShell {
public:
    virtual ~IPlatformAppShell() = default;

    virtual bool Initialize() = 0;
    virtual int RunMessageLoop() = 0;
    virtual void Shutdown() = 0;
};

} // namespace mousefx::platform
