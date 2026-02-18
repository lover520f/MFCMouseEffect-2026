#include "pch.h"
#include "EffectConfigInternal.h"

#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace mousefx::config_internal {

std::string ArgbToHex(Argb color) {
    std::ostringstream ss;
    ss << '#' << std::uppercase << std::setfill('0') << std::hex << std::setw(8) << color.value;
    return ss.str();
}

std::string WStringToUtf8(const std::wstring& ws) {
    if (ws.empty()) {
        return {};
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return {};
    }

    std::string out(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.data(), len, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') {
        out.pop_back();
    }
    return out;
}

std::string NormalizeHoldFollowMode(std::string mode) {
    mode = ToLowerAscii(mode);
    if (mode == "precise") {
        return "precise";
    }
    if (mode == "efficient") {
        return "efficient";
    }
    return "smooth";
}

std::string NormalizeHoldPresenterBackend(std::string backend) {
    backend = ToLowerAscii(TrimAscii(backend));
    if (backend.empty()) {
        return "auto";
    }
    return backend;
}

TrailHistoryProfile SanitizeTrailHistoryProfile(TrailHistoryProfile profile) {
    if (profile.durationMs < 80) {
        profile.durationMs = 80;
    }
    if (profile.durationMs > 2000) {
        profile.durationMs = 2000;
    }
    if (profile.maxPoints < 2) {
        profile.maxPoints = 2;
    }
    if (profile.maxPoints > 240) {
        profile.maxPoints = 240;
    }
    return profile;
}

TrailRendererParamsConfig SanitizeTrailParams(TrailRendererParamsConfig params) {
    params.streamer.glowWidthScale = ClampFloat(params.streamer.glowWidthScale, 0.5f, 4.0f);
    params.streamer.coreWidthScale = ClampFloat(params.streamer.coreWidthScale, 0.2f, 2.0f);
    params.streamer.headPower = ClampFloat(params.streamer.headPower, 0.8f, 3.0f);

    params.electric.amplitudeScale = ClampFloat(params.electric.amplitudeScale, 0.2f, 3.0f);
    params.electric.forkChance = ClampFloat(params.electric.forkChance, 0.0f, 0.5f);

    params.meteor.sparkRateScale = ClampFloat(params.meteor.sparkRateScale, 0.2f, 4.0f);
    params.meteor.sparkSpeedScale = ClampFloat(params.meteor.sparkSpeedScale, 0.2f, 4.0f);

    if (params.idleFade.startMs < 0) {
        params.idleFade.startMs = 0;
    }
    if (params.idleFade.endMs < 0) {
        params.idleFade.endMs = 0;
    }
    if (params.idleFade.startMs > 3000) {
        params.idleFade.startMs = 3000;
    }
    if (params.idleFade.endMs > 6000) {
        params.idleFade.endMs = 6000;
    }

    return params;
}

InputIndicatorConfig SanitizeInputIndicatorConfig(InputIndicatorConfig config) {
    config.positionMode = (config.positionMode == "absolute") ? "absolute" : "relative";
    config.offsetX = ClampInt(config.offsetX, -2000, 2000);
    config.offsetY = ClampInt(config.offsetY, -2000, 2000);
    config.absoluteX = ClampInt(config.absoluteX, -20000, 20000);
    config.absoluteY = ClampInt(config.absoluteY, -20000, 20000);

    if (config.keyDisplayMode != "all" &&
        config.keyDisplayMode != "significant" &&
        config.keyDisplayMode != "shortcut") {
        config.keyDisplayMode = "all";
    }
    config.keyLabelLayoutMode = ToLowerAscii(TrimAscii(config.keyLabelLayoutMode));
    if (config.keyLabelLayoutMode != "fixed_font" &&
        config.keyLabelLayoutMode != "fixed_area") {
        config.keyLabelLayoutMode = "fixed_font";
    }

    for (auto& [key, value] : config.perMonitorOverrides) {
        (void)key;
        value.absoluteX = ClampInt(value.absoluteX, -20000, 20000);
        value.absoluteY = ClampInt(value.absoluteY, -20000, 20000);
    }

    config.sizePx = ClampInt(config.sizePx, 40, 200);
    config.durationMs = ClampInt(config.durationMs, 120, 2000);
    return config;
}

namespace {

std::string NormalizeId(std::string value) {
    value = ToLowerAscii(TrimAscii(value));
    std::replace(value.begin(), value.end(), '-', '_');
    std::replace(value.begin(), value.end(), ' ', '_');
    return value;
}

std::string NormalizeMouseActionId(std::string value) {
    value = NormalizeId(std::move(value));
    if (value == "left" || value == "leftclick" || value == "lclick") return "left_click";
    if (value == "right" || value == "rightclick" || value == "rclick") return "right_click";
    if (value == "middle" || value == "middleclick" || value == "mclick") return "middle_click";
    if (value == "wheel_up" || value == "scrollup") return "scroll_up";
    if (value == "wheel_down" || value == "scrolldown") return "scroll_down";
    return value;
}

std::string NormalizeGestureId(std::string value) {
    return NormalizeId(std::move(value));
}

std::string NormalizeProcessExecutable(std::string value) {
    value = ToLowerAscii(TrimAscii(std::move(value)));
    if (value.empty()) {
        return {};
    }

    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    if (value.empty()) {
        return {};
    }

    std::replace(value.begin(), value.end(), '/', '\\');
    const size_t slashPos = value.find_last_of('\\');
    if (slashPos != std::string::npos) {
        value = value.substr(slashPos + 1);
    }
    value = TrimAscii(std::move(value));
    if (value.empty()) {
        return {};
    }

    if (value.find('.') == std::string::npos) {
        value += ".exe";
    }
    return value;
}

std::string NormalizeAutomationAppScopeToken(std::string value) {
    value = ToLowerAscii(TrimAscii(std::move(value)));
    if (value.empty() || value == "all" || value == "global" || value == "*") {
        return "all";
    }

    constexpr const char kProcessPrefix[] = "process:";
    if (value.rfind(kProcessPrefix, 0) == 0) {
        std::string exe = NormalizeProcessExecutable(value.substr(sizeof(kProcessPrefix) - 1));
        if (exe.empty()) {
            return "all";
        }
        return std::string(kProcessPrefix) + exe;
    }

    std::string exe = NormalizeProcessExecutable(std::move(value));
    if (exe.empty()) {
        return "all";
    }
    return std::string(kProcessPrefix) + exe;
}

std::vector<std::string> NormalizeAutomationAppScopes(std::vector<std::string> values) {
    std::vector<std::string> out;
    out.reserve(values.size());

    for (std::string& value : values) {
        const std::string normalized = NormalizeAutomationAppScopeToken(std::move(value));
        if (normalized == "all") {
            return {"all"};
        }
        if (std::find(out.begin(), out.end(), normalized) == out.end()) {
            out.push_back(normalized);
        }
    }

    if (out.empty()) {
        out.push_back("all");
    }
    return out;
}

std::string NormalizeGestureButton(std::string value) {
    value = NormalizeId(std::move(value));
    if (value == "l" || value == "left_button") return "left";
    if (value == "m" || value == "middle_button") return "middle";
    if (value == "r" || value == "right_button") return "right";
    if (value != "left" && value != "middle" && value != "right") {
        return "right";
    }
    return value;
}

} // namespace

InputAutomationConfig SanitizeInputAutomationConfig(InputAutomationConfig config) {
    auto sanitizeBindingList = [](std::vector<AutomationKeyBinding>* bindings, bool gestureBinding) {
        if (!bindings) {
            return;
        }
        for (AutomationKeyBinding& binding : *bindings) {
            binding.trigger = gestureBinding
                ? automation_chain::NormalizeChainText(binding.trigger, NormalizeGestureId)
                : automation_chain::NormalizeChainText(binding.trigger, NormalizeMouseActionId);
            binding.appScopes = NormalizeAutomationAppScopes(std::move(binding.appScopes));
            binding.keys = TrimAscii(binding.keys);
        }
    };

    sanitizeBindingList(&config.mouseMappings, false);
    config.gesture.triggerButton = NormalizeGestureButton(std::move(config.gesture.triggerButton));
    config.gesture.minStrokeDistancePx = ClampInt(config.gesture.minStrokeDistancePx, 10, 4000);
    config.gesture.sampleStepPx = ClampInt(config.gesture.sampleStepPx, 2, 256);
    config.gesture.maxDirections = ClampInt(config.gesture.maxDirections, 1, 8);
    sanitizeBindingList(&config.gesture.mappings, true);
    return config;
}

WasmConfig SanitizeWasmConfig(WasmConfig config) {
    config.manifestPath = TrimAscii(config.manifestPath);
    return config;
}

std::string ReadFileAsUtf8(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string raw = ss.str();
    if (raw.empty()) {
        return "";
    }

    if (raw.size() >= 3 &&
        static_cast<unsigned char>(raw[0]) == 0xEF &&
        static_cast<unsigned char>(raw[1]) == 0xBB &&
        static_cast<unsigned char>(raw[2]) == 0xBF) {
        raw = raw.substr(3);
    }

    int utf8Result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw.c_str(), -1, nullptr, 0);
    if (utf8Result > 0) {
        return raw;
    }

    int wlen = MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        return raw;
    }

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, &wstr[0], wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen <= 0) {
        return raw;
    }

    std::string utf8(static_cast<size_t>(ulen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);
    if (!utf8.empty() && utf8.back() == '\0') {
        utf8.pop_back();
    }
    return utf8;
}

} // namespace mousefx::config_internal
