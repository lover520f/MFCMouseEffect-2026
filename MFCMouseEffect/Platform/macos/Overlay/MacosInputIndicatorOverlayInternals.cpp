#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#if defined(__APPLE__)
#include <pthread.h>
#endif
#include <sstream>

namespace mousefx::macos_input_indicator {

int ClampInt(int value, int lo, int hi) {
    return std::max(lo, std::min(value, hi));
}

std::string MouseButtonLabel(MouseButton button) {
    switch (button) {
    case MouseButton::Left:
        return "L";
    case MouseButton::Right:
        return "R";
    case MouseButton::Middle:
        return "M";
    default:
        return "?";
    }
}

std::string ScrollLabel(int delta) {
    return delta >= 0 ? "SCR +" : "SCR -";
}

std::string KeyLabel(const KeyEvent& ev) {
    if (!ev.text.empty()) {
        return Utf16ToUtf8(ev.text.c_str());
    }
    std::ostringstream oss;
    if (ev.ctrl) {
        oss << "Ctrl+";
    }
    if (ev.shift) {
        oss << "Shift+";
    }
    if (ev.alt) {
        oss << "Alt+";
    }
    if (ev.meta || ev.win) {
        oss << "Cmd+";
    }
    oss << "K" << ev.vkCode;
    return oss.str();
}

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    if (pthread_main_np() != 0) {
        block();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), block);
}

void RunOnMainThreadAsync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), block);
}

void FlushMainThreadQueueSync() {
    if (pthread_main_np() != 0) {
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), ^{});
}
#endif

} // namespace mousefx::macos_input_indicator
