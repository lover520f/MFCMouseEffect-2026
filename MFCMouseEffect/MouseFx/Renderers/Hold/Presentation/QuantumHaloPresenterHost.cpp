#include "pch.h"
#include "QuantumHaloPresenterHost.h"

namespace mousefx {

bool QuantumHaloPresenterHost::Start() {
    if (started_) {
        return IsReady();
    }

    started_ = true;
    lastErrorReason_.clear();
    activeBackendName_.clear();
    backends_ = QuantumHaloPresenterBackendRegistry::Instance().ListByPriority();
    nextBackendIndex_ = 0;
    return EnsureBackendStarted();
}

void QuantumHaloPresenterHost::Shutdown() {
    DropBackend();
    backends_.clear();
    nextBackendIndex_ = 0;
    activeBackendName_.clear();
    started_ = false;
}

bool QuantumHaloPresenterHost::IsReady() const {
    return backend_ && backend_->IsReady();
}

const std::string& QuantumHaloPresenterHost::LastErrorReason() const {
    return lastErrorReason_;
}

const std::string& QuantumHaloPresenterHost::ActiveBackendName() const {
    return activeBackendName_;
}

bool QuantumHaloPresenterHost::RenderFrame(
    int cursorScreenX,
    int cursorScreenY,
    int sizePx,
    float t,
    uint64_t elapsedMs,
    uint32_t holdMs,
    const RippleStyle& style) {
    if (!started_) {
        if (!Start()) {
            return false;
        }
    }

    QuantumHaloPresenterFrameArgs frame{};
    frame.cursorScreenX = cursorScreenX;
    frame.cursorScreenY = cursorScreenY;
    frame.sizePx = sizePx;
    frame.t = t;
    frame.elapsedMs = elapsedMs;
    frame.holdMs = holdMs;
    frame.style = &style;

    while (EnsureBackendStarted()) {
        if (backend_->RenderFrame(frame)) {
            return true;
        }
        lastErrorReason_ = ComposeError(
            "backend_render_failed",
            activeBackendName_,
            backend_->LastErrorReason());
        DropBackend();
    }

    return false;
}

bool QuantumHaloPresenterHost::EnsureBackendStarted() {
    if (backend_ && backend_->IsReady()) {
        return true;
    }

    DropBackend();
    auto& registry = QuantumHaloPresenterBackendRegistry::Instance();
    while (nextBackendIndex_ < backends_.size()) {
        const auto& desc = backends_[nextBackendIndex_++];

        std::unique_ptr<IQuantumHaloPresenterBackend> candidate = registry.Create(desc.name);
        if (!candidate) {
            lastErrorReason_ = ComposeError("backend_create_failed", desc.name, "factory_missing");
            continue;
        }

        if (!candidate->Start() || !candidate->IsReady()) {
            const std::string reason = candidate->LastErrorReason();
            candidate->Shutdown();
            lastErrorReason_ = ComposeError("backend_start_failed", desc.name, reason);
            continue;
        }

        activeBackendName_ = desc.name;
        backend_ = std::move(candidate);
        lastErrorReason_.clear();
        return true;
    }

    if (backends_.empty()) {
        lastErrorReason_ = "no_presenter_backend_registered";
    } else if (lastErrorReason_.empty()) {
        lastErrorReason_ = "no_presenter_backend_available";
    }
    return false;
}

void QuantumHaloPresenterHost::DropBackend() {
    if (backend_) {
        backend_->Shutdown();
    }
    backend_.reset();
    activeBackendName_.clear();
}

std::string QuantumHaloPresenterHost::ComposeError(
    const std::string& prefix,
    const std::string& backendName,
    const std::string& detail) {
    std::string text = prefix;
    if (!backendName.empty()) {
        text += "_" + backendName;
    }
    if (!detail.empty()) {
        text += "_" + detail;
    }
    return text;
}

} // namespace mousefx
