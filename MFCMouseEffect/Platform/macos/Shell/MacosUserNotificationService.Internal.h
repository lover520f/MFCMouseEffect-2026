#pragma once

#include <string>

namespace mousefx::macos_notification_detail {

std::string EscapeForAppleScriptString(const std::string& value);
void AppendTestNotificationCapture(const std::string& titleUtf8, const std::string& messageUtf8);
bool ShowWarningViaAppleScript(const std::string& safeTitle, const std::string& safeMessage);

} // namespace mousefx::macos_notification_detail
