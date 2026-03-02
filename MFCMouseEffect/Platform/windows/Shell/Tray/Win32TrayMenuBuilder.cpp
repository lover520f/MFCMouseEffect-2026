#include "pch.h"

#include "Platform/windows/Shell/Tray/Win32TrayMenuBuilder.h"
#include "Platform/windows/Shell/Tray/Win32TrayMenuCommands.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "Settings/SettingsOptions.h"

#include <mutex>
#include <unordered_map>

using namespace mousefx;

namespace {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static bool IsZhUi(mousefx::AppController* mouseFx) {
    if (!mouseFx) return true;
    std::string lang = ToLowerAscii(mouseFx->Config().uiLanguage);
    if (lang.empty()) return true;
    return lang.rfind("zh", 0) == 0;
}

static std::wstring PickLabel(const wchar_t* zh, const wchar_t* en, bool isZh) {
    if (zh && en) {
        return isZh ? std::wstring(zh) : std::wstring(en);
    }
    if (zh) return std::wstring(zh);
    if (en) return std::wstring(en);
    return std::wstring();
}

static bool TryBuildThemeMenuCommand(const std::string& themeValue, UINT* outCmd) {
    if (!outCmd) return false;
    const std::string normalized = mousefx::NormalizeThemeName(themeValue);
    if (normalized == "chromatic") {
        *outCmd = kCmdThemeChromatic;
        return true;
    }
    if (normalized == "scifi") {
        *outCmd = kCmdThemeSciFi;
        return true;
    }
    if (normalized == "neon") {
        *outCmd = kCmdThemeNeon;
        return true;
    }
    if (normalized == "minimal") {
        *outCmd = kCmdThemeMinimal;
        return true;
    }
    if (normalized == "game") {
        *outCmd = kCmdThemeGame;
        return true;
    }
    return false;
}

constexpr UINT kDynamicThemeCmdBase = 8100;
constexpr UINT kDynamicThemeCmdMax = 8299;

std::mutex& DynamicThemeMenuMapMutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_map<UINT, std::string>& DynamicThemeMenuMap() {
    static std::unordered_map<UINT, std::string> map;
    return map;
}

void ResetDynamicThemeMenuMap() {
    std::lock_guard<std::mutex> guard(DynamicThemeMenuMapMutex());
    DynamicThemeMenuMap().clear();
}

void RegisterDynamicThemeMenuItem(UINT cmd, const std::string& themeValue) {
    std::lock_guard<std::mutex> guard(DynamicThemeMenuMapMutex());
    DynamicThemeMenuMap()[cmd] = themeValue;
}

bool TryReadDynamicThemeMenuItem(UINT cmd, std::string* outTheme) {
    if (!outTheme) return false;
    std::lock_guard<std::mutex> guard(DynamicThemeMenuMapMutex());
    const auto it = DynamicThemeMenuMap().find(cmd);
    if (it == DynamicThemeMenuMap().end()) {
        return false;
    }
    *outTheme = it->second;
    return true;
}

static std::wstring FallbackOptionLabel(UINT cmd, bool zh) {
    switch (cmd) {
    case kCmdClickRipple: return PickLabel(L"\u6C34\u6CE2\u7EB9", L"Ripple", zh);
    case kCmdClickStar: return PickLabel(L"\u661F\u661F", L"Star", zh);
    case kCmdClickText: return PickLabel(L"\u98D8\u6D6E\u6587\u5B57", L"Text", zh);
    case kCmdClickNone: return PickLabel(L"\u65E0", L"None", zh);

    case kCmdTrailMeteor: return PickLabel(L"\u7D62\u4E3D\u6D41\u661F", L"Meteor Shower", zh);
    case kCmdTrailStreamer: return PickLabel(L"\u9713\u8679\u6D41\u5149", L"Neon Streamer", zh);
    case kCmdTrailElectric: return PickLabel(L"\u8D5B\u535A\u7535\u5F27", L"Cyber Electric", zh);
    case kCmdTrailTubes: return PickLabel(L"\u79D1\u5E7B\u7BA1\u9053", L"Sci-Fi Tubes", zh);
    case kCmdTrailParticle: return PickLabel(L"\u5F69\u8679\u7C92\u5B50", L"Particle", zh);
    case kCmdTrailLine: return PickLabel(L"\u666E\u901A\u7EBF\u6761", L"Line", zh);
    case kCmdTrailNone: return PickLabel(L"\u65E0", L"None", zh);

    case kCmdScrollArrow: return PickLabel(L"\u65B9\u5411\u6307\u793A", L"Arrow", zh);
    case kCmdScrollHelix: return PickLabel(L"3D \u53CC\u87BA\u65CB", L"3D Helix", zh);
    case kCmdScrollTwinkle: return PickLabel(L"\u661F\u5C18\u55B7\u6D41", L"Stardust Stream", zh);
    case kCmdScrollNone: return PickLabel(L"\u65E0", L"None", zh);

    case kCmdHoldCharge: return PickLabel(L"\u84C4\u529B (\u80FD\u91CF\u73AF)", L"Energy Charge", zh);
    case kCmdHoldLightning: return PickLabel(L"\u95EA\u7535 (\u5947\u70B9)", L"Lightning", zh);
    case kCmdHoldHex: return PickLabel(L"\u516D\u8FB9\u5F62 (\u62A4\u76FE)", L"Hex Shield", zh);
    case kCmdHoldTechRing: return PickLabel(L"\u79D1\u6280\u5708 (3D)", L"Tech Ring (3D)", zh);
    case kCmdHoldHologram: return PickLabel(L"\u516E\u606F\u6295\u5F71 (3D)", L"Hologram (3D)", zh);
    case kCmdHoldNeon3D: return PickLabel(L"\u9713\u8679 HUD (3D)", L"Neon HUD (3D)", zh);
    case kCmdHoldQuantumHaloGpuV2: return PickLabel(L"\u91CF\u5B50\u5149\u73AF GPU", L"Quantum Halo GPU", zh);
    case kCmdHoldFluxFieldCpu: return PickLabel(L"\u78C1\u901A\u573A HUD CPU", L"FluxField HUD CPU", zh);
    case kCmdHoldFluxFieldGpuV2: return PickLabel(L"\u78C1\u901A\u573A HUD GPU", L"FluxField HUD GPU", zh);
    case kCmdHoldNone: return PickLabel(L"\u65E0", L"None", zh);

    case kCmdHoverGlow: return PickLabel(L"\u547C\u5438\u706F", L"Glow", zh);
    case kCmdHoverTubes: return PickLabel(L"\u87BA\u65CB\u60AC\u6D6E", L"Helix Suspension", zh);
    case kCmdHoverNone: return PickLabel(L"\u65E0", L"None", zh);
    default: return L"(Unnamed)";
    }
}

static bool TryReadOptionCommand(const mousefx::EffectOption* option, UINT* outCmd) {
    if (!option || !outCmd) return false;
    __try {
        *outCmd = option->trayCmd;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

static bool IsCurrentTypeMatchByCommand(const std::string& currentType, UINT cmd) {
    switch (cmd) {
    case kCmdClickRipple: return currentType == "ripple";
    case kCmdClickStar: return currentType == "star";
    case kCmdClickText: return currentType == "text";
    case kCmdClickNone: return currentType == "none";

    case kCmdTrailMeteor: return currentType == "meteor";
    case kCmdTrailStreamer: return currentType == "streamer";
    case kCmdTrailElectric: return currentType == "electric";
    case kCmdTrailTubes: return currentType == "tubes";
    case kCmdTrailParticle: return currentType == "particle";
    case kCmdTrailLine: return currentType == "line";
    case kCmdTrailNone: return currentType == "none";

    case kCmdScrollArrow: return currentType == "arrow";
    case kCmdScrollHelix: return currentType == "helix";
    case kCmdScrollTwinkle: return currentType == "twinkle";
    case kCmdScrollNone: return currentType == "none";

    case kCmdHoldCharge: return currentType == "charge";
    case kCmdHoldLightning: return currentType == "lightning";
    case kCmdHoldHex: return currentType == "hex";
    case kCmdHoldTechRing: return currentType == "tech_ring";
    case kCmdHoldHologram: return currentType == "hologram" || currentType == "scifi3d";
    case kCmdHoldNeon3D: return currentType == "hold_neon3d" || currentType == "neon3d";
    case kCmdHoldQuantumHaloGpuV2:
        return currentType == "hold_quantum_halo_gpu_v2" || currentType == "hold_neon3d_gpu_v2";
    case kCmdHoldFluxFieldCpu: return currentType == mousefx::hold_route::kTypeFluxFieldCpu;
    case kCmdHoldFluxFieldGpuV2: return currentType == mousefx::hold_route::kTypeFluxFieldGpuV2;
    case kCmdHoldNone: return currentType == "none";

    case kCmdHoverGlow: return currentType == "glow";
    case kCmdHoverTubes: return currentType == "tubes" || currentType == "suspension";
    case kCmdHoverNone: return currentType == "none";
    default: return false;
    }
}

static std::string GetCurrentEffectType(mousefx::AppController* mouseFx, mousefx::EffectCategory category) {
    if (!mouseFx) return "";
    const auto cfg = mouseFx->GetConfigSnapshot();
    switch (category) {
    case mousefx::EffectCategory::Click:
        return cfg.active.click;
    case mousefx::EffectCategory::Trail:
        return cfg.active.trail;
    case mousefx::EffectCategory::Scroll:
        return cfg.active.scroll;
    case mousefx::EffectCategory::Hold:
        return cfg.active.hold;
    case mousefx::EffectCategory::Hover:
        return cfg.active.hover;
    default:
        return "";
    }
}

static void AppendEffectSubMenu(HMENU parent,
                                const std::wstring& title,
                                mousefx::AppController* mouseFx,
                                mousefx::EffectCategory category,
                                const mousefx::EffectOption* opts,
                                size_t count) {
    if (!::IsMenu(parent)) return;
    if (!opts || count == 0 || count > 256) return;

    HMENU sub = CreatePopupMenu();
    if (!sub) return;

    const std::string currentType = GetCurrentEffectType(mouseFx, category);
    const bool zh = IsZhUi(mouseFx);
    for (size_t i = 0; i < count; ++i) {
        const mousefx::EffectOption* option = &opts[i];
        UINT cmd = 0;
        if (!TryReadOptionCommand(option, &cmd) || cmd == 0) {
            continue;
        }

        const std::wstring label = FallbackOptionLabel(cmd, zh);
        if (!AppendMenuW(sub, MF_STRING | MF_BYPOSITION, cmd, label.c_str())) {
            continue;
        }
        if (IsCurrentTypeMatchByCommand(currentType, cmd)) {
            CheckMenuItem(sub, cmd, MF_BYCOMMAND | MF_CHECKED);
        }
    }

    if (!AppendMenuW(parent, MF_POPUP | MF_BYPOSITION, reinterpret_cast<UINT_PTR>(sub), title.c_str())) {
        DestroyMenu(sub);
    }
}

static void AppendThemeSubMenu(HMENU parent, mousefx::AppController* mouseFx) {
    if (!::IsMenu(parent)) return;

    HMENU themeMenu = CreatePopupMenu();
    if (!themeMenu) return;

    ResetDynamicThemeMenuMap();

    const bool zh = IsZhUi(mouseFx);
    UINT nextDynamicCmd = kDynamicThemeCmdBase;
    std::string currentThemeNormalized = "neon";
    if (mouseFx) {
        const std::string configuredTheme = mouseFx->Config().theme;
        const std::string resolvedTheme = mousefx::ResolveRuntimeThemeName(configuredTheme);
        currentThemeNormalized = mousefx::NormalizeThemeName(resolvedTheme.empty() ? configuredTheme : resolvedTheme);
        if (currentThemeNormalized.empty()) {
            currentThemeNormalized = "neon";
        }
    }
    UINT checkedCmd = kCmdThemeNeon;
    bool hasCheckedCmd = false;

    for (const auto& option : mousefx::GetThemeOptions()) {
        UINT cmd = 0;
        if (!TryBuildThemeMenuCommand(option.value, &cmd)) {
            if (nextDynamicCmd > kDynamicThemeCmdMax) {
                continue;
            }
            cmd = nextDynamicCmd++;
            RegisterDynamicThemeMenuItem(cmd, option.value);
        }
        std::wstring label = zh ? option.labelZh : option.labelEn;
        if (label.empty()) {
            label = std::wstring(option.value.begin(), option.value.end());
        }
        if (AppendMenuW(themeMenu, MF_STRING, cmd, label.c_str())) {
            if (mousefx::NormalizeThemeName(option.value) == currentThemeNormalized) {
                checkedCmd = cmd;
                hasCheckedCmd = true;
            }
        }
    }

    if (hasCheckedCmd) {
        CheckMenuItem(themeMenu, checkedCmd, MF_BYCOMMAND | MF_CHECKED);
    }

    if (!AppendMenuW(parent, MF_POPUP | MF_BYPOSITION, reinterpret_cast<UINT_PTR>(themeMenu),
                     PickLabel(L"\u4e3b\u9898", L"Theme", zh).c_str())) {
        DestroyMenu(themeMenu);
    }
}

static bool TryBuildEffectJsonByCommand(UINT cmd, std::string* outJson) {
    if (!outJson) return true;

    auto setEffect = [&](const char* category, const char* effectType) {
        *outJson = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category + "\",\"type\":\"" + effectType + "\"}";
    };
    auto clearEffect = [&](const char* category) {
        *outJson = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
    };

    switch (cmd) {
    case kCmdClickRipple: setEffect("click", "ripple"); return true;
    case kCmdClickStar: setEffect("click", "star"); return true;
    case kCmdClickText: setEffect("click", "text"); return true;
    case kCmdClickNone: clearEffect("click"); return true;

    case kCmdTrailMeteor: setEffect("trail", "meteor"); return true;
    case kCmdTrailStreamer: setEffect("trail", "streamer"); return true;
    case kCmdTrailElectric: setEffect("trail", "electric"); return true;
    case kCmdTrailTubes: setEffect("trail", "tubes"); return true;
    case kCmdTrailParticle: setEffect("trail", "particle"); return true;
    case kCmdTrailLine: setEffect("trail", "line"); return true;
    case kCmdTrailNone: clearEffect("trail"); return true;

    case kCmdScrollArrow: setEffect("scroll", "arrow"); return true;
    case kCmdScrollHelix: setEffect("scroll", "helix"); return true;
    case kCmdScrollTwinkle: setEffect("scroll", "twinkle"); return true;
    case kCmdScrollNone: clearEffect("scroll"); return true;

    case kCmdHoldCharge: setEffect("hold", "charge"); return true;
    case kCmdHoldLightning: setEffect("hold", "lightning"); return true;
    case kCmdHoldHex: setEffect("hold", "hex"); return true;
    case kCmdHoldTechRing: setEffect("hold", "tech_ring"); return true;
    case kCmdHoldHologram: setEffect("hold", "hologram"); return true;
    case kCmdHoldNeon3D: setEffect("hold", "hold_neon3d"); return true;
    case kCmdHoldQuantumHaloGpuV2: setEffect("hold", "hold_quantum_halo_gpu_v2"); return true;
    case kCmdHoldFluxFieldCpu: setEffect("hold", mousefx::hold_route::kTypeFluxFieldCpu); return true;
    case kCmdHoldFluxFieldGpuV2: setEffect("hold", mousefx::hold_route::kTypeFluxFieldGpuV2); return true;
    case kCmdHoldNone: clearEffect("hold"); return true;

    case kCmdHoverGlow: setEffect("hover", "glow"); return true;
    case kCmdHoverTubes: setEffect("hover", "tubes"); return true;
    case kCmdHoverNone: clearEffect("hover"); return true;
    default:
        break;
    }
    return false;
}

} // namespace

void Win32TrayMenuBuilder::BuildTrayMenu(HMENU menu, mousefx::AppController* mouseFx) {
    if (!::IsMenu(menu)) return;

    size_t n = 0;
    const bool zh = IsZhUi(mouseFx);

    const mousefx::EffectOption* clickOpts = mousefx::ClickMetadata(n);
    AppendEffectSubMenu(menu, PickLabel(L"\u70b9\u51fb\u7279\u6548", L"Click Effects", zh), mouseFx, mousefx::EffectCategory::Click,
                        clickOpts, n);

    const mousefx::EffectOption* trailOpts = mousefx::TrailMetadata(n);
    AppendEffectSubMenu(menu, PickLabel(L"\u62d6\u5c3e\u7279\u6548", L"Trail Effects", zh), mouseFx, mousefx::EffectCategory::Trail,
                        trailOpts, n);

    const mousefx::EffectOption* scrollOpts = mousefx::ScrollMetadata(n);
    AppendEffectSubMenu(menu, PickLabel(L"\u6eda\u8f6e\u7279\u6548", L"Scroll Effects", zh), mouseFx, mousefx::EffectCategory::Scroll,
                        scrollOpts, n);

    const mousefx::EffectOption* holdOpts = mousefx::HoldMetadata(n);
    AppendEffectSubMenu(menu, PickLabel(L"\u957f\u6309\u7279\u6548", L"Hold Effects", zh), mouseFx, mousefx::EffectCategory::Hold,
                        holdOpts, n);

    const mousefx::EffectOption* hoverOpts = mousefx::HoverMetadata(n);
    AppendEffectSubMenu(menu, PickLabel(L"\u60ac\u505c\u7279\u6548", L"Hover Effects", zh), mouseFx, mousefx::EffectCategory::Hover,
                        hoverOpts, n);

    AppendThemeSubMenu(menu, mouseFx);

    AppendMenuW(menu, MF_STRING, kCmdStarRepo, PickLabel(L"\u2605 Star \u9879\u76EE", L"\u2605 Star Project", zh).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdTraySettings, PickLabel(L"\u8BBE\u7F6E...", L"Settings...", zh).c_str());
    AppendMenuW(menu, MF_STRING, kCmdTrayReloadConfig, PickLabel(L"\u91CD\u8F7D\u914D\u7F6E", L"Reload config", zh).c_str());
    AppendMenuW(menu, MF_STRING, kCmdTrayExit, PickLabel(L"\u9000\u51FA", L"Exit", zh).c_str());
}

bool Win32TrayMenuBuilder::TryBuildIpcJson(UINT cmd, std::string* outJson) {
    if (cmd == kCmdTrayReloadConfig) {
        if (outJson) *outJson = "{\"cmd\":\"reload_config\"}";
        return true;
    }
    return TryBuildEffectJsonByCommand(cmd, outJson);
}

bool Win32TrayMenuBuilder::TryBuildTheme(UINT cmd, std::string* outTheme) {
    if (!outTheme) return false;
    switch (cmd) {
    case kCmdThemeChromatic: *outTheme = "chromatic"; return true;
    case kCmdThemeSciFi: *outTheme = "scifi"; return true;
    case kCmdThemeNeon: *outTheme = "neon"; return true;
    case kCmdThemeMinimal: *outTheme = "minimal"; return true;
    case kCmdThemeGame: *outTheme = "game"; return true;
    default:
        break;
    }
    return TryReadDynamicThemeMenuItem(cmd, outTheme);
}
