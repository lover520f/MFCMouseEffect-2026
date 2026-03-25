#include "pch.h"

#include "Platform/windows/Control/Win32DispatchMessageCodec.h"

#include <windows.h>
#include <windowsx.h>

#include "MouseFx/Core/Protocol/MouseFxMessages.h"

namespace mousefx {

DispatchMessage Win32DispatchMessageCodec::Decode(
    uintptr_t sourceHandle,
    uint32_t msg,
    uintptr_t wParam,
    intptr_t lParam) const {
    DispatchMessage out{};
    out.sourceHandle = sourceHandle;
    out.nativeMsg = msg;
    out.nativeWParam = wParam;
    out.nativeLParam = lParam;

    switch (msg) {
    case WM_MFX_CLICK:
        out.kind = DispatchMessageKind::Click;
        out.clickEvent = reinterpret_cast<ClickEvent*>(lParam);
        break;
    case WM_MFX_MOVE:
        out.kind = DispatchMessageKind::Move;
        out.x = static_cast<int32_t>(wParam);
        out.y = static_cast<int32_t>(lParam);
        break;
    case WM_MFX_SCROLL:
        out.kind = DispatchMessageKind::Scroll;
        out.delta = UnpackScrollDispatchDelta(wParam);
        out.scrollHorizontal = (UnpackScrollDispatchFlags(wParam) & kScrollDispatchFlagHorizontal) != 0;
        out.x = GET_X_LPARAM(static_cast<LPARAM>(lParam));
        out.y = GET_Y_LPARAM(static_cast<LPARAM>(lParam));
        break;
    case WM_MFX_KEY:
        out.kind = DispatchMessageKind::Key;
        out.keyEvent = reinterpret_cast<KeyEvent*>(lParam);
        break;
    case WM_MFX_BUTTON_DOWN:
        out.kind = DispatchMessageKind::ButtonDown;
        out.button = static_cast<uint32_t>(wParam);
        out.x = GET_X_LPARAM(static_cast<LPARAM>(lParam));
        out.y = GET_Y_LPARAM(static_cast<LPARAM>(lParam));
        break;
    case WM_MFX_BUTTON_UP:
        out.kind = DispatchMessageKind::ButtonUp;
        out.button = static_cast<uint32_t>(wParam);
        break;
    case WM_TIMER:
        out.kind = DispatchMessageKind::Timer;
        out.timerId = static_cast<uint32_t>(wParam);
        break;
    case WM_MFX_EXEC_CMD:
        out.kind = DispatchMessageKind::ExecCmd;
        out.commandJson = reinterpret_cast<const std::string*>(lParam);
        break;
    case WM_MFX_GET_CONFIG:
        out.kind = DispatchMessageKind::GetConfig;
        out.configOut = reinterpret_cast<EffectConfig*>(lParam);
        break;
    default:
        out.kind = DispatchMessageKind::Unknown;
        break;
    }

    return out;
}

intptr_t Win32DispatchMessageCodec::DefaultResult(
    uintptr_t sourceHandle,
    uint32_t msg,
    uintptr_t wParam,
    intptr_t lParam) const {
    return static_cast<intptr_t>(DefWindowProcW(
        reinterpret_cast<HWND>(sourceHandle),
        static_cast<UINT>(msg),
        static_cast<WPARAM>(wParam),
        static_cast<LPARAM>(lParam)));
}

} // namespace mousefx
