#include "pch.h"
#include "InputAutomationEngine.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx {
namespace {

std::string NormalizeTextId(std::string value) {
    value = ToLowerAscii(TrimAscii(value));
    std::replace(value.begin(), value.end(), '-', '_');
    std::replace(value.begin(), value.end(), ' ', '_');
    return value;
}

GestureRecognitionConfig BuildGestureConfig(const InputAutomationConfig& config) {
    GestureRecognitionConfig out;
    out.enabled = config.enabled && config.gesture.enabled;
    out.triggerButton = config.gesture.triggerButton;
    out.minStrokeDistancePx = config.gesture.minStrokeDistancePx;
    out.sampleStepPx = config.gesture.sampleStepPx;
    out.maxDirections = config.gesture.maxDirections;
    return out;
}

std::string NormalizeShortcutText(std::string value) {
    return TrimAscii(value);
}

} // namespace

void InputAutomationEngine::UpdateConfig(const InputAutomationConfig& config) {
    config_ = config;
    gestureRecognizer_.UpdateConfig(BuildGestureConfig(config_));
    suppressNextClickActionId_.clear();
}

void InputAutomationEngine::Reset() {
    gestureRecognizer_.Reset();
    suppressNextClickActionId_.clear();
}

void InputAutomationEngine::OnMouseMove(const POINT& pt) {
    gestureRecognizer_.OnMouseMove(pt);
}

void InputAutomationEngine::OnButtonDown(const POINT& pt, int button) {
    gestureRecognizer_.OnButtonDown(pt, button);
}

void InputAutomationEngine::OnButtonUp(const POINT& pt, int button) {
    const std::string gestureId = gestureRecognizer_.OnButtonUp(pt, button);
    if (!gestureId.empty()) {
        if (TriggerGesture(gestureId)) {
            suppressNextClickActionId_ = NormalizeMouseActionId(ClickActionIdFromButtonCode(button));
        }
    }
}

void InputAutomationEngine::OnClick(const ClickEvent& ev) {
    const std::string actionId = ClickActionId(ev.button);
    if (!suppressNextClickActionId_.empty()) {
        const bool shouldSuppress = (NormalizeMouseActionId(actionId) == suppressNextClickActionId_);
        suppressNextClickActionId_.clear();
        if (shouldSuppress) {
            return;
        }
    }
    TriggerMouseAction(actionId);
}

void InputAutomationEngine::OnScroll(short delta) {
    TriggerMouseAction(ScrollActionId(delta));
}

std::string InputAutomationEngine::NormalizeId(std::string value) {
    return NormalizeTextId(std::move(value));
}

std::string InputAutomationEngine::NormalizeMouseActionId(std::string value) {
    value = NormalizeId(std::move(value));
    if (value == "left" || value == "leftclick" || value == "lclick") return "left_click";
    if (value == "right" || value == "rightclick" || value == "rclick") return "right_click";
    if (value == "middle" || value == "middleclick" || value == "mclick") return "middle_click";
    if (value == "wheel_up" || value == "scrollup") return "scroll_up";
    if (value == "wheel_down" || value == "scrolldown") return "scroll_down";
    return value;
}

std::string InputAutomationEngine::NormalizeGestureId(std::string value) {
    return NormalizeId(std::move(value));
}

std::string InputAutomationEngine::ClickActionId(MouseButton button) {
    switch (button) {
    case MouseButton::Left: return "left_click";
    case MouseButton::Right: return "right_click";
    case MouseButton::Middle: return "middle_click";
    default: break;
    }
    return {};
}

std::string InputAutomationEngine::ClickActionIdFromButtonCode(int button) {
    if (button == 1) return "left_click";
    if (button == 2) return "right_click";
    if (button == 3) return "middle_click";
    return {};
}

std::string InputAutomationEngine::ScrollActionId(short delta) {
    if (delta > 0) return "scroll_up";
    if (delta < 0) return "scroll_down";
    return {};
}

bool InputAutomationEngine::TriggerMouseAction(const std::string& actionId) {
    if (!config_.enabled || actionId.empty()) {
        return false;
    }
    const AutomationKeyBinding* binding =
        FindEnabledBinding(config_.mouseMappings, actionId, false);
    if (!binding) {
        return false;
    }
    return keyboardInjector_.SendChord(NormalizeShortcutText(binding->keys));
}

bool InputAutomationEngine::TriggerGesture(const std::string& gestureId) {
    if (!config_.enabled || !config_.gesture.enabled || gestureId.empty()) {
        return false;
    }
    const AutomationKeyBinding* binding =
        FindEnabledBinding(config_.gesture.mappings, gestureId, true);
    if (!binding) {
        return false;
    }
    return keyboardInjector_.SendChord(NormalizeShortcutText(binding->keys));
}

const AutomationKeyBinding* InputAutomationEngine::FindEnabledBinding(
    const std::vector<AutomationKeyBinding>& mappings,
    const std::string& triggerId,
    bool gestureBinding) const {
    const std::string normalizedTrigger =
        gestureBinding ? NormalizeGestureId(triggerId) : NormalizeMouseActionId(triggerId);

    for (const AutomationKeyBinding& binding : mappings) {
        if (!binding.enabled) {
            continue;
        }
        const std::string candidate =
            gestureBinding ? NormalizeGestureId(binding.trigger) : NormalizeMouseActionId(binding.trigger);
        if (candidate != normalizedTrigger) {
            continue;
        }
        if (NormalizeShortcutText(binding.keys).empty()) {
            continue;
        }
        return &binding;
    }
    return nullptr;
}

} // namespace mousefx
