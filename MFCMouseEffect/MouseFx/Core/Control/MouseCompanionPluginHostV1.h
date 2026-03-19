#pragma once

#include <memory>
#include <string>
#include <utility>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"

namespace mousefx {

class IMouseCompanionPluginV1 {
public:
    virtual ~IMouseCompanionPluginV1() = default;

    virtual void Initialize(const MouseCompanionPetRuntimeConfig& config) = 0;
    virtual void OnConfigChanged(const MouseCompanionPetRuntimeConfig& config) = 0;
    virtual void OnInput(const MouseCompanionPetInputEvent& event) = 0;
    virtual void Tick(uint64_t nowTickMs, float dtSeconds) = 0;
    virtual void SamplePose(MouseCompanionPetPoseFrame* outFrame) const = 0;
    virtual void Shutdown() = 0;
};

namespace detail {

inline const char* MouseCompanionPluginV1InputKindName(MouseCompanionPetInputKind kind) {
    switch (kind) {
    case MouseCompanionPetInputKind::Move: return "move";
    case MouseCompanionPetInputKind::Scroll: return "scroll";
    case MouseCompanionPetInputKind::Click: return "click";
    case MouseCompanionPetInputKind::ButtonDown: return "button_down";
    case MouseCompanionPetInputKind::ButtonUp: return "button_up";
    case MouseCompanionPetInputKind::HoverStart: return "hover_start";
    case MouseCompanionPetInputKind::HoverEnd: return "hover_end";
    case MouseCompanionPetInputKind::HoldStart: return "hold_start";
    case MouseCompanionPetInputKind::HoldUpdate: return "hold_update";
    case MouseCompanionPetInputKind::HoldEnd: return "hold_end";
    default: return "unknown";
    }
}

inline const char* MouseCompanionPluginV1ActionName(MouseCompanionPetActionHint hint) {
    switch (hint) {
    case MouseCompanionPetActionHint::Idle: return "idle";
    case MouseCompanionPetActionHint::Follow: return "follow";
    case MouseCompanionPetActionHint::Click: return "click_react";
    case MouseCompanionPetActionHint::Drag: return "drag";
    case MouseCompanionPetActionHint::Hold: return "hold_react";
    case MouseCompanionPetActionHint::Scroll: return "scroll_react";
    case MouseCompanionPetActionHint::None:
    default: return "idle";
    }
}

class MouseCompanionBuiltinPluginV1 final : public IMouseCompanionPluginV1 {
public:
    void Initialize(const MouseCompanionPetRuntimeConfig& config) override {
        config_ = config;
        poseFrame_.Clear();
    }

    void OnConfigChanged(const MouseCompanionPetRuntimeConfig& config) override {
        config_ = config;
    }

    void OnInput(const MouseCompanionPetInputEvent& event) override {
        poseFrame_.sampleTickMs = event.tickMs;
        poseFrame_.actionName = MouseCompanionPluginV1ActionName(event.actionHint);
        poseFrame_.actionIntensity = event.actionIntensity;
    }

    void Tick(uint64_t nowTickMs, float /*dtSeconds*/) override {
        poseFrame_.sampleTickMs = nowTickMs;
    }

    void SamplePose(MouseCompanionPetPoseFrame* outFrame) const override {
        if (!outFrame) {
            return;
        }
        *outFrame = poseFrame_;
    }

    void Shutdown() override {
        poseFrame_.Clear();
    }

    void CommitResolvedFrame(const MouseCompanionPetPoseFrame& frame) {
        poseFrame_ = frame;
    }

private:
    MouseCompanionPetRuntimeConfig config_{};
    MouseCompanionPetPoseFrame poseFrame_{};
};

} // namespace detail

class MouseCompanionPluginHostV1 final {
public:
    MouseCompanionPluginHostV1() = default;

    void Reset(const MouseCompanionPetRuntimeConfig& config, uint64_t nowTickMs) {
        config_ = config;
        diagnostics_ = MouseCompanionPluginV1Diagnostics{};
        diagnostics_.hostReady = true;
        diagnostics_.hostPhase = "phase1_v1_builtin_parallel";
        diagnostics_.activePluginId = "mousefx.pet.native.v1.builtin";
        diagnostics_.activePluginVersion = "0.1.0";
        diagnostics_.engineApiVersion = "pet-plugin-v1";
        diagnostics_.compatibilityStatus = config.enabled ? "parallel_builtin_ready" : "disabled_by_config";
        diagnostics_.fallbackReason = config.enabled ? "builtin_native_plugin" : "config_disabled";
        diagnostics_.lastActionName = "idle";
        diagnostics_.lastTickMs = nowTickMs;
        plugin_ = std::make_unique<detail::MouseCompanionBuiltinPluginV1>();
        builtinPlugin_ = static_cast<detail::MouseCompanionBuiltinPluginV1*>(plugin_.get());
        plugin_->Initialize(config_);
    }

    void OnConfigChanged(const MouseCompanionPetRuntimeConfig& config, uint64_t nowTickMs) {
        config_ = config;
        if (!plugin_) {
            Reset(config, nowTickMs);
            return;
        }
        diagnostics_.compatibilityStatus = config.enabled ? "parallel_builtin_ready" : "disabled_by_config";
        diagnostics_.fallbackReason = config.enabled ? "builtin_native_plugin" : "config_disabled";
        diagnostics_.lastTickMs = nowTickMs;
        plugin_->OnConfigChanged(config_);
    }

    void OnInput(const MouseCompanionPetInputEvent& event) {
        if (!plugin_) {
            return;
        }
        plugin_->OnInput(event);
        diagnostics_.lastEventName = detail::MouseCompanionPluginV1InputKindName(event.kind);
        diagnostics_.lastActionName = detail::MouseCompanionPluginV1ActionName(event.actionHint);
        diagnostics_.lastEventTickMs = event.tickMs;
        ++diagnostics_.eventCount;
    }

    void Tick(uint64_t nowTickMs, float dtSeconds) {
        if (!plugin_) {
            return;
        }
        plugin_->Tick(nowTickMs, dtSeconds);
        diagnostics_.lastTickMs = nowTickMs;
        ++diagnostics_.tickCount;
    }

    void SamplePose(MouseCompanionPetPoseFrame* outFrame) {
        if (!outFrame) {
            return;
        }
        outFrame->Clear();
        if (!plugin_) {
            return;
        }
        plugin_->SamplePose(outFrame);
        diagnostics_.lastPoseSampleTickMs = outFrame->sampleTickMs;
        ++diagnostics_.poseSampleCount;
    }

    MouseCompanionPluginV1Diagnostics SnapshotDiagnostics() const {
        return diagnostics_;
    }

    void CommitResolvedFrame(const MouseCompanionPetPoseFrame& frame) {
        if (!builtinPlugin_) {
            return;
        }
        builtinPlugin_->CommitResolvedFrame(frame);
        diagnostics_.lastActionName = frame.actionName;
        diagnostics_.lastPoseSampleTickMs = frame.sampleTickMs;
    }

    void Shutdown() {
        if (plugin_) {
            plugin_->Shutdown();
        }
        plugin_.reset();
        builtinPlugin_ = nullptr;
    }

private:
    MouseCompanionPetRuntimeConfig config_{};
    MouseCompanionPluginV1Diagnostics diagnostics_{};
    std::unique_ptr<IMouseCompanionPluginV1> plugin_{};
    detail::MouseCompanionBuiltinPluginV1* builtinPlugin_{nullptr};
};

} // namespace mousefx
