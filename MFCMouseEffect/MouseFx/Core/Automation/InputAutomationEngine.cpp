#include "pch.h"
#include "InputAutomationEngine.h"

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/InputAutomationDispatch.h"
#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace mousefx {
namespace {

constexpr std::chrono::milliseconds kMouseChainMaxStepIntervalMs(900);
constexpr std::chrono::milliseconds kMouseChainMaxTotalIntervalMs(1800);
constexpr std::chrono::milliseconds kGestureChainMaxStepIntervalMs(2200);
constexpr std::chrono::milliseconds kGestureChainMaxTotalIntervalMs(5000);

GestureRecognitionConfig BuildGestureConfig(const InputAutomationConfig& config) {
    GestureRecognitionConfig out;
    out.enabled = config.enabled && config.gesture.enabled;
    out.minStrokeDistancePx = config.gesture.minStrokeDistancePx;
    out.sampleStepPx = config.gesture.sampleStepPx;
    out.maxDirections = config.gesture.maxDirections;
    return out;
}

std::string ButtonNameFromCode(int button) {
    if (button == 1) return "left";
    if (button == 2) return "right";
    if (button == 3) return "middle";
    return {};
}

struct NormalizedPoint final {
    double x = 0.0;
    double y = 0.0;
};

bool IsCustomGestureMode(const AutomationKeyBinding& binding) {
    return ToLowerAscii(TrimAscii(binding.gesturePattern.mode)) == "custom";
}

std::vector<std::vector<AutomationKeyBinding::GesturePoint>> TemplateStrokesForBinding(
    const AutomationKeyBinding& binding) {
    if (!binding.gesturePattern.customStrokes.empty()) {
        return binding.gesturePattern.customStrokes;
    }
    if (!binding.gesturePattern.customPoints.empty()) {
        return {binding.gesturePattern.customPoints};
    }
    return {};
}

std::vector<std::vector<NormalizedPoint>> NormalizeTemplateStrokes(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& strokes) {
    std::vector<std::vector<NormalizedPoint>> out;
    if (strokes.empty()) {
        return out;
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    bool hasPoint = false;
    for (const auto& stroke : strokes) {
        for (const auto& point : stroke) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
            hasPoint = true;
        }
    }
    if (!hasPoint) {
        return out;
    }

    const double width = std::max(1.0, static_cast<double>(maxX - minX));
    const double height = std::max(1.0, static_cast<double>(maxY - minY));
    for (const auto& stroke : strokes) {
        std::vector<NormalizedPoint> normalizedStroke;
        normalizedStroke.reserve(stroke.size());
        for (const auto& point : stroke) {
            NormalizedPoint normalized;
            normalized.x = ((static_cast<double>(point.x) - static_cast<double>(minX)) / width) * 100.0;
            normalized.y = ((static_cast<double>(point.y) - static_cast<double>(minY)) / height) * 100.0;
            if (!normalizedStroke.empty()) {
                const NormalizedPoint& prev = normalizedStroke.back();
                if (std::abs(prev.x - normalized.x) < 0.05 &&
                    std::abs(prev.y - normalized.y) < 0.05) {
                    continue;
                }
            }
            normalizedStroke.push_back(normalized);
        }
        if (!normalizedStroke.empty()) {
            out.push_back(std::move(normalizedStroke));
        }
    }
    return out;
}

std::vector<std::vector<NormalizedPoint>> NormalizeCapturedStrokes(
    const std::vector<std::vector<ScreenPoint>>& strokes) {
    std::vector<std::vector<NormalizedPoint>> out;
    if (strokes.empty()) {
        return out;
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    bool hasPoint = false;
    for (const auto& stroke : strokes) {
        for (const auto& point : stroke) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
            hasPoint = true;
        }
    }
    if (!hasPoint) {
        return out;
    }

    const double width = std::max(1.0, static_cast<double>(maxX - minX));
    const double height = std::max(1.0, static_cast<double>(maxY - minY));
    for (const auto& stroke : strokes) {
        std::vector<NormalizedPoint> normalizedStroke;
        normalizedStroke.reserve(stroke.size());
        for (const auto& point : stroke) {
            NormalizedPoint normalized;
            normalized.x = ((static_cast<double>(point.x) - static_cast<double>(minX)) / width) * 100.0;
            normalized.y = ((static_cast<double>(point.y) - static_cast<double>(minY)) / height) * 100.0;
            if (!normalizedStroke.empty()) {
                const NormalizedPoint& prev = normalizedStroke.back();
                if (std::abs(prev.x - normalized.x) < 0.05 &&
                    std::abs(prev.y - normalized.y) < 0.05) {
                    continue;
                }
            }
            normalizedStroke.push_back(normalized);
        }
        if (!normalizedStroke.empty()) {
            out.push_back(std::move(normalizedStroke));
        }
    }
    return out;
}

double PointDistance(const NormalizedPoint& lhs, const NormalizedPoint& rhs) {
    const double dx = lhs.x - rhs.x;
    const double dy = lhs.y - rhs.y;
    return std::sqrt(dx * dx + dy * dy);
}

double StrokeLength(const std::vector<NormalizedPoint>& stroke) {
    if (stroke.size() < 2) {
        return 0.0;
    }
    double length = 0.0;
    for (size_t i = 1; i < stroke.size(); ++i) {
        length += PointDistance(stroke[i - 1], stroke[i]);
    }
    return length;
}

std::vector<NormalizedPoint> ResampleStroke(const std::vector<NormalizedPoint>& stroke, size_t sampleCount) {
    if (stroke.empty() || sampleCount == 0) {
        return {};
    }
    if (stroke.size() == 1 || sampleCount == 1) {
        return std::vector<NormalizedPoint>(sampleCount, stroke.front());
    }

    std::vector<double> cumulativeLengths;
    cumulativeLengths.reserve(stroke.size());
    cumulativeLengths.push_back(0.0);
    for (size_t i = 1; i < stroke.size(); ++i) {
        cumulativeLengths.push_back(cumulativeLengths.back() + PointDistance(stroke[i - 1], stroke[i]));
    }

    const double totalLength = cumulativeLengths.back();
    if (!(totalLength > 1e-6)) {
        return std::vector<NormalizedPoint>(sampleCount, stroke.front());
    }

    std::vector<NormalizedPoint> resampled;
    resampled.reserve(sampleCount);
    size_t segmentIndex = 1;
    for (size_t i = 0; i < sampleCount; ++i) {
        const double ratio = (sampleCount <= 1)
            ? 0.0
            : static_cast<double>(i) / static_cast<double>(sampleCount - 1);
        const double targetLength = ratio * totalLength;
        while (segmentIndex < cumulativeLengths.size() &&
               cumulativeLengths[segmentIndex] < targetLength) {
            ++segmentIndex;
        }
        if (segmentIndex >= cumulativeLengths.size()) {
            resampled.push_back(stroke.back());
            continue;
        }
        const size_t from = (segmentIndex == 0) ? 0 : segmentIndex - 1;
        const double segmentStart = cumulativeLengths[from];
        const double segmentEnd = cumulativeLengths[segmentIndex];
        const double segmentLength = std::max(1e-6, segmentEnd - segmentStart);
        const double localRatio = (targetLength - segmentStart) / segmentLength;
        const NormalizedPoint point{
            stroke[from].x + (stroke[segmentIndex].x - stroke[from].x) * localRatio,
            stroke[from].y + (stroke[segmentIndex].y - stroke[from].y) * localRatio
        };
        resampled.push_back(point);
    }
    return resampled;
}

double StrokeSimilarityScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return -1.0;
    }
    const size_t sampleCount = std::clamp<size_t>(
        std::max(lhs.size(), rhs.size()) * 2,
        8,
        96);
    const std::vector<NormalizedPoint> sampledLhs = ResampleStroke(lhs, sampleCount);
    const std::vector<NormalizedPoint> sampledRhs = ResampleStroke(rhs, sampleCount);
    if (sampledLhs.size() != sampledRhs.size() || sampledLhs.empty()) {
        return -1.0;
    }

    double totalDistance = 0.0;
    for (size_t i = 0; i < sampledLhs.size(); ++i) {
        totalDistance += PointDistance(sampledLhs[i], sampledRhs[i]);
    }
    const double meanDistance = totalDistance / static_cast<double>(sampledLhs.size());
    const double maxDistance = std::sqrt(100.0 * 100.0 + 100.0 * 100.0);
    const double ratio = std::clamp(1.0 - (meanDistance / maxDistance), 0.0, 1.0);
    return ratio * 100.0;
}

double GestureSimilarityScore(
    const std::vector<std::vector<NormalizedPoint>>& templateStrokes,
    const std::vector<std::vector<NormalizedPoint>>& capturedStrokes) {
    if (templateStrokes.empty() ||
        capturedStrokes.empty() ||
        templateStrokes.size() != capturedStrokes.size()) {
        return -1.0;
    }

    double weightedScore = 0.0;
    double totalWeight = 0.0;
    for (size_t i = 0; i < templateStrokes.size(); ++i) {
        const double strokeScore = StrokeSimilarityScore(templateStrokes[i], capturedStrokes[i]);
        if (strokeScore < 0.0) {
            return -1.0;
        }
        const double weight = std::max(1.0, std::max(
            StrokeLength(templateStrokes[i]),
            StrokeLength(capturedStrokes[i])));
        weightedScore += strokeScore * weight;
        totalWeight += weight;
    }

    if (!(totalWeight > 0.0)) {
        return -1.0;
    }
    return weightedScore / totalWeight;
}

} // namespace

void InputAutomationEngine::UpdateConfig(const InputAutomationConfig& config) {
    auto maxChainLengthForMappings = [](const std::vector<AutomationKeyBinding>& mappings, bool gestureBinding) {
        size_t maxLength = 1;
        for (const AutomationKeyBinding& binding : mappings) {
            if (!binding.enabled) {
                continue;
            }

            const size_t chainLength = gestureBinding
                ? automation_chain::NormalizedChainLength(binding.trigger, automation_ids::NormalizeGestureId)
                : automation_chain::NormalizedChainLength(binding.trigger, automation_ids::NormalizeMouseActionId);
            if (chainLength > maxLength) {
                maxLength = chainLength;
            }
        }
        return std::max<size_t>(1, maxLength);
    };
    auto maxCustomStrokeCountForMappings = [](const std::vector<AutomationKeyBinding>& mappings) {
        size_t maxStrokeCount = 1;
        for (const AutomationKeyBinding& binding : mappings) {
            if (!binding.enabled || !IsCustomGestureMode(binding)) {
                continue;
            }
            const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokes =
                TemplateStrokesForBinding(binding);
            if (!templateStrokes.empty()) {
                maxStrokeCount = std::max(maxStrokeCount, templateStrokes.size());
            }
        }
        return std::max<size_t>(1, maxStrokeCount);
    };

    config_ = config;
    gestureRecognizer_.UpdateConfig(BuildGestureConfig(config_));
    suppressNextClickActionId_.clear();
    mouseActionHistory_.clear();
    gestureHistory_.clear();
    customGestureStrokeHistory_.clear();
    mouseChainCap_ = maxChainLengthForMappings(config_.mouseMappings, false);
    gestureChainCap_ = maxChainLengthForMappings(config_.gesture.mappings, true);
    customGestureStrokeCap_ = maxCustomStrokeCountForMappings(config_.gesture.mappings);
    mouseChainTimingLimit_ = BuildMouseChainTimingLimit();
    gestureChainTimingLimit_ = BuildGestureChainTimingLimit();
}

void InputAutomationEngine::Reset() {
    gestureRecognizer_.Reset();
    suppressNextClickActionId_.clear();
    mouseActionHistory_.clear();
    gestureHistory_.clear();
    customGestureStrokeHistory_.clear();
    currentModifiers_ = {};
}

void InputAutomationEngine::OnMouseMove(const ScreenPoint& pt) {
    gestureRecognizer_.OnMouseMove(pt);
}

void InputAutomationEngine::OnButtonDown(const ScreenPoint& pt, int button) {
    gestureRecognizer_.OnButtonDown(pt, button);
}

void InputAutomationEngine::OnButtonUp(const ScreenPoint& pt, int button) {
    const GestureRecognizer::Result gesture = gestureRecognizer_.OnButtonUp(pt, button);
    if (gesture.button > 0 && !gesture.samplePoints.empty()) {
        AppendCustomGestureStroke(gesture.button, gesture.samplePoints);
    }
    if (TriggerGesture(gesture.gestureId, gesture.button)) {
        suppressNextClickActionId_ = automation_ids::NormalizeMouseActionId(ClickActionIdFromButtonCode(button));
    }
}

void InputAutomationEngine::OnClick(const ClickEvent& ev) {
    const std::string actionId = ClickActionId(ev.button);
    if (!suppressNextClickActionId_.empty()) {
        const bool shouldSuppress = (automation_ids::NormalizeMouseActionId(actionId) == suppressNextClickActionId_);
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

void InputAutomationEngine::OnKey(const KeyEvent& ev) {
    currentModifiers_ = ModifierStateFromKeyEvent(ev);
}

void InputAutomationEngine::SetForegroundProcessService(IForegroundProcessService* service) {
    foregroundProcessService_ = service;
}

void InputAutomationEngine::SetKeyboardInjector(IKeyboardInjector* injector) {
    keyboardInjector_ = injector;
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

InputModifierState InputAutomationEngine::ModifierStateFromKeyEvent(const KeyEvent& ev) {
    InputModifierState modifiers;
    modifiers.primary = ev.ctrl || ev.meta;
    modifiers.shift = ev.shift;
    modifiers.alt = ev.alt;
    return modifiers;
}

InputAutomationEngine::ChainTimingLimit InputAutomationEngine::BuildMouseChainTimingLimit() {
    ChainTimingLimit limit;
    limit.maxStepInterval = kMouseChainMaxStepIntervalMs;
    limit.maxTotalInterval = kMouseChainMaxTotalIntervalMs;
    return limit;
}

InputAutomationEngine::ChainTimingLimit InputAutomationEngine::BuildGestureChainTimingLimit() {
    ChainTimingLimit limit;
    limit.maxStepInterval = kGestureChainMaxStepIntervalMs;
    limit.maxTotalInterval = kGestureChainMaxTotalIntervalMs;
    return limit;
}

bool InputAutomationEngine::TriggerMouseAction(const std::string& actionId) {
    if (!config_.enabled || actionId.empty()) {
        return false;
    }
    return automation_dispatch::DispatchAction(
        config_.mouseMappings,
        &mouseActionHistory_,
        mouseChainCap_,
        mouseChainTimingLimit_,
        actionId,
        InputModifierState{},
        automation_ids::NormalizeMouseActionId,
        foregroundProcessService_,
        keyboardInjector_);
}

bool InputAutomationEngine::TriggerGesture(const std::string& gestureId, int button) {
    if (!config_.enabled || !config_.gesture.enabled) {
        return false;
    }
    const std::string triggerButton = ButtonNameFromCode(button);
    if (triggerButton.empty()) {
        return false;
    }
    if (TriggerCustomGesture(button, triggerButton)) {
        return true;
    }
    if (gestureId.empty()) {
        return false;
    }

    std::vector<AutomationKeyBinding> filteredMappings;
    filteredMappings.reserve(config_.gesture.mappings.size());
    for (const AutomationKeyBinding& binding : config_.gesture.mappings) {
        if (binding.triggerButton == triggerButton && !IsCustomGestureMode(binding)) {
            filteredMappings.push_back(binding);
        }
    }
    if (filteredMappings.empty()) {
        return false;
    }
    return automation_dispatch::DispatchAction(
        filteredMappings,
        &gestureHistory_,
        gestureChainCap_,
        gestureChainTimingLimit_,
        gestureId,
        currentModifiers_,
        automation_ids::NormalizeGestureId,
        foregroundProcessService_,
        keyboardInjector_);
}

bool InputAutomationEngine::TriggerCustomGesture(int button, const std::string& triggerButton) {
    if (!keyboardInjector_) {
        return false;
    }

    struct MatchCandidate final {
        const AutomationKeyBinding* binding = nullptr;
        size_t strokeCount = 0;
        double score = -1.0;
        int scopeSpecificity = -1;
    };

    const std::string processBaseName = foregroundProcessService_
        ? foregroundProcessService_->CurrentProcessBaseName()
        : std::string{};
    MatchCandidate best{};
    for (const AutomationKeyBinding& binding : config_.gesture.mappings) {
        if (!binding.enabled || binding.triggerButton != triggerButton || !IsCustomGestureMode(binding)) {
            continue;
        }
        if (!automation_scope::AppScopeMatchesProcess(binding.appScopes, processBaseName)) {
            continue;
        }
        if (!automation_match::ModifierConditionMatches(binding.modifiers, currentModifiers_)) {
            continue;
        }

        const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokes =
            TemplateStrokesForBinding(binding);
        if (templateStrokes.empty()) {
            continue;
        }

        std::vector<std::vector<ScreenPoint>> capturedStrokes;
        capturedStrokes.reserve(templateStrokes.size());
        for (auto it = customGestureStrokeHistory_.rbegin();
             it != customGestureStrokeHistory_.rend() &&
             capturedStrokes.size() < templateStrokes.size();
             ++it) {
            if (it->button == button) {
                capturedStrokes.push_back(it->points);
            }
        }
        if (capturedStrokes.size() != templateStrokes.size()) {
            continue;
        }
        std::reverse(capturedStrokes.begin(), capturedStrokes.end());

        if (gestureChainTimingLimit_.maxStepInterval.count() > 0 ||
            gestureChainTimingLimit_.maxTotalInterval.count() > 0) {
            std::vector<std::chrono::steady_clock::time_point> timestamps;
            timestamps.reserve(templateStrokes.size());
            for (auto it = customGestureStrokeHistory_.rbegin();
                 it != customGestureStrokeHistory_.rend() &&
                 timestamps.size() < templateStrokes.size();
                 ++it) {
                if (it->button == button) {
                    timestamps.push_back(it->timestamp);
                }
            }
            if (timestamps.size() != templateStrokes.size()) {
                continue;
            }
            std::reverse(timestamps.begin(), timestamps.end());
            bool timingMatched = true;
            if (gestureChainTimingLimit_.maxStepInterval.count() > 0) {
                for (size_t i = 1; i < timestamps.size(); ++i) {
                    if (timestamps[i] - timestamps[i - 1] > gestureChainTimingLimit_.maxStepInterval) {
                        timingMatched = false;
                        break;
                    }
                }
            }
            if (timingMatched &&
                gestureChainTimingLimit_.maxTotalInterval.count() > 0 &&
                timestamps.size() > 1 &&
                timestamps.back() - timestamps.front() > gestureChainTimingLimit_.maxTotalInterval) {
                timingMatched = false;
            }
            if (!timingMatched) {
                continue;
            }
        }

        const std::vector<std::vector<NormalizedPoint>> normalizedTemplate =
            NormalizeTemplateStrokes(templateStrokes);
        const std::vector<std::vector<NormalizedPoint>> normalizedCaptured =
            NormalizeCapturedStrokes(capturedStrokes);
        const double score = GestureSimilarityScore(normalizedTemplate, normalizedCaptured);
        if (score < 0.0) {
            continue;
        }

        const int threshold = std::clamp(binding.gesturePattern.matchThresholdPercent, 50, 95);
        if (score + 1e-6 < static_cast<double>(threshold)) {
            continue;
        }

        const int scopeSpecificity = automation_scope::AppScopeSpecificity(binding.appScopes);
        if (!best.binding ||
            score > best.score + 1e-6 ||
            (std::abs(score - best.score) <= 1e-6 && templateStrokes.size() > best.strokeCount) ||
            (std::abs(score - best.score) <= 1e-6 &&
             templateStrokes.size() == best.strokeCount &&
             scopeSpecificity > best.scopeSpecificity)) {
            best.binding = &binding;
            best.strokeCount = templateStrokes.size();
            best.score = score;
            best.scopeSpecificity = scopeSpecificity;
        }
    }

    if (!best.binding) {
        return false;
    }

    const std::string keys = TrimAscii(best.binding->keys);
    if (keys.empty() || !keyboardInjector_->SendChord(keys)) {
        return false;
    }
    ConsumeRecentCustomGestureStrokes(button, best.strokeCount);
    return true;
}

void InputAutomationEngine::AppendCustomGestureStroke(
    int button,
    const std::vector<ScreenPoint>& points) {
    if (button <= 0 || points.empty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    customGestureStrokeHistory_.push_back(CustomGestureStrokeEntry{
        button,
        points,
        now,
    });

    const size_t cap = std::max<size_t>(1, customGestureStrokeCap_);
    while (customGestureStrokeHistory_.size() > cap) {
        customGestureStrokeHistory_.erase(customGestureStrokeHistory_.begin());
    }
    if (gestureChainTimingLimit_.maxTotalInterval.count() > 0) {
        const auto oldestAllowed = now - gestureChainTimingLimit_.maxTotalInterval;
        while (customGestureStrokeHistory_.size() > 1 &&
               customGestureStrokeHistory_.front().timestamp < oldestAllowed) {
            customGestureStrokeHistory_.erase(customGestureStrokeHistory_.begin());
        }
    }
}

void InputAutomationEngine::ConsumeRecentCustomGestureStrokes(int button, size_t count) {
    if (button <= 0 || count == 0 || customGestureStrokeHistory_.empty()) {
        return;
    }

    for (size_t i = customGestureStrokeHistory_.size(); i > 0 && count > 0; --i) {
        const size_t index = i - 1;
        if (customGestureStrokeHistory_[index].button != button) {
            continue;
        }
        customGestureStrokeHistory_.erase(customGestureStrokeHistory_.begin() + static_cast<std::ptrdiff_t>(index));
        --count;
    }
}

} // namespace mousefx
