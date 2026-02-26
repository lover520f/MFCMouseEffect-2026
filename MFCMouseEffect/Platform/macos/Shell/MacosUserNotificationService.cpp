#include "Platform/macos/Shell/MacosUserNotificationService.h"
#include "Platform/macos/Shell/MacosUserNotificationService.Internal.h"

#include <cstdio>
#include <string>

namespace mousefx {

void MacosUserNotificationService::ShowWarning(
    const std::string& titleUtf8,
    const std::string& messageUtf8) {
    const std::string safeTitle = macos_notification_detail::EscapeForAppleScriptString(
        titleUtf8.empty() ? "MFCMouseEffect" : titleUtf8);
    const std::string safeMessage = macos_notification_detail::EscapeForAppleScriptString(
        messageUtf8.empty() ? "(empty)" : messageUtf8);
    macos_notification_detail::AppendTestNotificationCapture(safeTitle, safeMessage);

    if (macos_notification_detail::ShowWarningViaAppleScript(safeTitle, safeMessage)) {
        return;
    }

    std::fprintf(stderr, "[mousefx][warn] %s: %s\n", safeTitle.c_str(), safeMessage.c_str());
}

} // namespace mousefx
