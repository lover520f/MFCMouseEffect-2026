#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Wasm/WasmEffectHost.h"

namespace mousefx {

void AppController::InitializeWasmHost() {
    if (!wasmEffectHost_) {
        wasmEffectHost_ = std::make_unique<wasm::WasmEffectHost>();
    }

    // Phase 1 safety: host is wired but disabled by default.
    wasmEffectHost_->SetEnabled(false);
}

void AppController::ShutdownWasmHost() {
    if (!wasmEffectHost_) {
        return;
    }
    wasmEffectHost_->SetEnabled(false);
    wasmEffectHost_->UnloadPlugin();
}

} // namespace mousefx

