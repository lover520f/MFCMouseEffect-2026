#include "pch.h"

#include "SettingsStateMapper.Diagnostics.h"

#include "MouseFx/Core/Control/AppController.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

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
                "全局输入采集已降级：未授予 macOS 权限。请在“系统设置 > 隐私与安全性”中为 "
                "MFCMouseEffect 同时开启“辅助功能”和“输入监控”，然后重启应用。";
        } else {
            notice["message"] =
                "Global input capture is degraded: macOS permissions are missing. "
                "Grant both Accessibility and Input Monitoring to MFCMouseEffect in "
                "System Settings > Privacy & Security, then restart the app.";
        }
        break;
    case AppController::InputCaptureFailureReason::Unsupported:
        if (zh) {
            notice["message"] =
                "当前平台暂不支持全局输入采集，已自动降级为仅保留可用功能。";
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
                std::string("全局输入采集启动失败，当前以降级模式运行。错误码：") +
                std::to_string(status.error) + "。";
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
