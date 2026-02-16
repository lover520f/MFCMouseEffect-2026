#include "pch.h"
#include "QuantumHaloPresenterHost.h"

#include "QuantumHaloPresenterSelection.h"

#include <algorithm>

namespace mousefx {
namespace {

void PrioritizePreferredBackend(
    std::vector<QuantumHaloPresenterBackendRegistry::Descriptor>* backends,
    const std::string& preferredName) {
    if (!backends || preferredName.empty() || preferredName == QuantumHaloPresenterSelection::kAuto) {
        return;
    }
    auto& list = *backends;
    auto it = std::find_if(list.begin(), list.end(), [&](const auto& item) {
        return item.name == preferredName;
    });
    if (it == list.end() || it == list.begin()) {
        return;
    }
    const auto picked = *it;
    list.erase(it);
    list.insert(list.begin(), picked);
}

} // namespace

bool QuantumHaloPresenterHost::Start() {
    if (started_) {
        return IsReady();
    }

    started_ = true;
    lastErrorReason_.clear();
    lastBackendFailureReason_.clear();
    backendSwitchCount_ = 0;
    activeBackendName_.clear();
    preferredBackendName_ = QuantumHaloPresenterSelection::GetEffectiveBackendPreference();
    backends_ = QuantumHaloPresenterBackendRegistry::Instance().ListByPriority();
    PrioritizePreferredBackend(&backends_, preferredBackendName_);
    nextBackendIndex_ = 0;
    return EnsureBackendStarted();
}

void QuantumHaloPresenterHost::Shutdown() {
    DropBackend();
    backends_.clear();
    nextBackendIndex_ = 0;
    activeBackendName_.clear();
    preferredBackendName_.clear();
    lastBackendFailureReason_.clear();
    backendSwitchCount_ = 0;
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

const std::string& QuantumHaloPresenterHost::PreferredBackendName() const {
    return preferredBackendName_;
}

const std::string& QuantumHaloPresenterHost::LastBackendFailureReason() const {
    return lastBackendFailureReason_;
}

uint32_t QuantumHaloPresenterHost::BackendSwitchCount() const {
    return backendSwitchCount_;
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
        lastBackendFailureReason_ = ComposeError(
            "backend_render_failed",
            activeBackendName_,
            backend_->LastErrorReason());
        lastErrorReason_ = lastBackendFailureReason_;
        ++backendSwitchCount_;
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
            lastBackendFailureReason_ = ComposeError("backend_create_failed", desc.name, "factory_missing");
            lastErrorReason_ = lastBackendFailureReason_;
            continue;
        }

        if (!candidate->Start() || !candidate->IsReady()) {
            const std::string reason = candidate->LastErrorReason();
            candidate->Shutdown();
            lastBackendFailureReason_ = ComposeError("backend_start_failed", desc.name, reason);
            lastErrorReason_ = lastBackendFailureReason_;
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
