#include "Platform/macos/Shell/MacosTrayMenuLocalization.h"

#if defined(__APPLE__)
#include "Platform/macos/Shell/MacosTrayMenuLocalizationSwiftBridge.h"
#endif

namespace mousefx {

MacosTrayMenuText ResolveMacosTrayMenuText() {
#if defined(__APPLE__)
    if (mfx_macos_tray_prefers_zh_language_v1() > 0) {
        MacosTrayMenuText text;
        text.themeTitle = u8"\u4e3b\u9898";
        text.settingsTitle = u8"\u8bbe\u7f6e";
        text.exitTitle = u8"\u9000\u51fa";
        text.preferZhLabels = true;
        return text;
    }
#endif
    return {};
}

} // namespace mousefx
