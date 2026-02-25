#include "pch.h"

#include "Platform/macos/System/MacosInputEventUtils.h"

namespace mousefx::macos_input_event {

MouseButton MouseButtonFromEvent(CGEventRef event) {
    const int64_t buttonValue = CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
    if (buttonValue == 1) {
        return MouseButton::Right;
    }
    if (buttonValue == 2) {
        return MouseButton::Middle;
    }
    return MouseButton::Left;
}

bool IsMouseMoveType(CGEventType type) {
    return type == kCGEventMouseMoved ||
           type == kCGEventLeftMouseDragged ||
           type == kCGEventRightMouseDragged ||
           type == kCGEventOtherMouseDragged;
}

bool IsMouseDownType(CGEventType type) {
    return type == kCGEventLeftMouseDown ||
           type == kCGEventRightMouseDown ||
           type == kCGEventOtherMouseDown;
}

bool IsMouseUpType(CGEventType type) {
    return type == kCGEventLeftMouseUp ||
           type == kCGEventRightMouseUp ||
           type == kCGEventOtherMouseUp;
}

ScreenPoint ToScreenPoint(const CGPoint& pt) {
    ScreenPoint out{};
    out.x = static_cast<int32_t>(pt.x);
    out.y = static_cast<int32_t>(pt.y);
    return out;
}

} // namespace mousefx::macos_input_event
