#include "pch.h"
#include "WebSettingsServer.TestAutomationShortcutApiRoutes.h"

#include <cstdint>
#include <limits>
#include <string>

#include "MouseFx/Core/Automation/ShortcutTextFormatter.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationRouteUtils.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestRouteCommon.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/System/MacosVirtualKeyMapper.h"
#endif

using json = nlohmann::json;

namespace mousefx {
using websettings_test_automation::IsAutomationShortcutTestApiEnabled;
using websettings_test_routes::ParseBooleanOrDefault;
using websettings_test_routes::ParseInt32OrDefault;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

namespace {

uint32_t ResolveVirtualKeyFromMacKeyCode(uint16_t macKeyCode) {
#if MFX_PLATFORM_MACOS
    return macos_keymap::VirtualKeyFromMacKeyCode(macKeyCode);
#else
    (void)macKeyCode;
    return 0;
#endif
}

} // namespace

bool HandleWebSettingsTestAutomationShortcutApiRoute(
    const HttpRequest& req,
    const std::string& path,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/automation/test-shortcut-from-mac-keycode") {
        if (!IsAutomationShortcutTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const int32_t rawMacKeyCode = ParseInt32OrDefault(payload, "mac_key_code", -1);
        const bool validMacKeyCode =
            rawMacKeyCode >= 0 &&
            rawMacKeyCode <= static_cast<int32_t>(std::numeric_limits<uint16_t>::max());
        const uint32_t vkCode = validMacKeyCode
            ? ResolveVirtualKeyFromMacKeyCode(static_cast<uint16_t>(rawMacKeyCode))
            : 0;

        KeyEvent event{};
        event.vkCode = vkCode;
        event.ctrl = ParseBooleanOrDefault(payload, "ctrl", false);
        event.shift = ParseBooleanOrDefault(payload, "shift", false);
        event.alt = ParseBooleanOrDefault(payload, "alt", false);
        event.win = ParseBooleanOrDefault(
            payload,
            "cmd",
            ParseBooleanOrDefault(payload, "win", false));
        event.meta = event.win;
        event.systemKey = event.alt || event.meta;

        const bool supported = validMacKeyCode && event.vkCode != 0;
        const std::string shortcut = supported ? shortcut_text::FormatShortcutText(event) : std::string{};
        SetJsonResponse(resp, json({
            {"ok", true},
            {"mac_key_code", rawMacKeyCode},
            {"vk_code", event.vkCode},
            {"supported", supported},
            {"reason", supported ? std::string{} : std::string("invalid_or_unmapped_keycode")},
            {"shortcut", shortcut},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
