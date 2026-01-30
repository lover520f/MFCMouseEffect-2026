#include "pch.h"

#include "TrayMenuBuilder.h"

#include "TrayMenuCommands.h"

#include "MouseFx/AppController.h"

#include <vector>

namespace {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

struct EffectMenuItem {
    UINT cmd;
    const TCHAR* label;
    const char* type; // nullptr => "none"
    std::vector<std::string> matchTypes; // empty => matches type
};

static bool MatchesType(const std::string& currentType, const EffectMenuItem& item) {
    if (!item.type) return currentType.empty();
    if (!item.matchTypes.empty()) {
        for (const auto& m : item.matchTypes) {
            if (currentType == m) return true;
        }
        return false;
    }
    return currentType == item.type;
}

static std::string GetCurrentEffectType(mousefx::AppController* mouseFx, mousefx::EffectCategory category) {
    if (!mouseFx) return "";
    auto* effect = mouseFx->GetEffect(category);
    if (!effect) return "";
    const char* name = effect->TypeName();
    return name ? std::string(name) : std::string();
}

static void AppendEffectSubMenu(CMenu& parent, const TCHAR* title, mousefx::AppController* mouseFx,
                                mousefx::EffectCategory category, const std::vector<EffectMenuItem>& items) {
    CMenu sub;
    sub.CreatePopupMenu();

    const std::string currentType = GetCurrentEffectType(mouseFx, category);

    for (const auto& it : items) {
        sub.AppendMenu(MF_STRING, it.cmd, it.label);
        if (MatchesType(currentType, it)) {
            sub.CheckMenuItem(it.cmd, MF_CHECKED);
        }
    }

    parent.AppendMenu(MF_POPUP, (UINT_PTR)sub.Detach(), title);
}

static void AppendThemeSubMenu(CMenu& parent, mousefx::AppController* mouseFx) {
    CMenu themeMenu;
    themeMenu.CreatePopupMenu();

    themeMenu.AppendMenu(MF_STRING, kCmdThemeChromatic, L"\u70ab\u5f69 (Chromatic)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeSciFi, L"\u79d1\u5e7b (Sci-Fi)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeNeon, L"\u9713\u8679 (Neon)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeMinimal, L"\u6781\u7b80 (Minimal)");
    themeMenu.AppendMenu(MF_STRING, kCmdThemeGame, L"\u6e38\u620f\u611f (Game)");

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

static bool TryBuildEffectJson(UINT cmd, std::string* outJson) {
    struct Map {
        UINT cmd;
        const char* category;
        const char* type; // nullptr => clear
    };

    static const Map kMap[] = {
        // Click
        { kCmdClickRipple, "click", "ripple" },
        { kCmdClickStar,   "click", "star" },
        { kCmdClickText,   "click", "text" },
        { kCmdClickNone,   "click", nullptr },
        // Trail
        { kCmdTrailLine,     "trail", "line" },
        { kCmdTrailStreamer, "trail", "streamer" },
        { kCmdTrailElectric, "trail", "electric" },
        { kCmdTrailTubes,    "trail", "tubes" },
        { kCmdTrailParticle, "trail", "particle" },
        { kCmdTrailNone,     "trail", nullptr },
        // Scroll
        { kCmdScrollArrow, "scroll", "arrow" },
        { kCmdScrollNone,  "scroll", nullptr },
        // Hold
        { kCmdHoldCharge,   "hold", "charge" },
        { kCmdHoldLightning,"hold", "lightning" },
        { kCmdHoldHex,      "hold", "hex" },
        { kCmdHoldTechRing, "hold", "tech_ring" },
        { kCmdHoldSciFi3D,  "hold", "hologram" },
        { kCmdHoldNone,     "hold", nullptr },
        // Hover
        { kCmdHoverGlow,  "hover", "glow" },
        { kCmdHoverTubes, "hover", "tubes" },
        { kCmdHoverNone,  "hover", nullptr },
    };

    for (const auto& m : kMap) {
        if (m.cmd != cmd) continue;
        if (!outJson) return true;

        if (!m.type) {
            *outJson = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + m.category + "\"}";
        } else {
            *outJson = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + m.category + "\",\"type\":\"" + m.type + "\"}";
        }
        return true;
    }

    return false;
}

} // namespace

void TrayMenuBuilder::BuildTrayMenu(CMenu& menu, mousefx::AppController* mouseFx) {
    menu.CreatePopupMenu();

    AppendEffectSubMenu(menu, L"\u70b9\u51fb\u7279\u6548 (Click)", mouseFx, mousefx::EffectCategory::Click, {
        { kCmdClickRipple, L"\u6c34\u6ce2\u7eb9 (Ripple)", "ripple" },
        { kCmdClickStar,  L"\u661f\u661f (Star)", "star" },
        { kCmdClickText,  L"\u98d8\u6d6e\u6587\u5b57 (Text)", "text" },
        { kCmdClickNone,  L"\u65e0 (None)", nullptr },
    });

    AppendEffectSubMenu(menu, L"\u62d6\u5c3e\u7279\u6548 (Trail)", mouseFx, mousefx::EffectCategory::Trail, {
        { kCmdTrailStreamer, L"\u9713\u8679\u6d41\u5149 (Streamer)", "streamer" },
        { kCmdTrailElectric, L"\u8d5b\u535a\u7535\u5f27 (Electric)", "electric" },
        { kCmdTrailTubes, L"\u79d1\u5e7b\u7ba1\u9053 (Tubes)", "tubes" },
        { kCmdTrailParticle, L"\u5f69\u8679\u7c92\u5b50 (Particle)", "particle" },
        { kCmdTrailLine, L"\u666e\u901a\u7ebf\u6761 (Line)", "line" },
        { kCmdTrailNone, L"\u65e0 (None)", nullptr },
    });

    AppendEffectSubMenu(menu, L"\u6eda\u8f6e\u7279\u6548 (Scroll)", mouseFx, mousefx::EffectCategory::Scroll, {
        { kCmdScrollArrow, L"\u65b9\u5411\u6307\u793a (Arrow)", "arrow" },
        { kCmdScrollNone,  L"\u65e0 (None)", nullptr },
    });

    AppendEffectSubMenu(menu, L"\u957f\u6309\u7279\u6548 (Hold)", mouseFx, mousefx::EffectCategory::Hold, {
        { kCmdHoldCharge, L"\u84c4\u529b (Charge)", "charge" },
        { kCmdHoldLightning, L"\u95ea\u7535 (Lightning)", "lightning" },
        { kCmdHoldHex, L"\u516d\u8fb9\u5f62 (Hex)", "hex" },
        { kCmdHoldTechRing, L"\u79d1\u6280\u5708 (3D)", "tech_ring" },
        { kCmdHoldSciFi3D, L"\u5168\u606f\u6295\u5f71 (3D)", "hologram", { "hologram", "scifi3d" } },
        { kCmdHoldNone, L"\u65e0 (None)", nullptr },
    });

    AppendEffectSubMenu(menu, L"\u60ac\u505c\u7279\u6548 (Hover)", mouseFx, mousefx::EffectCategory::Hover, {
        { kCmdHoverGlow, L"\u547c\u5438\u706f (Glow)", "glow" },
        { kCmdHoverTubes, L"\u87ba\u65cb\u60ac\u6d6e (Helix)", "tubes", { "tubes", "suspension" } },
        { kCmdHoverNone, L"\u65e0 (None)", nullptr },
    });

    AppendThemeSubMenu(menu, mouseFx);

    menu.AppendMenu(MF_STRING, kCmdStarRepo, L"\u9879\u76ee\u5730\u5740 / \u652f\u6301\u4f5c\u8005 (Project/Star)");
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, kCmdTraySettings, L"\u8bbe\u7f6e... (Settings...)");
    menu.AppendMenu(MF_STRING, kCmdTrayExit, L"\u9000\u51fa");
}

bool TrayMenuBuilder::TryBuildIpcJson(UINT cmd, std::string* outJson) {
    return TryBuildEffectJson(cmd, outJson);
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
