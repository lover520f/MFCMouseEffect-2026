#pragma once

#include <memory>

#include "MouseFx/Core/Shell/IDpiAwarenessService.h"
#include "MouseFx/Core/Shell/IEventLoopService.h"
#include "MouseFx/Core/Shell/ISettingsLauncher.h"
#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"
#include "MouseFx/Core/Shell/ITrayService.h"
#include "MouseFx/Core/Shell/IUserNotificationService.h"

namespace mousefx {

// Platform service bundle, similar to Flutter's per-platform package wiring.
struct ShellPlatformServices {
    std::unique_ptr<ITrayService> trayService{};
    std::unique_ptr<ISettingsLauncher> settingsLauncher{};
    std::unique_ptr<ISingleInstanceGuard> singleInstanceGuard{};
    std::unique_ptr<IDpiAwarenessService> dpiAwarenessService{};
    std::unique_ptr<IEventLoopService> eventLoopService{};
    std::unique_ptr<IUserNotificationService> notifier{};
};

} // namespace mousefx
