#include "pch.h"

#include "Platform/macos/Control/MacosDispatchMessageCodec.h"

#include "MouseFx/Core/Protocol/MouseFxMessages.h"

#include <cstdint>

namespace mousefx {

namespace {

uint64_t ToUnsignedPayload(intptr_t payload) {
    return static_cast<uint64_t>(static_cast<uintptr_t>(payload));
}

} // namespace

DispatchMessage MacosDispatchMessageCodec::Decode(
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
        out.x = UnpackPointX(lParam);
        out.y = UnpackPointY(lParam);
        break;
    case WM_MFX_KEY:
        out.kind = DispatchMessageKind::Key;
        out.keyEvent = reinterpret_cast<KeyEvent*>(lParam);
        break;
    case WM_MFX_BUTTON_DOWN:
        out.kind = DispatchMessageKind::ButtonDown;
        out.button = static_cast<uint32_t>(wParam);
        out.x = UnpackPointX(lParam);
        out.y = UnpackPointY(lParam);
        break;
    case WM_MFX_BUTTON_UP:
        out.kind = DispatchMessageKind::ButtonUp;
        out.button = static_cast<uint32_t>(wParam);
        break;
    case kTimerMessageId:
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

intptr_t MacosDispatchMessageCodec::DefaultResult(
    uintptr_t /*sourceHandle*/,
    uint32_t /*msg*/,
    uintptr_t /*wParam*/,
    intptr_t /*lParam*/) const {
    return 0;
}

intptr_t MacosDispatchMessageCodec::PackPointPayload(int32_t x, int32_t y) {
    const uint64_t ux = static_cast<uint64_t>(static_cast<uint32_t>(x));
    const uint64_t uy = static_cast<uint64_t>(static_cast<uint32_t>(y));
    return static_cast<intptr_t>((ux << 32u) | uy);
}

int32_t MacosDispatchMessageCodec::UnpackPointX(intptr_t payload) {
    const uint64_t raw = ToUnsignedPayload(payload);
    return static_cast<int32_t>(static_cast<uint32_t>(raw >> 32u));
}

int32_t MacosDispatchMessageCodec::UnpackPointY(intptr_t payload) {
    const uint64_t raw = ToUnsignedPayload(payload);
    return static_cast<int32_t>(static_cast<uint32_t>(raw & 0xFFFFFFFFu));
}

} // namespace mousefx
