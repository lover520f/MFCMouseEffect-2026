#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjector.Internal.h"

#if defined(__APPLE__)
#import <ApplicationServices/ApplicationServices.h>
#endif

#include <cstdlib>

namespace mousefx::macos_keyboard_injector::inject_detail {

bool IsKeyboardInjectorDryRunEnabled() {
    const char* raw = std::getenv("MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    return raw[0] == '1' || raw[0] == 'y' || raw[0] == 'Y' || raw[0] == 't' || raw[0] == 'T';
}

bool PostKeyEvent(uint16_t keyCode, bool keyDown, uint64_t flags) {
#if !defined(__APPLE__)
    (void)keyCode;
    (void)keyDown;
    (void)flags;
    return false;
#else
    CGEventRef event = CGEventCreateKeyboardEvent(nullptr, static_cast<CGKeyCode>(keyCode), keyDown ? true : false);
    if (event == nullptr) {
        return false;
    }

    CGEventSetFlags(event, static_cast<CGEventFlags>(flags));
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return true;
#endif
}

} // namespace mousefx::macos_keyboard_injector::inject_detail
