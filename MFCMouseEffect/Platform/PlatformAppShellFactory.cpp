#include "pch.h"

#include "Platform/PlatformAppShellFactory.h"

#include <utility>

#if defined(_WIN32)
#include "Platform/windows/Shell/Win32AppShell.h"
#endif

namespace mousefx::platform {

namespace {

#if defined(_WIN32)
class Win32PlatformAppShell final : public IPlatformAppShell {
public:
    explicit Win32PlatformAppShell(ShellPlatformServices services)
        : shell_(std::move(services)) {
    }

    bool Initialize() override {
        return shell_.Initialize();
    }

    int RunMessageLoop() override {
        return shell_.RunMessageLoop();
    }

    void Shutdown() override {
        shell_.Shutdown();
    }

private:
    Win32AppShell shell_;
};
#else
class NullPlatformAppShell final : public IPlatformAppShell {
public:
    bool Initialize() override {
        return false;
    }

    int RunMessageLoop() override {
        return -1;
    }

    void Shutdown() override {
    }
};
#endif

} // namespace

std::unique_ptr<IPlatformAppShell> CreatePlatformAppShell(ShellPlatformServices services) {
#if defined(_WIN32)
    return std::make_unique<Win32PlatformAppShell>(std::move(services));
#else
    (void)services;
    return std::make_unique<NullPlatformAppShell>();
#endif
}

} // namespace mousefx::platform
