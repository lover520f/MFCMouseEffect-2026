#include "pch.h"

#include "TrayMenuBuilder.h"
#include "TrayMenuCommands.h"

#include "MouseFx/Core/AppController.h"
#include "Settings/SettingsOptions.h"

#include <vector>

using namespace mousefx;

namespace {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static std::string GetCurrentEffectType(mousefx::AppController* mouseFx, mousefx::EffectCategory category) {
    if (!mouseFx) return "";
    auto* effect = mouseFx->GetEffect(category);
    if (!effect) return "";
    const char* name = effect->TypeName();
    return name ? std::string(name) : std::string();
}

static void AppendEffectSubMenu(CMenu& parent, const TCHAR* title, mousefx::AppController* mouseFx,
                                mousefx::EffectCategory category, const mousefx::EffectOption* opts, size_t count) {
    CMenu sub;
    sub.CreatePopupMenu();

    const std::string currentType = GetCurrentEffectType(mouseFx, category);

    for (size_t i = 0; i < count; ++i) {
        const auto& it = opts[i];
        
        // Use English labels in Tray menu as secondary (or just stick to one)
        // Original code used a mix. Let's use localized labels if available.
        // Actually TrayMenuBuilder.cpp previously hardcoded "Label (English)".
        // I'll use displayEn as a fallback or combined.
        
        CString label;
        label.Format(L"%s (%s)", it.displayZh, it.displayEn);

        sub.AppendMenu(MF_STRING, it.trayCmd, label);
        if (it.IsMatch(currentType)) {
            sub.CheckMenuItem(it.trayCmd, MF_CHECKED);
        }
    }

    parent.AppendMenu(MF_POPUP, (UINT_PTR)sub.Detach(), title);
}

static void AppendThemeSubMenu(CMenu& parent, mousefx::AppController* mouseFx) {
    CMenu themeMenu;
    themeMenu.CreatePopupMenu();

    themeMenu.AppendMenu(MF_STRING, kCmdThemeChromatic, L"\u70ab\u5f69 (Chromatic)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeSciFi,     L"\u79d1\u5e7b (Sci-Fi)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeNeon,      L"\u9713\u8679 (Neon)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeMinimal,   L"\u6781\u7b80 (Minimal)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeGame,      L"\u6e38\u620f\u611f (Game)");

    if (mouseFx) {
        std::string theme = ToLowerAscii(mouseFx->Config().theme);
        if (theme == "scifi" || theme == "sci-fi" || theme == "sci_fi") themeMenu.CheckMenuItem(kCmdThemeSciFi, MF_CHECKED);
        else if (theme == "chromatic") themeMenu.CheckMenuItem(kCmdThemeChromatic, MF_CHECKED);
        else if (theme == "minimal") themeMenu.CheckMenuItem(kCmdThemeMinimal, MF_CHECKED);
        else if (theme == "game") themeMenu.CheckMenuItem(kCmdThemeGame, MF_CHECKED);
        else themeMenu.CheckMenuItem(kCmdThemeNeon, MF_CHECKED);
    }

    parent.AppendMenu(MF_POPUP, (UINT_PTR)themeMenu.Detach(), L"\u4e3b\u9898 (Theme)");
}

static bool TryBuildEffectJsonFromMetadata(UINT cmd, const mousefx::EffectOption* opts, size_t count, const char* category, std::string* outJson) {
    for (size_t i = 0; i < count; ++i) {
        if (opts[i].trayCmd == cmd) {
            if (!outJson) return true;
            if (std::string(opts[i].value) == "none") {
                *outJson = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
            } else {
                *outJson = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category + "\",\"type\":\"" + opts[i].value + "\"}";
            }
            return true;
        }
    }
    return false;
}

} // namespace

void TrayMenuBuilder::BuildTrayMenu(CMenu& menu, mousefx::AppController* mouseFx) {
    menu.CreatePopupMenu();
    size_t n = 0;

    AppendEffectSubMenu(menu, L"\u70b9\u51fb\u7279\u6548 (Click)", mouseFx, mousefx::EffectCategory::Click, mousefx::ClickMetadata(n), n);
    AppendEffectSubMenu(menu, L"\u62d6\u5c3e\u7279\u6548 (Trail)", mouseFx, mousefx::EffectCategory::Trail, mousefx::TrailMetadata(n), n);
    AppendEffectSubMenu(menu, L"\u6eda\u8f6e\u7279\u6548 (Scroll)", mouseFx, mousefx::EffectCategory::Scroll, mousefx::ScrollMetadata(n), n);
    AppendEffectSubMenu(menu, L"\u957f\u6309\u7279\u6548 (Hold)", mouseFx, mousefx::EffectCategory::Hold, mousefx::HoldMetadata(n), n);
    AppendEffectSubMenu(menu, L"\u60ac\u505c\u7279\u6548 (Hover)", mouseFx, mousefx::EffectCategory::Hover, mousefx::HoverMetadata(n), n);

    AppendThemeSubMenu(menu, mouseFx);

    menu.AppendMenu(MF_STRING, kCmdStarRepo, L"\u9879\u76ee\u5730\u5740 / \u652f\u6301\u4f5c\u8005 (Project/Star)");
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, kCmdTraySettings, L"\u8bbe\u7f6e... (Settings...)");
    menu.AppendMenu(MF_STRING, kCmdTrayExit, L"\u9000\u51fa");
}

bool TrayMenuBuilder::TryBuildIpcJson(UINT cmd, std::string* outJson) {
    size_t n = 0;
    if (TryBuildEffectJsonFromMetadata(cmd, mousefx::ClickMetadata(n), n, "click", outJson)) return true;
    if (TryBuildEffectJsonFromMetadata(cmd, mousefx::TrailMetadata(n), n, "trail", outJson)) return true;
    if (TryBuildEffectJsonFromMetadata(cmd, mousefx::ScrollMetadata(n), n, "scroll", outJson)) return true;
    if (TryBuildEffectJsonFromMetadata(cmd, mousefx::HoldMetadata(n), n, "hold", outJson)) return true;
    if (TryBuildEffectJsonFromMetadata(cmd, mousefx::HoverMetadata(n), n, "hover", outJson)) return true;
    return false;
}

bool TrayMenuBuilder::TryBuildTheme(UINT cmd, std::string* outTheme) {
    if (!outTheme) return false;
    switch (cmd) {
        case kCmdThemeChromatic: *outTheme = "chromatic"; return true;
        case kCmdThemeSciFi:     *outTheme = "scifi"; return true;
        case kCmdThemeNeon:      *outTheme = "neon"; return true;
        case kCmdThemeMinimal:   *outTheme = "minimal"; return true;
        case kCmdThemeGame:      *outTheme = "game"; return true;
        default: return false;
    }
}
