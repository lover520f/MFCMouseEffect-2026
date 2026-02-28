#include "pch.h"

#include "Platform/macos/Shell/MacosTrayService.h"

#include "MouseFx/Core/Shell/IAppShellHost.h"
#include "Platform/macos/Shell/MacosTrayMenuFactory.h"
#include "Platform/macos/Shell/MacosTrayMenuLocalization.h"

namespace mousefx {

#if defined(__APPLE__)

struct MacosTrayService::Impl {
    IAppShellHost* host = nullptr;
    macos_tray::MacosTrayMenuObjects tray{};
    bool started = false;
};

MacosTrayService::MacosTrayService()
    : impl_(std::make_unique<Impl>()) {}

MacosTrayService::~MacosTrayService() {
    Stop();
}

bool MacosTrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    if (!impl_ || host == nullptr) {
        return false;
    }
    if (impl_->started) {
        return true;
    }

    impl_->host = host;
    if (!showTrayIcon) {
        impl_->started = true;
        return true;
    }

    const MacosTrayMenuText menuText = ResolveMacosTrayMenuText();
    const bool started = macos_tray::BuildMacosTrayMenu(host, menuText, &impl_->tray);
    if (!started) {
        Stop();
        return false;
    }

    impl_->started = true;
    return true;
}

void MacosTrayService::Stop() {
    if (!impl_ || !impl_->started) {
        return;
    }

    macos_tray::ReleaseMacosTrayMenu(&impl_->tray);

    impl_->host = nullptr;
    impl_->started = false;
}

void MacosTrayService::RequestExit() {
    if (!impl_ || !impl_->started) {
        return;
    }

    // Remove the status item immediately so users do not see a stale tray icon
    // while shutdown is draining.
    macos_tray::ReleaseMacosTrayMenu(&impl_->tray);
    macos_tray::TerminateMacosTrayApp();

    impl_->host = nullptr;
    impl_->started = false;
}

#else

struct MacosTrayService::Impl {};

MacosTrayService::MacosTrayService()
    : impl_(std::make_unique<Impl>()) {}

MacosTrayService::~MacosTrayService() = default;

bool MacosTrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    (void)showTrayIcon;
    return host != nullptr;
}

void MacosTrayService::Stop() {
}

void MacosTrayService::RequestExit() {
}

#endif

} // namespace mousefx
