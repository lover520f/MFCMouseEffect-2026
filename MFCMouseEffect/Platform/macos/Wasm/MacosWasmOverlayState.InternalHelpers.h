#pragma once

#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

namespace mousefx::platform::macos::wasm_overlay_state_detail {

WasmOverlayAdmissionResult TryAcquireWasmOverlaySlotLocked(WasmOverlayKind kind);
void ResetWasmOverlayStateLocked();

} // namespace mousefx::platform::macos::wasm_overlay_state_detail
