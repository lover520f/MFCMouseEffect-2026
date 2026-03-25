#include "pch.h"

#include "SettingsStateMapper.Diagnostics.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

std::string Utf8FromWide(const wchar_t* text) {
    return Utf16ToUtf8(text);
}

const char* InputCaptureReasonToCode(AppController::InputCaptureFailureReason reason) {
    using Reason = AppController::InputCaptureFailureReason;
    switch (reason) {
    case Reason::PermissionDenied:
        return "permission_denied";
    case Reason::Unsupported:
        return "unsupported";
    case Reason::StartFailed:
        return "start_failed";
    case Reason::None:
    default:
        return "none";
    }
}

} // namespace

json BuildInputCaptureState(const AppController* controller, const std::string& lang) {
    if (!controller) {
        return {};
    }

    const AppController::InputCaptureRuntimeStatus status = controller->InputCaptureStatus();
    json out;
    out["active"] = status.active;
    out["error"] = status.error;
    out["reason"] = InputCaptureReasonToCode(status.reason);
    out["degraded"] = !status.active;
    out["effects_suspended"] = controller->EffectsSuspendedByInputCapture();
#if MFX_PLATFORM_MACOS
    out["required_permissions"] = json::array({"accessibility", "input_monitoring"});
#endif

    if (status.active || status.reason == AppController::InputCaptureFailureReason::None) {
        return out;
    }

    json notice;
    notice["level"] = "warn";
    notice["reason"] = InputCaptureReasonToCode(status.reason);
    notice["error"] = status.error;

    const bool zh = (lang == "zh-CN");
    switch (status.reason) {
    case AppController::InputCaptureFailureReason::PermissionDenied:
        if (zh) {
            notice["message"] =
                Utf8FromWide(
                    L"\u672A\u6388\u4E88 macOS \u8F85\u52A9\u529F\u80FD\u548C\u8F93\u5165\u76D1\u63A7"
                    L"\u6743\u9650\uFF0C\u8F93\u5165\u91C7\u96C6\u5DF2\u964D\u7EA7\uFF1B"
                    L"\u6388\u6743\u540E\u65E0\u9700\u91CD\u542F\u3002");
        } else {
            notice["message"] =
                "Global input capture is degraded: macOS permissions are missing. "
                "Grant both Accessibility and Input Monitoring to MFCMouseEffect in "
                "System Settings > Privacy & Security. The app recovers automatically after "
                "permissions are restored; restart is not required.";
        }
        break;
    case AppController::InputCaptureFailureReason::Unsupported:
        if (zh) {
            notice["message"] =
                Utf8FromWide(
                    L"\u5F53\u524D\u5E73\u53F0\u6682\u4E0D\u652F\u6301\u5168\u5C40\u8F93\u5165\u91C7\u96C6"
                    L"\uFF0C\u5DF2\u81EA\u52A8\u964D\u7EA7\u4E3A\u4EC5\u4FDD\u7559\u53EF\u7528\u529F\u80FD\u3002");
        } else {
            notice["message"] =
                "Global input capture is not supported on this platform. "
                "The app is running in degraded mode with available features only.";
        }
        break;
    case AppController::InputCaptureFailureReason::StartFailed:
    default:
        if (zh) {
            notice["message"] =
                Utf8FromWide(
                    L"\u5168\u5C40\u8F93\u5165\u91C7\u96C6\u542F\u52A8\u5931\u8D25\uFF0C"
                    L"\u5F53\u524D\u4EE5\u964D\u7EA7\u6A21\u5F0F\u8FD0\u884C\u3002"
                    L"\u9519\u8BEF\u7801\uFF1A") +
                std::to_string(status.error) + Utf8FromWide(L"\u3002");
        } else {
            notice["message"] =
                std::string("Global input capture failed to start. Running in degraded mode. Error code: ") +
                std::to_string(status.error) + ".";
        }
        break;
    }

    out["notice"] = notice;
    return out;
}

} // namespace mousefx
