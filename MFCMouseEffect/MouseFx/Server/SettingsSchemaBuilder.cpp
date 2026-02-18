// SettingsSchemaBuilder.cpp -- Settings schema JSON extracted from WebSettingsServer

#include "pch.h"
#include "SettingsSchemaBuilder.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.h"
#include "MouseFx/ThirdParty/json.hpp"
#include "MouseFx/Utils/MonitorUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Settings/SettingsOptions.h"

using json = nlohmann::json;

namespace mousefx {

static std::string LabelByLang(const std::wstring& zh, const std::wstring& en, const std::string& lang) {
    const wchar_t* ws = (lang == "zh-CN") ? zh.c_str() : en.c_str();
    return EnsureUtf8(Utf16ToUtf8(ws));
}

static json MakeOpt(const char* value, const wchar_t* zh, const wchar_t* en, const std::string& lang) {
    json o;
    o["value"] = value ? value : "";
    std::string label = LabelByLang(zh ? std::wstring(zh) : L"", en ? std::wstring(en) : L"", lang);
    if (label.empty()) label = value ? value : "";
    o["label"] = label;
    return o;
}

std::string BuildSettingsSchemaJson(const EffectConfig& config) {
    const std::string lang = config.uiLanguage.empty() ? "zh-CN" : config.uiLanguage;

    json out;
    out["ui_languages"] = json::array({
        {{"value","zh-CN"},{"label", LabelByLang(L"\u4e2d\u6587", L"Chinese", lang)}},
        {{"value","en-US"},{"label", LabelByLang(L"\u82f1\u6587", L"English", lang)}}
    });

    out["themes"] = json::array({
        {{"value","chromatic"},{"label", LabelByLang(L"\u70ab\u5f69", L"Chromatic", lang)}},
        {{"value","neon"},{"label", LabelByLang(L"\u9713\u8679", L"Neon", lang)}},
        {{"value","scifi"},{"label", LabelByLang(L"\u79d1\u5e7b", L"Sci-Fi", lang)}},
        {{"value","minimal"},{"label", LabelByLang(L"\u6781\u7b80", L"Minimal", lang)}},
        {{"value","game"},{"label", LabelByLang(L"\u6e38\u620f\u611f", L"Game", lang)}}
    });

    out["hold_follow_modes"] = json::array({
        {{"value","precise"},{"label", LabelByLang(L"\u7cbe\u51c6\u8ddf\u968f\uff08\u4f4e\u5ef6\u8fdf\uff09", L"Precise (Low Latency)", lang)}},
        {{"value","smooth"},{"label", LabelByLang(L"\u5149\u6807\u4f18\u5148\uff08\u63a8\u8350\uff09", L"Cursor Priority (Recommended)", lang)}},
        {{"value","efficient"},{"label", LabelByLang(L"\u6027\u80fd\u4f18\u5148\uff08CPU\u53cb\u597d\uff09", L"Performance First (CPU Saver)", lang)}}
    });
    {
        json presenterBackends = json::array();
        presenterBackends.push_back({
            {"value", "auto"},
            {"label", LabelByLang(L"\u81ea\u52a8\u9009\u62e9\uff08\u63a8\u8350\uff09", L"Auto (Recommended)", lang)}
        });
        for (const auto& backend : QuantumHaloPresenterBackendRegistry::Instance().ListByPriority()) {
            presenterBackends.push_back({
                {"value", backend.name},
                {"label", backend.name}
            });
        }
        out["hold_presenter_backends"] = std::move(presenterBackends);
    }
    out["input_indicator_position_modes"] = json::array({
        {{"value","relative"},{"label", LabelByLang(L"\u76f8\u5bf9\u5149\u6807", L"Relative To Cursor", lang)}},
        {{"value","absolute"},{"label", LabelByLang(L"\u5c4f\u5e55\u7edd\u5bf9\u5750\u6807", L"Absolute Screen Position", lang)}}
    });
    out["key_display_modes"] = json::array({
        {{"value","all"},{"label", LabelByLang(L"\u663e\u793a\u5168\u90e8", L"Display All", lang)}},
        {{"value","significant"},{"label", LabelByLang(L"\u4ec5\u91cd\u8981\u6309\u952e (\u63a8\u8350)", L"Significant Keys Only (Recommended)", lang)}},
        {{"value","shortcut"},{"label", LabelByLang(L"\u4ec5\u5feb\u6377\u952e", L"Shortcuts Only", lang)}}
    });
    out["key_label_layout_modes"] = json::array({
        {{"value","fixed_font"},{"label", LabelByLang(L"\u56fa\u5b9a\u5b57\u4f53\u5927\u5c0f\uff08\u81ea\u52a8\u6269\u5c55\u663e\u793a\u533a\u57df\uff09", L"Fixed Font Size (Expand Area)", lang)}},
        {{"value","fixed_area"},{"label", LabelByLang(L"\u56fa\u5b9a\u663e\u793a\u533a\u57df\uff08\u81ea\u52a8\u7f29\u5c0f\u5b57\u4f53\uff09", L"Fixed Area (Shrink Font)", lang)}}
    });
    out["automation_mouse_actions"] = json::array({
        {{"value","left_click"},{"label", LabelByLang(L"\u5de6\u952e\u5355\u51fb", L"Left Click", lang)}},
        {{"value","right_click"},{"label", LabelByLang(L"\u53f3\u952e\u5355\u51fb", L"Right Click", lang)}},
        {{"value","middle_click"},{"label", LabelByLang(L"\u4e2d\u952e\u5355\u51fb", L"Middle Click", lang)}},
        {{"value","scroll_up"},{"label", LabelByLang(L"\u6eda\u8f6e\u5411\u4e0a", L"Wheel Up", lang)}},
        {{"value","scroll_down"},{"label", LabelByLang(L"\u6eda\u8f6e\u5411\u4e0b", L"Wheel Down", lang)}},
    });
    out["automation_app_scopes"] = json::array({
        {{"value","all"},{"label", LabelByLang(L"\u5168\u90e8\u5e94\u7528", L"All Apps", lang)}},
        {{"value","selected"},{"label", LabelByLang(L"\u6307\u5b9a\u5e94\u7528\uff08\u591a\u9009\uff09", L"Selected Apps (Multi)", lang)}},
    });
    out["automation_gesture_buttons"] = json::array({
        {{"value","right"},{"label", LabelByLang(L"\u53f3\u952e\u62d6\u62fd (\u63a8\u8350)", L"Right Drag (Recommended)", lang)}},
        {{"value","middle"},{"label", LabelByLang(L"\u4e2d\u952e\u62d6\u62fd", L"Middle Drag", lang)}},
        {{"value","left"},{"label", LabelByLang(L"\u5de6\u952e\u62d6\u62fd", L"Left Drag", lang)}},
    });
    out["automation_gesture_patterns"] = json::array({
        {{"value","up"},{"label", LabelByLang(L"\u5411\u4e0a", L"Up", lang)}},
        {{"value","down"},{"label", LabelByLang(L"\u5411\u4e0b", L"Down", lang)}},
        {{"value","left"},{"label", LabelByLang(L"\u5411\u5de6", L"Left", lang)}},
        {{"value","right"},{"label", LabelByLang(L"\u5411\u53f3", L"Right", lang)}},
        {{"value","up_right"},{"label", LabelByLang(L"\u5411\u4e0a\u540e\u5411\u53f3", L"Up Then Right", lang)}},
        {{"value","up_left"},{"label", LabelByLang(L"\u5411\u4e0a\u540e\u5411\u5de6", L"Up Then Left", lang)}},
        {{"value","down_right"},{"label", LabelByLang(L"\u5411\u4e0b\u540e\u5411\u53f3", L"Down Then Right", lang)}},
        {{"value","down_left"},{"label", LabelByLang(L"\u5411\u4e0b\u540e\u5411\u5de6", L"Down Then Left", lang)}},
    });

    // Enumerate connected monitors for target-monitor dropdown.
    {
        json tmOpts = json::array();
        tmOpts.push_back({{"value","cursor"},{"label", LabelByLang(L"\u8ddf\u968f\u5149\u6807\u6240\u5728\u5c4f\u5e55", L"Follow Cursor Screen", lang)}});
        tmOpts.push_back({{"value","primary"},{"label", LabelByLang(L"\u4e3b\u5c4f\u5e55", L"Primary Monitor", lang)}});
        tmOpts.push_back({{"value","custom"},{"label", LabelByLang(L"\u81ea\u5b9a\u4e49\u591a\u5c4f", L"Custom Multi-Screen", lang)}});
        auto monitors = mousefx::EnumMonitors();
        json monArr = json::array();
        for (const auto& m : monitors) {
            std::wstring monLabel = m.deviceName;
            if (m.isPrimary) monLabel += L" (Primary)";
            std::string monLabelUtf8 = Utf16ToUtf8(monLabel.c_str());
            tmOpts.push_back({{"value", m.id},{"label", monLabelUtf8}});
            monArr.push_back({
                {"id", m.id},
                {"name", Utf16ToUtf8(m.deviceName.c_str())},
                {"left", m.bounds.left}, {"top", m.bounds.top},
                {"right", m.bounds.right}, {"bottom", m.bounds.bottom},
                {"is_primary", m.isPrimary}
            });
        }
        out["target_monitor_options"] = tmOpts;
        out["monitors"] = monArr;
    }

    auto build = [&](const EffectOption* (*fn)(size_t&), const char* key) {
        size_t n = 0;
        const EffectOption* opts = fn(n);
        json arr = json::array();
        for (size_t i = 0; i < n; ++i) {
            arr.push_back(MakeOpt(opts[i].value, opts[i].displayZh, opts[i].displayEn, lang));
        }
        out["effects"][key] = arr;
    };

    build(mousefx::ClickMetadata, "click");
    build(mousefx::TrailMetadata, "trail");
    build(mousefx::ScrollMetadata, "scroll");
    build(mousefx::HoldMetadata, "hold");
    build(mousefx::HoverMetadata, "hover");

    out["wasm"] = {
        {"supported_api_version", 1},
        {"default_enabled", false},
        {"diagnostic_keys", json::array({
            "enabled",
            "runtime_backend",
            "runtime_fallback_reason",
            "plugin_loaded",
            "plugin_api_version",
            "active_plugin_id",
            "active_plugin_name",
            "active_manifest_path",
            "active_wasm_path",
            "last_call_duration_us",
            "last_output_bytes",
            "last_command_count",
            "last_call_exceeded_budget",
            "last_call_rejected_by_budget",
            "last_output_truncated_by_budget",
            "last_command_truncated_by_budget",
            "last_budget_reason",
            "last_parse_error",
            "last_error"
        })}
    };

    return out.dump();
}

} // namespace mousefx
