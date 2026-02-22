#include "pch.h"
#include "QuantumHaloDCompPresenterBackend.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.h"

namespace mousefx {

bool QuantumHaloDCompPresenterBackend::Start() {
    lastErrorReason_.clear();
    if (presenter_.Start()) {
        return true;
    }
    lastErrorReason_ = presenter_.LastErrorReason();
    if (lastErrorReason_.empty()) {
        lastErrorReason_ = "start_failed";
    }
    return false;
}

void QuantumHaloDCompPresenterBackend::Shutdown() {
    presenter_.Shutdown();
}

bool QuantumHaloDCompPresenterBackend::IsReady() const {
    return presenter_.IsReady();
}

const std::string& QuantumHaloDCompPresenterBackend::LastErrorReason() const {
    if (!lastErrorReason_.empty()) {
        return lastErrorReason_;
    }
    return presenter_.LastErrorReason();
}

bool QuantumHaloDCompPresenterBackend::RenderFrame(const QuantumHaloPresenterFrameArgs& frame) {
    lastErrorReason_.clear();
    if (!frame.style) {
        lastErrorReason_ = "frame_style_null";
        return false;
    }
    const bool ok = presenter_.RenderFrame(
        frame.cursorScreenX,
        frame.cursorScreenY,
        frame.sizePx,
        frame.t,
        frame.elapsedMs,
        frame.holdMs,
        *frame.style);
    if (!ok) {
        lastErrorReason_ = presenter_.LastErrorReason();
        if (lastErrorReason_.empty()) {
            lastErrorReason_ = "render_frame_failed";
        }
    }
    return ok;
}

REGISTER_QUANTUM_HALO_PRESENTER_BACKEND("dcomp_d3d11", 100, QuantumHaloDCompPresenterBackend)

} // namespace mousefx
