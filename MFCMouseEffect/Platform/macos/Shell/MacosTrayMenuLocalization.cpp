#include "Platform/macos/Shell/MacosTrayMenuLocalization.h"

#if defined(__APPLE__)
#include "Platform/macos/Shell/MacosTrayMenuLocalizationSwiftBridge.h"
#endif

namespace mousefx {

MacosTrayMenuText BuildMacosTrayMenuText(bool preferZhLabels) {
    if (preferZhLabels) {
        MacosTrayMenuText text;
        text.themeTitle = u8"\u4e3b\u9898";
        text.clickTitle = u8"\u70b9\u51fb\u7279\u6548";
        text.trailTitle = u8"\u62d6\u5c3e\u7279\u6548";
        text.scrollTitle = u8"\u6eda\u8f6e\u7279\u6548";
        text.holdTitle = u8"\u957f\u6309\u7279\u6548";
        text.hoverTitle = u8"\u60ac\u505c\u7279\u6548";
        text.starProjectTitle = u8"\u2605 Star \u9879\u76ee";
        text.settingsTitle = u8"\u8bbe\u7f6e";
        text.reloadConfigTitle = u8"\u91cd\u8f7d\u914d\u7f6e";
        text.exitTitle = u8"\u9000\u51fa";
        text.preferZhLabels = true;
        return text;
    }
    return {};
}

MacosTrayMenuText ResolveMacosTrayMenuText() {
#if defined(__APPLE__)
    if (mfx_macos_tray_prefers_zh_language_v1() > 0) {
        return BuildMacosTrayMenuText(true);
    }
#endif
    return BuildMacosTrayMenuText(false);
}

} // namespace mousefx
