#include "pch.h"
#include "EffectConfigInternal.h"

#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <fstream>
#include <iomanip>
#include <sstream>

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

    for (auto& [key, value] : config.perMonitorOverrides) {
        (void)key;
        value.absoluteX = ClampInt(value.absoluteX, -20000, 20000);
        value.absoluteY = ClampInt(value.absoluteY, -20000, 20000);
    }

    config.sizePx = ClampInt(config.sizePx, 40, 200);
    config.durationMs = ClampInt(config.durationMs, 120, 2000);
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
