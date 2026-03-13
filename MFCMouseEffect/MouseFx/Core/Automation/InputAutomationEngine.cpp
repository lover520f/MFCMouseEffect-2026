#include "pch.h"
#include "InputAutomationEngine.h"

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/InputAutomationDispatch.h"
#include "MouseFx/Core/Automation/GestureMatchSelection.h"
#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Input/GestureSimilarity.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>

namespace mousefx {
namespace {

using automation_gesture_selection::Candidate;
using automation_gesture_selection::EffectiveRunnerUpScore;
using automation_gesture_selection::PreferLeftOverRight;

constexpr std::chrono::milliseconds kMouseChainMaxStepIntervalMs(900);
constexpr std::chrono::milliseconds kMouseChainMaxTotalIntervalMs(1800);
constexpr std::chrono::milliseconds kGestureChainMaxStepIntervalMs(2200);
constexpr std::chrono::milliseconds kGestureChainMaxTotalIntervalMs(5000);
constexpr std::chrono::milliseconds kButtonlessIdleResetMs(320);
constexpr std::chrono::milliseconds kPressedGestureResultHoldMs(900);
constexpr double kButtonlessArmMinMovePx = 16.0;
constexpr int kButtonlessThresholdBoostPercent = 6;
constexpr double kButtonlessRunnerUpMarginPercent = 8.0;
constexpr double kButtonlessMinPathLengthFactor = 1.35;
constexpr size_t kButtonlessMinSamplePointCount = 5;
constexpr size_t kButtonlessNoisyMinPoints = 18;
constexpr double kButtonlessNoisyMaxStartEndRatio = 0.16;
constexpr double kButtonlessNoisyHighTurnDensity = 3.1;
constexpr double kButtonlessNoisyFastPxPerMs = 2.8;
constexpr size_t kPressedNoisyMinPoints = 26;
constexpr double kPressedNoisyMaxStartEndRatio = 0.12;
constexpr double kPressedNoisyHighTurnDensity = 3.8;
constexpr double kPressedNoisyFastPxPerMs = 3.4;
constexpr size_t kDiagnosticsGestureEventCap = 30;
constexpr int kDefaultDebugPreviewMaxPoints = 180;
constexpr size_t kButtonlessPreviewTrailCap = 96;
constexpr double kDefaultCustomMinEffectiveStrokeLengthPx = 18.0;
constexpr int kButtonNone = 0;
constexpr int kButtonLeft = 1;
constexpr int kButtonRight = 2;
constexpr int kButtonMiddle = 3;

double SimilarityAmbiguityMargin(double bestScore) {
    const char* env = std::getenv("MFX_GESTURE_AMBIGUITY_MARGIN");
    if (env != nullptr) {
        try {
            const double overridden = std::stod(std::string(env));
            if (std::isfinite(overridden) && overridden >= 0.5 && overridden <= 20.0) {
                return overridden;
            }
        } catch (...) {
        }
    }
    if (bestScore >= 92.0) {
        return 1.5;
    }
    if (bestScore >= 86.0) {
        return 2.5;
    }
    if (bestScore >= 80.0) {
        return 3.5;
    }
    return 5.0;
}

double CustomMinEffectiveStrokeLengthPx() {
    const char* env = std::getenv("MFX_GESTURE_CUSTOM_MIN_EFFECTIVE_STROKE_PX");
    if (env != nullptr) {
        try {
            const double overridden = std::stod(std::string(env));
            if (std::isfinite(overridden) && overridden >= 0.0 && overridden <= 300.0) {
                return overridden;
            }
        } catch (...) {
        }
    }
    return kDefaultCustomMinEffectiveStrokeLengthPx;
}

int EnvIntClamped(const char* key, int fallback, int minValue, int maxValue) {
    const char* env = std::getenv(key);
    if (env == nullptr) {
        return fallback;
    }
    try {
        const int parsed = std::stoi(std::string(env));
        if (parsed < minValue || parsed > maxValue) {
            return fallback;
        }
        return parsed;
    } catch (...) {
        return fallback;
    }
}

int DebugPreviewPointCap() {
    return EnvIntClamped(
        "MFX_GESTURE_DEBUG_PREVIEW_MAX_POINTS",
        kDefaultDebugPreviewMaxPoints,
        32,
        512);
}

GestureMatchOptions BuildGestureMatchOptions(bool buttonlessRoute) {
    GestureMatchOptions options{};
    if (buttonlessRoute) {
        // Buttonless route benefits from tighter windows to reduce accidental triggers.
        options.windowCoverageMinPercent = 22;
        options.windowCoverageStepPercent = 8;
        options.windowSlideDivisor = 5;
        options.spatialWindowMaxCandidates = 88;
        options.timeWindowMinMs = 160;
        options.timeWindowMaxMs = 1450;
        options.timeWindowStepMs = 120;
        options.timeWindowAnchorStepMs = 70;
        options.timeWindowMaxCandidates = 72;
        options.timeResampleStepMs = 26;
        options.timeResampleMaxPoints = 88;
    } else {
        options.windowCoverageMinPercent = 18;
        options.windowCoverageStepPercent = 7;
        options.windowSlideDivisor = 5;
        options.spatialWindowMaxCandidates = 132;
        options.timeWindowMinMs = 140;
        options.timeWindowMaxMs = 1750;
        options.timeWindowStepMs = 110;
        options.timeWindowAnchorStepMs = 60;
        options.timeWindowMaxCandidates = 96;
        options.timeResampleStepMs = 22;
        options.timeResampleMaxPoints = 108;
    }

    options.windowCoverageMinPercent = EnvIntClamped(
        "MFX_GESTURE_WINDOW_COVERAGE_MIN_PERCENT",
        options.windowCoverageMinPercent,
        10,
        90);
    options.windowCoverageMaxPercent = EnvIntClamped(
        "MFX_GESTURE_WINDOW_COVERAGE_MAX_PERCENT",
        options.windowCoverageMaxPercent,
        options.windowCoverageMinPercent,
        100);
    options.windowCoverageStepPercent = EnvIntClamped(
        "MFX_GESTURE_WINDOW_COVERAGE_STEP_PERCENT",
        options.windowCoverageStepPercent,
        1,
        30);
    options.windowSlideDivisor = EnvIntClamped(
        "MFX_GESTURE_WINDOW_SLIDE_DIVISOR",
        options.windowSlideDivisor,
        1,
        10);
    options.spatialWindowMaxCandidates = EnvIntClamped(
        "MFX_GESTURE_WINDOW_SPATIAL_MAX_CANDIDATES",
        options.spatialWindowMaxCandidates,
        16,
        400);
    options.timeWindowMinMs = EnvIntClamped(
        "MFX_GESTURE_WINDOW_TIME_MIN_MS",
        options.timeWindowMinMs,
        60,
        4000);
    options.timeWindowMaxMs = EnvIntClamped(
        "MFX_GESTURE_WINDOW_TIME_MAX_MS",
        options.timeWindowMaxMs,
        options.timeWindowMinMs,
        8000);
    options.timeWindowStepMs = EnvIntClamped(
        "MFX_GESTURE_WINDOW_TIME_STEP_MS",
        options.timeWindowStepMs,
        10,
        1200);
    options.timeWindowAnchorStepMs = EnvIntClamped(
        "MFX_GESTURE_WINDOW_TIME_ANCHOR_STEP_MS",
        options.timeWindowAnchorStepMs,
        10,
        800);
    options.timeWindowMaxCandidates = EnvIntClamped(
        "MFX_GESTURE_WINDOW_TIME_MAX_CANDIDATES",
        options.timeWindowMaxCandidates,
        8,
        400);
    options.timeResampleStepMs = EnvIntClamped(
        "MFX_GESTURE_TIME_RESAMPLE_STEP_MS",
        options.timeResampleStepMs,
        8,
        120);
    options.timeResampleMaxPoints = EnvIntClamped(
        "MFX_GESTURE_TIME_RESAMPLE_MAX_POINTS",
        options.timeResampleMaxPoints,
        16,
        300);
    return options;
}

GestureRecognitionConfig BuildGestureConfig(const InputAutomationConfig& config) {
    GestureRecognitionConfig out;
    out.enabled = config.enabled && config.gesture.enabled;
    out.minStrokeDistancePx = config.gesture.minStrokeDistancePx;
    out.sampleStepPx = config.gesture.sampleStepPx;
    out.maxDirections = config.gesture.maxDirections;
    return out;
}

std::string ButtonNameFromCode(int button) {
    if (button == kButtonNone) return "none";
    if (button == kButtonLeft) return "left";
    if (button == kButtonRight) return "right";
    if (button == kButtonMiddle) return "middle";
    return {};
}

std::string CanonicalGestureId(const std::string& value) {
    return automation_ids::NormalizeGestureId(value);
}

size_t CountGestureDirectionUnits(const std::string& gestureId) {
    const std::string normalized = CanonicalGestureId(gestureId);
    if (normalized.empty()) {
        return 0;
    }
    std::vector<std::string> tokens;
    size_t from = 0;
    while (from < normalized.size()) {
        const size_t pos = normalized.find('_', from);
        if (pos == std::string::npos) {
            tokens.push_back(normalized.substr(from));
            break;
        }
        tokens.push_back(normalized.substr(from, pos - from));
        from = pos + 1;
    }
    size_t count = 0;
    for (size_t i = 0; i < tokens.size();) {
        const std::string& token = tokens[i];
        if (token == "diag") {
            // diag_<vertical>_<horizontal> counts as one direction unit.
            ++count;
            if (i + 2 < tokens.size()) {
                i += 3;
            } else {
                ++i;
            }
            continue;
        }
        ++count;
        ++i;
    }
    return count;
}

double MatchCoverageRatio(const GestureMatchWindow& window, size_t capturedPointCount) {
    if (capturedPointCount == 0) {
        return 1.0;
    }
    const size_t start = std::min(window.start, capturedPointCount);
    const size_t end = std::min(window.end, capturedPointCount);
    if (end <= start) {
        return 1.0;
    }
    return std::clamp(
        static_cast<double>(end - start) / static_cast<double>(capturedPointCount),
        0.0,
        1.0);
}

bool ShouldRejectContainedSimpleCandidate(
    size_t recognizedDirectionUnits,
    size_t candidateDirectionUnits,
    double coverageRatio) {
    if (recognizedDirectionUnits < 4 || candidateDirectionUnits == 0) {
        return false;
    }
    if (recognizedDirectionUnits < candidateDirectionUnits + 2) {
        return false;
    }
    // Generic containment suppression:
    // if recognizer sees a much more complex chain but this candidate only
    // explains a partial local window, treat it as likely sub-shape hijack.
    return coverageRatio < 0.84;
}

std::vector<InputAutomationEngine::GestureRouteEvent::PreviewPoint> BuildPreviewPoints(
    const std::vector<ScreenPoint>* points) {
    std::vector<InputAutomationEngine::GestureRouteEvent::PreviewPoint> out;
    if (!points || points->empty()) {
        return out;
    }
    const size_t kMaxPreviewPoints = static_cast<size_t>(DebugPreviewPointCap());
    const size_t count = points->size();
    if (count <= kMaxPreviewPoints) {
        out.reserve(count);
        for (const ScreenPoint& pt : *points) {
            out.push_back({pt.x, pt.y});
        }
        return out;
    }
    out.reserve(kMaxPreviewPoints);
    const double step = static_cast<double>(count - 1) / static_cast<double>(kMaxPreviewPoints - 1);
    for (size_t i = 0; i < kMaxPreviewPoints; ++i) {
        const size_t index = static_cast<size_t>(std::round(step * static_cast<double>(i)));
        const size_t safeIndex = std::min(index, count - 1);
        const ScreenPoint& pt = (*points)[safeIndex];
        out.push_back({pt.x, pt.y});
    }
    return out;
}

std::vector<ScreenPoint> FlattenStrokePreviewPoints(
    const std::vector<std::vector<ScreenPoint>>& strokes) {
    std::vector<ScreenPoint> out;
    size_t reserveCount = 0;
    for (const auto& stroke : strokes) {
        reserveCount += stroke.size();
    }
    if (reserveCount == 0) {
        return out;
    }
    out.reserve(reserveCount);
    for (const auto& stroke : strokes) {
        if (stroke.empty()) {
            continue;
        }
        out.insert(out.end(), stroke.begin(), stroke.end());
    }
    return out;
}

std::vector<AutomationKeyBinding::GesturePoint> FlattenTemplateStrokePoints(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& strokes) {
    std::vector<AutomationKeyBinding::GesturePoint> out;
    size_t reserveCount = 0;
    for (const auto& stroke : strokes) {
        reserveCount += stroke.size();
    }
    if (reserveCount == 0) {
        return out;
    }
    out.reserve(reserveCount);
    for (const auto& stroke : strokes) {
        out.insert(out.end(), stroke.begin(), stroke.end());
    }
    return out;
}

uint64_t HashPreviewPoints(
    const std::vector<InputAutomationEngine::GestureRouteEvent::PreviewPoint>& points) {
    // FNV-1a 64-bit
    uint64_t hash = 1469598103934665603ull;
    const auto mix = [&hash](uint64_t value) {
        hash ^= value;
        hash *= 1099511628211ull;
    };
    for (const auto& pt : points) {
        mix(static_cast<uint64_t>(static_cast<uint32_t>(pt.x)));
        mix(static_cast<uint64_t>(static_cast<uint32_t>(pt.y)));
    }
    mix(static_cast<uint64_t>(points.size()));
    return hash;
}

bool IsCustomGestureMode(const AutomationKeyBinding& binding) {
    return IsCustomGesturePatternMode(binding.gesturePattern.mode);
}

std::string NormalizedGestureTailActionId(const AutomationKeyBinding& binding) {
    const std::vector<std::string> chain =
        automation_chain::NormalizeChainTokens(binding.trigger, automation_ids::NormalizeGestureId);
    if (chain.empty()) {
        return {};
    }
    return chain.back();
}

struct ButtonlessDispatchGuard final {
    bool accepted = true;
    const char* reason = "buttonless_candidate_ready";
};

ButtonlessDispatchGuard EvaluateButtonlessGestureGuard(
    const InputAutomationConfig& config,
    const GestureSimilarityMetrics& metrics,
    double bestScore,
    double runnerUpScore,
    int thresholdPercent) {
    const double minPathLengthPx = std::max(
        static_cast<double>(std::max(config.gesture.minStrokeDistancePx, 1)) * kButtonlessMinPathLengthFactor,
        96.0);
    if (metrics.pointCount < kButtonlessMinSamplePointCount ||
        metrics.pathLengthPx + 1e-6 < minPathLengthPx) {
        return ButtonlessDispatchGuard{false, "buttonless_candidate_too_short"};
    }

    const int guardedThreshold = std::clamp(
        thresholdPercent + kButtonlessThresholdBoostPercent,
        50,
        98);
    if (bestScore + 1e-6 < static_cast<double>(guardedThreshold)) {
        return ButtonlessDispatchGuard{false, "buttonless_candidate_below_guard_threshold"};
    }
    if (runnerUpScore >= 0.0 &&
        bestScore - runnerUpScore < kButtonlessRunnerUpMarginPercent) {
        return ButtonlessDispatchGuard{false, "buttonless_candidate_ambiguous"};
    }
    return {};
}

bool IsNoisyMotion(
    const std::vector<ScreenPoint>& points,
    const std::vector<uint32_t>* sampleTimesMs,
    bool buttonlessRoute) {
    const size_t minPoints = buttonlessRoute ? kButtonlessNoisyMinPoints : kPressedNoisyMinPoints;
    const double noisyMaxStartEndRatio = buttonlessRoute ? kButtonlessNoisyMaxStartEndRatio : kPressedNoisyMaxStartEndRatio;
    const double noisyHighTurnDensity = buttonlessRoute ? kButtonlessNoisyHighTurnDensity : kPressedNoisyHighTurnDensity;
    const double noisyFastPxPerMs = buttonlessRoute ? kButtonlessNoisyFastPxPerMs : kPressedNoisyFastPxPerMs;
    if (points.size() < minPoints) {
        return false;
    }
    double pathLength = 0.0;
    size_t turnCount = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        const double dx = static_cast<double>(points[i].x) - static_cast<double>(points[i - 1].x);
        const double dy = static_cast<double>(points[i].y) - static_cast<double>(points[i - 1].y);
        const double segLen = std::sqrt(dx * dx + dy * dy);
        pathLength += segLen;
        if (i < 2) {
            continue;
        }
        const double pdx = static_cast<double>(points[i - 1].x) - static_cast<double>(points[i - 2].x);
        const double pdy = static_cast<double>(points[i - 1].y) - static_cast<double>(points[i - 2].y);
        const double prevLen = std::sqrt(pdx * pdx + pdy * pdy);
        if (prevLen < 1e-6 || segLen < 1e-6) {
            continue;
        }
        const double dot = std::clamp((pdx * dx + pdy * dy) / (prevLen * segLen), -1.0, 1.0);
        const double angleDeg = std::acos(dot) * (180.0 / 3.14159265358979323846);
        if (angleDeg >= 62.0) {
            ++turnCount;
        }
    }
    if (pathLength < 120.0) {
        return false;
    }
    const double startEndDx = static_cast<double>(points.back().x) - static_cast<double>(points.front().x);
    const double startEndDy = static_cast<double>(points.back().y) - static_cast<double>(points.front().y);
    const double startEndDist = std::sqrt(startEndDx * startEndDx + startEndDy * startEndDy);
    const double startEndRatio = startEndDist / std::max(1.0, pathLength);
    const double turnDensity = static_cast<double>(turnCount) / std::max(1.0, pathLength / 50.0);

    bool fastAndChaotic = false;
    if (sampleTimesMs && sampleTimesMs->size() == points.size() && sampleTimesMs->back() > 0) {
        const double durationMs = static_cast<double>(sampleTimesMs->back());
        const double avgPxPerMs = pathLength / durationMs;
        fastAndChaotic = (avgPxPerMs >= noisyFastPxPerMs && turnDensity >= (buttonlessRoute ? 1.6 : 2.1));
    }
    const bool denseChaos = (turnDensity >= noisyHighTurnDensity && turnCount >= (buttonlessRoute ? 7 : 10));
    const bool closedScribble = (startEndRatio <= noisyMaxStartEndRatio && turnCount >= (buttonlessRoute ? 6 : 9));
    return denseChaos || closedScribble || (buttonlessRoute && fastAndChaotic);
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
                GestureTemplateStrokesFromPattern(binding.gesturePattern);
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
    leftButtonDown_ = false;
    rightButtonDown_ = false;
    middleButtonDown_ = false;
    UpdateButtonlessGestureConfig();
    SetDiagnosticsConfigSnapshot();
    UpdateGestureDiagnostics(
        "config_updated",
        "ready",
        {},
        {},
        buttonlessGestureEnabled_ ? "none" : "",
        false,
        false,
        false,
        false,
        0);
}

void InputAutomationEngine::Reset() {
    gestureRecognizer_.Reset();
    ResetButtonlessGestureState();
    suppressNextClickActionId_.clear();
    mouseActionHistory_.clear();
    gestureHistory_.clear();
    customGestureStrokeHistory_.clear();
    currentModifiers_ = {};
    leftButtonDown_ = false;
    rightButtonDown_ = false;
    middleButtonDown_ = false;
    UpdateGestureDiagnostics("runtime_reset", "state_cleared", {}, {}, {}, false, false, false, false, 0);
}

void InputAutomationEngine::OnMouseMove(const ScreenPoint& pt) {
    gestureRecognizer_.OnMouseMove(pt);
    if (DiagnosticsEnabled() && AnyPointerButtonDown()) {
        const GestureRecognizer::Result snapshot = gestureRecognizer_.Snapshot();
        const std::string recognizedId = CanonicalGestureId(snapshot.gestureId);
        UpdateGestureDiagnostics(
            "gesture_drag_snapshot",
            recognizedId.empty() ? "collecting" : "gesture_id_ready",
            recognizedId,
            {},
            ButtonNameFromCode(snapshot.button),
            false,
            false,
            false,
            false,
            snapshot.samplePoints.size(),
            0,
            nullptr,
            -1.0,
            snapshot.previewPoints.empty() ? nullptr : &snapshot.previewPoints);
    }
    HandleButtonlessGestureMove(pt);
}

void InputAutomationEngine::OnButtonDown(const ScreenPoint& pt, int button) {
    SetButtonState(button, true);
    ResetButtonlessGestureState();
    gestureRecognizer_.OnButtonDown(pt, button);
}

void InputAutomationEngine::OnButtonUp(const ScreenPoint& pt, int button) {
    SetButtonState(button, false);
    const GestureRecognizer::Result gesture = gestureRecognizer_.OnButtonUp(pt, button);
    if (gesture.button > 0 && !gesture.samplePoints.empty()) {
        AppendCustomGestureStroke(
            gesture.button,
            gesture.samplePoints,
            gesture.sampleTimesMs.empty() ? nullptr : &gesture.sampleTimesMs);
    }
    const bool injected = TriggerGesture(
            gesture.gestureId,
            gesture.button,
            gesture.samplePoints.empty() ? nullptr : &gesture.samplePoints,
            gesture.sampleTimesMs.empty() ? nullptr : &gesture.sampleTimesMs,
            gesture.previewPoints.empty() ? nullptr : &gesture.previewPoints);
    if (gesture.button > 0 && !gesture.samplePoints.empty()) {
        lastPressedGestureResultAt_ = std::chrono::steady_clock::now();
    }
    if (injected) {
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
    SetDiagnosticsConfigSnapshot();
}

void InputAutomationEngine::SetForegroundProcessService(IForegroundProcessService* service) {
    foregroundProcessService_ = service;
}

void InputAutomationEngine::SetKeyboardInjector(IKeyboardInjector* injector) {
    keyboardInjector_ = injector;
}

void InputAutomationEngine::SetDiagnosticsEnabled(bool enabled) {
    diagnosticsEnabled_.store(enabled, std::memory_order_release);
    if (!enabled) {
        std::lock_guard<std::mutex> lock(diagnosticsMutex_);
        diagnostics_ = Diagnostics{};
        diagnosticsEventSeq_ = 0;
        return;
    }
    SetDiagnosticsConfigSnapshot();
    UpdateGestureDiagnostics(
        "diagnostics_enabled",
        "ready",
        {},
        {},
        {},
        false,
        false,
        false,
        false,
        0);
}

bool InputAutomationEngine::DiagnosticsEnabled() const {
    return diagnosticsEnabled_.load(std::memory_order_acquire);
}

InputAutomationEngine::Diagnostics InputAutomationEngine::ReadDiagnostics() const {
    if (!DiagnosticsEnabled()) {
        return Diagnostics{};
    }
    std::lock_guard<std::mutex> lock(diagnosticsMutex_);
    return diagnostics_;
}

void InputAutomationEngine::SetButtonState(int button, bool down) {
    if (button == kButtonLeft) {
        leftButtonDown_ = down;
        return;
    }
    if (button == kButtonRight) {
        rightButtonDown_ = down;
        return;
    }
    if (button == kButtonMiddle) {
        middleButtonDown_ = down;
    }
}

bool InputAutomationEngine::AnyPointerButtonDown() const {
    return leftButtonDown_ || rightButtonDown_ || middleButtonDown_;
}

bool InputAutomationEngine::HasNoButtonGestureMappings() const {
    if (!config_.enabled || !config_.gesture.enabled) {
        return false;
    }
    return CountNoButtonGestureMappings() > 0;
}

uint64_t InputAutomationEngine::CountNoButtonGestureMappings() const {
    uint64_t count = 0;
    for (const AutomationKeyBinding& binding : config_.gesture.mappings) {
        if (binding.enabled && binding.triggerButton == "none") {
            ++count;
        }
    }
    return count;
}

void InputAutomationEngine::SetDiagnosticsConfigSnapshot() {
    if (!DiagnosticsEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(diagnosticsMutex_);
    diagnostics_.automationEnabled = config_.enabled;
    diagnostics_.gestureEnabled = config_.gesture.enabled;
    diagnostics_.buttonlessGestureEnabled = buttonlessGestureEnabled_;
    diagnostics_.pointerButtonDown = AnyPointerButtonDown();
    diagnostics_.gestureMappingCount = static_cast<uint64_t>(config_.gesture.mappings.size());
    diagnostics_.buttonlessGestureMappingCount = CountNoButtonGestureMappings();
}

void InputAutomationEngine::UpdateGestureDiagnostics(
    const char* stage,
    const char* reason,
    const std::string& recognizedGestureId,
    const std::string& matchedGestureId,
    const std::string& triggerButton,
    bool matched,
    bool injected,
    bool usedCustom,
    bool usedPreset,
    size_t samplePointCount,
    size_t candidateCount,
    const GestureMatchWindow* bestWindow,
    double runnerUpScore,
    const std::vector<ScreenPoint>* previewPoints) {
    if (!DiagnosticsEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(diagnosticsMutex_);
    diagnostics_.automationEnabled = config_.enabled;
    diagnostics_.gestureEnabled = config_.gesture.enabled;
    diagnostics_.buttonlessGestureEnabled = buttonlessGestureEnabled_;
    diagnostics_.pointerButtonDown = AnyPointerButtonDown();
    diagnostics_.gestureMappingCount = static_cast<uint64_t>(config_.gesture.mappings.size());
    diagnostics_.buttonlessGestureMappingCount = CountNoButtonGestureMappings();
    diagnostics_.lastStage = stage ? stage : "";
    diagnostics_.lastReason = reason ? reason : "";
    diagnostics_.lastRecognizedGestureId = recognizedGestureId;
    diagnostics_.lastMatchedGestureId = matchedGestureId;
    diagnostics_.lastGestureId = !matchedGestureId.empty() ? matchedGestureId : recognizedGestureId;
    diagnostics_.lastTriggerButton = triggerButton;
    diagnostics_.lastMatched = matched;
    diagnostics_.lastInjected = injected;
    diagnostics_.lastUsedCustom = usedCustom;
    diagnostics_.lastUsedPreset = usedPreset;
    diagnostics_.lastSamplePointCount = static_cast<int>(samplePointCount);
    diagnostics_.lastCandidateCount = static_cast<int>(candidateCount);
    diagnostics_.lastBestWindowStart = bestWindow ? static_cast<int>(bestWindow->start) : -1;
    diagnostics_.lastBestWindowEnd = bestWindow ? static_cast<int>(bestWindow->end) : -1;
    diagnostics_.lastRunnerUpScore = runnerUpScore;
    diagnostics_.lastPreviewPoints = BuildPreviewPoints(previewPoints);
    diagnostics_.lastPreviewPathHash = HashPreviewPoints(diagnostics_.lastPreviewPoints);
    diagnostics_.lastModifiers = currentModifiers_;

    GestureRouteEvent event{};
    event.timestampMs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    event.stage = diagnostics_.lastStage;
    event.reason = diagnostics_.lastReason;
    event.gestureId = diagnostics_.lastGestureId;
    event.recognizedGestureId = diagnostics_.lastRecognizedGestureId;
    event.matchedGestureId = diagnostics_.lastMatchedGestureId;
    event.triggerButton = diagnostics_.lastTriggerButton;
    event.matched = diagnostics_.lastMatched;
    event.injected = diagnostics_.lastInjected;
    event.usedCustom = diagnostics_.lastUsedCustom;
    event.usedPreset = diagnostics_.lastUsedPreset;
    event.samplePointCount = diagnostics_.lastSamplePointCount;
    event.candidateCount = diagnostics_.lastCandidateCount;
    event.bestWindowStart = diagnostics_.lastBestWindowStart;
    event.bestWindowEnd = diagnostics_.lastBestWindowEnd;
    event.runnerUpScore = diagnostics_.lastRunnerUpScore;
    event.previewPoints = diagnostics_.lastPreviewPoints;
    event.previewPathHash = diagnostics_.lastPreviewPathHash;
    event.modifiers = diagnostics_.lastModifiers;

    if (ShouldAppendGestureRouteEventLocked(event)) {
        event.seq = ++diagnosticsEventSeq_;
        diagnostics_.recentEvents.push_back(event);
        while (diagnostics_.recentEvents.size() > kDiagnosticsGestureEventCap) {
            diagnostics_.recentEvents.erase(diagnostics_.recentEvents.begin());
        }
        diagnostics_.lastEventSeq = event.seq;
    } else if (!diagnostics_.recentEvents.empty()) {
        diagnostics_.lastEventSeq = diagnostics_.recentEvents.back().seq;
    }
}

bool InputAutomationEngine::ShouldAppendGestureRouteEventLocked(const GestureRouteEvent& event) const {
    if (diagnostics_.recentEvents.empty()) {
        return true;
    }
    const GestureRouteEvent& last = diagnostics_.recentEvents.back();
    return
        last.stage != event.stage ||
        last.reason != event.reason ||
        last.gestureId != event.gestureId ||
        last.recognizedGestureId != event.recognizedGestureId ||
        last.matchedGestureId != event.matchedGestureId ||
        last.triggerButton != event.triggerButton ||
        last.matched != event.matched ||
        last.injected != event.injected ||
        last.usedCustom != event.usedCustom ||
        last.usedPreset != event.usedPreset ||
        last.samplePointCount != event.samplePointCount ||
        last.candidateCount != event.candidateCount ||
        last.bestWindowStart != event.bestWindowStart ||
        last.bestWindowEnd != event.bestWindowEnd ||
        std::abs(last.runnerUpScore - event.runnerUpScore) > 1e-6 ||
        last.previewPathHash != event.previewPathHash ||
        last.modifiers.primary != event.modifiers.primary ||
        last.modifiers.shift != event.modifiers.shift ||
        last.modifiers.alt != event.modifiers.alt;
}

void InputAutomationEngine::UpdateButtonlessGestureConfig() {
    buttonlessGestureEnabled_ = HasNoButtonGestureMappings();

    GestureRecognitionConfig buttonlessConfig = BuildGestureConfig(config_);
    buttonlessConfig.enabled = buttonlessGestureEnabled_;
    buttonlessConfig.triggerButton = "none";
    buttonlessGestureRecognizer_.UpdateConfig(buttonlessConfig);
    ResetButtonlessGestureState();
    SetDiagnosticsConfigSnapshot();
}

void InputAutomationEngine::ResetButtonlessGestureState() {
    buttonlessGestureRecognizer_.Reset();
    buttonlessGestureTriggered_ = false;
    buttonlessLastGestureId_.clear();
    buttonlessPreviewTrail_.clear();
    buttonlessHasLastMovePoint_ = false;
    buttonlessLastMovePoint_ = {};
    buttonlessLastMoveAt_ = {};
    SetDiagnosticsConfigSnapshot();
}

void InputAutomationEngine::HandleButtonlessGestureMove(const ScreenPoint& pt) {
    const auto now = std::chrono::steady_clock::now();
    if (buttonlessPreviewTrail_.empty() ||
        buttonlessPreviewTrail_.back().x != pt.x ||
        buttonlessPreviewTrail_.back().y != pt.y) {
        buttonlessPreviewTrail_.push_back(pt);
        if (buttonlessPreviewTrail_.size() > kButtonlessPreviewTrailCap) {
            buttonlessPreviewTrail_.erase(buttonlessPreviewTrail_.begin());
        }
    }
    if (AnyPointerButtonDown()) {
        ResetButtonlessGestureState();
        return;
    }
    if (!buttonlessGestureEnabled_) {
        ResetButtonlessGestureState();
        return;
    }
    if (lastPressedGestureResultAt_.time_since_epoch().count() > 0 &&
        now - lastPressedGestureResultAt_ < kPressedGestureResultHoldMs) {
        ResetButtonlessGestureState();
        return;
    }

    const bool hadPreviousMove = buttonlessLastMoveAt_.time_since_epoch().count() > 0;
    const auto idleGap = hadPreviousMove
        ? now - buttonlessLastMoveAt_
        : std::chrono::steady_clock::duration::zero();
    if (buttonlessGestureRecognizer_.IsActive() &&
        hadPreviousMove &&
        idleGap > kButtonlessIdleResetMs) {
        UpdateGestureDiagnostics(
            "buttonless_idle_reset",
            "idle_timeout",
            {},
            {},
            "none",
            false,
            false,
            false,
            false,
            0);
        ResetButtonlessGestureState();
    }

    // Once buttonless gesture already triggered, keep the idle arm state fresh
    // but stop feeding new points into recognizer to avoid endless preview growth.
    if (buttonlessGestureTriggered_) {
        UpdateGestureDiagnostics(
            "buttonless_move_skipped",
            "post_trigger_hold",
            buttonlessLastGestureId_,
            {},
            "none",
            false,
            false,
            false,
            false,
            buttonlessPreviewTrail_.size(),
            0,
            nullptr,
            -1.0,
            buttonlessPreviewTrail_.empty() ? nullptr : &buttonlessPreviewTrail_);
        buttonlessLastMovePoint_ = pt;
        buttonlessHasLastMovePoint_ = true;
        buttonlessLastMoveAt_ = now;
        return;
    }

    if (!buttonlessGestureRecognizer_.IsActive()) {
        if (!buttonlessHasLastMovePoint_) {
            UpdateGestureDiagnostics(
                "buttonless_move_skipped",
                "awaiting_first_motion_arm",
                {},
                {},
                "none",
                false,
                false,
                false,
                false,
                0);
            buttonlessLastMovePoint_ = pt;
            buttonlessHasLastMovePoint_ = true;
            buttonlessLastMoveAt_ = now;
            return;
        }
        const double dx = static_cast<double>(pt.x) - static_cast<double>(buttonlessLastMovePoint_.x);
        const double dy = static_cast<double>(pt.y) - static_cast<double>(buttonlessLastMovePoint_.y);
        const double movedDistance = std::sqrt(dx * dx + dy * dy);
        if (movedDistance + 1e-6 < kButtonlessArmMinMovePx) {
            UpdateGestureDiagnostics(
                "buttonless_move_skipped",
                "awaiting_motion_arm",
                {},
                {},
                "none",
                false,
                false,
                false,
                false,
                buttonlessPreviewTrail_.size(),
                0,
                nullptr,
                -1.0,
                buttonlessPreviewTrail_.empty() ? nullptr : &buttonlessPreviewTrail_);
            buttonlessLastMovePoint_ = pt;
            buttonlessHasLastMovePoint_ = true;
            buttonlessLastMoveAt_ = now;
            return;
        }
        buttonlessGestureRecognizer_.OnButtonDown(buttonlessLastMovePoint_, kButtonNone);
        buttonlessGestureRecognizer_.OnMouseMove(pt);
        UpdateGestureDiagnostics(
            "buttonless_arm",
            "motion_arm_ready",
            {},
            {},
            "none",
            false,
            false,
            false,
            false,
            0);
        buttonlessLastMovePoint_ = pt;
        buttonlessHasLastMovePoint_ = true;
        buttonlessLastMoveAt_ = now;
    } else {
        buttonlessGestureRecognizer_.OnMouseMove(pt);
        buttonlessLastMovePoint_ = pt;
        buttonlessHasLastMovePoint_ = true;
        buttonlessLastMoveAt_ = now;
    }
    const GestureRecognizer::Result snapshot = buttonlessGestureRecognizer_.Snapshot();
    const std::string recognizedGestureId = CanonicalGestureId(snapshot.gestureId);
    if (recognizedGestureId.empty()) {
        UpdateGestureDiagnostics(
            "buttonless_snapshot",
            "gesture_id_empty",
            {},
            {},
            "none",
            false,
            false,
            false,
            false,
            snapshot.samplePoints.size(),
            0,
            nullptr,
            -1.0,
            snapshot.samplePoints.empty() ? nullptr : &snapshot.samplePoints);
        buttonlessLastGestureId_.clear();
        return;
    }
    if (IsNoisyMotion(
            snapshot.samplePoints,
            snapshot.sampleTimesMs.empty() ? nullptr : &snapshot.sampleTimesMs,
            true)) {
        UpdateGestureDiagnostics(
            "buttonless_snapshot",
            "buttonless_noisy_motion_filtered",
            recognizedGestureId,
            {},
            "none",
            false,
            false,
            false,
            false,
            snapshot.samplePoints.size(),
            0,
            nullptr,
            -1.0,
            snapshot.samplePoints.empty() ? nullptr : &snapshot.samplePoints);
        buttonlessLastGestureId_ = recognizedGestureId;
        return;
    }
    if (TriggerGesture(
            recognizedGestureId,
            kButtonNone,
            snapshot.samplePoints.empty() ? nullptr : &snapshot.samplePoints,
            snapshot.sampleTimesMs.empty() ? nullptr : &snapshot.sampleTimesMs,
            snapshot.previewPoints.empty() ? nullptr : &snapshot.previewPoints)) {
        buttonlessLastGestureId_ = recognizedGestureId;
        buttonlessGestureTriggered_ = true;
    } else {
        buttonlessLastGestureId_.clear();
    }
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
    if (button == kButtonLeft) return "left_click";
    if (button == kButtonRight) return "right_click";
    if (button == kButtonMiddle) return "middle_click";
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
    automation_dispatch::DispatchTrace trace{};
    const bool injected = automation_dispatch::DispatchAction(
        config_.mouseMappings,
        &mouseActionHistory_,
        mouseChainCap_,
        mouseChainTimingLimit_,
        actionId,
        InputModifierState{},
        automation_ids::NormalizeMouseActionId,
        foregroundProcessService_,
        keyboardInjector_,
        &trace);
    (void)trace;
    return injected;
}

bool InputAutomationEngine::TriggerGesture(
    const std::string& gestureId,
    int button,
    const std::vector<ScreenPoint>* currentStroke,
    const std::vector<uint32_t>* currentStrokeTimesMs,
    const std::vector<ScreenPoint>* previewStroke) {
    const std::vector<ScreenPoint>* previewForDiagnostics =
        (previewStroke && !previewStroke->empty()) ? previewStroke : currentStroke;
    if (!config_.enabled || !config_.gesture.enabled) {
        const std::string recognizedGestureId = CanonicalGestureId(gestureId);
        UpdateGestureDiagnostics(
            "gesture_trigger",
            !config_.enabled ? "automation_disabled" : "gesture_disabled",
            recognizedGestureId,
            {},
            {},
            false,
            false,
            false,
            false,
            currentStroke ? currentStroke->size() : 0);
        return false;
    }
    const std::string triggerButton = ButtonNameFromCode(button);
    const std::string recognizedGestureId = CanonicalGestureId(gestureId);
    if (triggerButton.empty()) {
        UpdateGestureDiagnostics(
            "gesture_trigger",
            "unknown_button_code",
            recognizedGestureId,
            {},
            {},
            false,
            false,
            false,
            false,
            currentStroke ? currentStroke->size() : 0);
        return false;
    }
    if (triggerButton != "none" && currentStroke && !currentStroke->empty()) {
        if (IsNoisyMotion(
                *currentStroke,
                (currentStrokeTimesMs && currentStrokeTimesMs->size() == currentStroke->size())
                    ? currentStrokeTimesMs
                    : nullptr,
                false)) {
            UpdateGestureDiagnostics(
                "gesture_trigger",
                "pressed_noisy_motion_filtered",
                recognizedGestureId,
                {},
                triggerButton,
                false,
                false,
                false,
                false,
                currentStroke->size(),
                0,
                nullptr,
                -1.0,
                previewForDiagnostics);
            return false;
        }
    }
    if (TriggerCustomGesture(
            recognizedGestureId,
            button,
            triggerButton,
            currentStroke,
            currentStrokeTimesMs,
            previewForDiagnostics)) {
        return true;
    }
    std::vector<AutomationKeyBinding> filteredMappings;
    filteredMappings.reserve(config_.gesture.mappings.size());
    for (const AutomationKeyBinding& binding : config_.gesture.mappings) {
        if (binding.triggerButton == triggerButton && !IsCustomGestureMode(binding)) {
            filteredMappings.push_back(binding);
        }
    }
    if (filteredMappings.empty()) {
        UpdateGestureDiagnostics(
            "gesture_trigger",
            "no_preset_mapping_for_button",
            recognizedGestureId,
            {},
            triggerButton,
            false,
            false,
            false,
            true,
            currentStroke ? currentStroke->size() : 0,
            0,
            nullptr,
            -1.0,
            previewForDiagnostics);
        return false;
    }

    std::vector<AutomationKeyBinding> dispatchMappings = filteredMappings;
    std::string dispatchGestureId = recognizedGestureId;
    bool usedPresetSimilarity = false;
    double bestSimilarityScore = -1.0;
    double runnerUpSimilarityScore = -1.0;
    int bestSimilarityThreshold = 75;
    int bestSimilarityScope = -1;
    GestureMatchWindow bestSimilarityWindow{};
    size_t bestSimilarityCandidateCount = 0;
    std::string similarityRejectReason =
        (currentStroke && !currentStroke->empty()) ? "preset_window_not_matched" : "gesture_id_empty";
    const GestureSimilarityMetrics currentStrokeMetrics =
        (currentStroke && !currentStroke->empty())
            ? MeasureCapturedGesture({*currentStroke})
            : GestureSimilarityMetrics{};
    const size_t recognizedDirectionUnits = CountGestureDirectionUnits(recognizedGestureId);
    const GestureMatchOptions matchOptions = BuildGestureMatchOptions(triggerButton == "none");

    if (currentStroke && !currentStroke->empty()) {
        struct PresetSimilarityCandidate final {
            AutomationKeyBinding binding;
            std::string actionId;
            GestureMatchResult result{};
            GestureTemplateProfile profile{};
            int threshold = 75;
            int scopeSpecificity = -1;
            size_t capturedPointCount = 0;
        };

        std::vector<PresetSimilarityCandidate> candidates;
        candidates.reserve(filteredMappings.size());
        std::string bestActionId;

        for (const AutomationKeyBinding& binding : filteredMappings) {
            if (!binding.enabled) {
                continue;
            }
            const std::string actionId = NormalizedGestureTailActionId(binding);
            if (actionId.empty()) {
                continue;
            }

            const GestureMatchResult result =
                (currentStrokeTimesMs && currentStrokeTimesMs->size() == currentStroke->size())
                ? MatchPresetGestureSimilarity(actionId, *currentStroke, *currentStrokeTimesMs, matchOptions)
                : MatchPresetGestureSimilarity(actionId, *currentStroke, matchOptions);
            if (result.bestScore < 0.0) {
                continue;
            }
            const int threshold = std::clamp(binding.gesturePattern.matchThresholdPercent, 50, 95);
            if (result.bestScore + 1e-6 < static_cast<double>(threshold)) {
                continue;
            }
            const size_t candidateDirectionUnits = CountGestureDirectionUnits(actionId);
            const double coverageRatio = MatchCoverageRatio(result.bestWindow, currentStroke->size());
            if (ShouldRejectContainedSimpleCandidate(
                    recognizedDirectionUnits,
                    candidateDirectionUnits,
                    coverageRatio)) {
                continue;
            }

            const int scopeSpecificity = automation_scope::AppScopeSpecificity(binding.appScopes);
            candidates.push_back(PresetSimilarityCandidate{
                binding,
                actionId,
                result,
                MeasurePresetGestureProfile(actionId),
                threshold,
                scopeSpecificity,
                currentStroke->size(),
            });

            const Candidate currentCandidate{
                result.bestScore,
                result.runnerUpScore,
                result.bestWindow,
                result.candidateCount,
                MeasurePresetGestureProfile(actionId),
                scopeSpecificity,
                threshold,
                currentStroke->size(),
            };
            const Candidate bestCandidate{
                bestSimilarityScore,
                runnerUpSimilarityScore,
                bestSimilarityWindow,
                bestSimilarityCandidateCount,
                bestActionId.empty() ? GestureTemplateProfile{} : MeasurePresetGestureProfile(bestActionId),
                bestSimilarityScope,
                bestSimilarityThreshold,
                currentStroke->size(),
            };

            if (bestActionId.empty() || PreferLeftOverRight(currentCandidate, bestCandidate)) {
                bestActionId = actionId;
                bestSimilarityScore = result.bestScore;
                bestSimilarityWindow = result.bestWindow;
                bestSimilarityCandidateCount = result.candidateCount;
                runnerUpSimilarityScore = result.runnerUpScore;
                bestSimilarityThreshold = threshold;
                bestSimilarityScope = scopeSpecificity;
            }
        }

        if (!bestActionId.empty()) {
            dispatchMappings.clear();
            std::vector<Candidate> rankedCandidates;
            rankedCandidates.reserve(candidates.size());
            for (const PresetSimilarityCandidate& candidate : candidates) {
                if (candidate.actionId == bestActionId) {
                    dispatchMappings.push_back(candidate.binding);
                    continue;
                }
                rankedCandidates.push_back(Candidate{
                    candidate.result.bestScore,
                    candidate.result.runnerUpScore,
                    candidate.result.bestWindow,
                    candidate.result.candidateCount,
                    candidate.profile,
                    candidate.scopeSpecificity,
                    candidate.threshold,
                    candidate.capturedPointCount,
                });
            }
            const Candidate winnerCandidate{
                bestSimilarityScore,
                runnerUpSimilarityScore,
                bestSimilarityWindow,
                bestSimilarityCandidateCount,
                MeasurePresetGestureProfile(bestActionId),
                bestSimilarityScope,
                bestSimilarityThreshold,
                currentStroke->size(),
            };
            runnerUpSimilarityScore = EffectiveRunnerUpScore(winnerCandidate, rankedCandidates);
            if (runnerUpSimilarityScore >= 0.0) {
                const double ambiguityMargin = SimilarityAmbiguityMargin(bestSimilarityScore);
                if (bestSimilarityScore - runnerUpSimilarityScore < ambiguityMargin) {
                    similarityRejectReason = "preset_window_ambiguous";
                    dispatchMappings.clear();
                    dispatchGestureId.clear();
                    bestActionId.clear();
                }
            }
            if (triggerButton == "none") {
                const ButtonlessDispatchGuard guard = EvaluateButtonlessGestureGuard(
                    config_,
                    currentStrokeMetrics,
                    bestSimilarityScore,
                    runnerUpSimilarityScore,
                    bestSimilarityThreshold);
                if (!guard.accepted) {
                    similarityRejectReason = guard.reason;
                    dispatchMappings.clear();
                    dispatchGestureId.clear();
                    bestActionId.clear();
                }
            }
        }

        if (!bestActionId.empty()) {
            dispatchGestureId = bestActionId;
            usedPresetSimilarity = true;
        }
    }

    if (dispatchGestureId.empty()) {
        UpdateGestureDiagnostics(
            "gesture_trigger",
            similarityRejectReason.c_str(),
            recognizedGestureId,
            {},
            triggerButton,
            false,
            false,
            false,
            true,
            currentStroke ? currentStroke->size() : 0,
            bestSimilarityCandidateCount,
            bestSimilarityScore >= 0.0 ? &bestSimilarityWindow : nullptr,
            runnerUpSimilarityScore,
            previewForDiagnostics);
        return false;
    }

    automation_dispatch::DispatchTrace trace{};
    const bool injected = automation_dispatch::DispatchAction(
        dispatchMappings,
        &gestureHistory_,
        gestureChainCap_,
        gestureChainTimingLimit_,
        dispatchGestureId,
        currentModifiers_,
        automation_ids::NormalizeGestureId,
        foregroundProcessService_,
        keyboardInjector_,
        &trace);
    std::string reason = "preset_binding_not_matched";
    if (usedPresetSimilarity) {
        reason = "preset_window_binding_not_matched";
        if (!trace.actionAccepted) {
            reason = "preset_window_action_not_accepted";
        } else if (trace.bindingMatched && !injected) {
            reason = "preset_window_binding_matched_but_inject_failed";
        } else if (injected) {
            reason = "preset_window_injected";
        }
    } else {
        if (!trace.actionAccepted) {
            reason = "preset_action_not_accepted";
        } else if (trace.bindingMatched && !injected) {
            reason = "preset_binding_matched_but_inject_failed";
        } else if (injected) {
            reason = "preset_binding_injected";
        }
    }
    UpdateGestureDiagnostics(
        "gesture_trigger",
        reason.c_str(),
        recognizedGestureId,
        trace.bindingMatched ? dispatchGestureId : std::string{},
        triggerButton,
        trace.bindingMatched,
        injected,
        false,
        true,
        currentStroke ? currentStroke->size() : 0,
        bestSimilarityCandidateCount,
        bestSimilarityScore >= 0.0 ? &bestSimilarityWindow : nullptr,
        runnerUpSimilarityScore,
        previewForDiagnostics);
    return injected;
}

bool InputAutomationEngine::TriggerCustomGesture(
    const std::string& recognizedGestureId,
    int button,
    const std::string& triggerButton,
    const std::vector<ScreenPoint>* currentStroke,
    const std::vector<uint32_t>* currentStrokeTimesMs,
    const std::vector<ScreenPoint>* previewStroke) {
    const std::vector<ScreenPoint>* previewForDiagnostics =
        (previewStroke && !previewStroke->empty()) ? previewStroke : currentStroke;
    const std::string diagnosticsMatchedGestureId = recognizedGestureId;
    if (!keyboardInjector_) {
        UpdateGestureDiagnostics(
            "custom_trigger",
            "keyboard_injector_unavailable",
            recognizedGestureId,
            {},
            triggerButton,
            false,
            false,
            true,
            false,
            currentStroke ? currentStroke->size() : 0,
            0,
            nullptr,
            -1.0,
            previewForDiagnostics);
        return false;
    }

    struct MatchCandidate final {
        const AutomationKeyBinding* binding = nullptr;
        size_t strokeCount = 0;
        GestureMatchResult result{};
        GestureTemplateProfile profile{};
        int scopeSpecificity = -1;
        int threshold = 75;
        size_t capturedPointCount = 0;
        std::vector<ScreenPoint> previewPoints{};
    };

    const std::string processBaseName = foregroundProcessService_
        ? foregroundProcessService_->CurrentProcessBaseName()
        : std::string{};
    std::vector<MatchCandidate> matches;
    matches.reserve(config_.gesture.mappings.size());
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
            GestureTemplateStrokesFromPattern(binding.gesturePattern);
        if (templateStrokes.empty()) {
            continue;
        }
        std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokesForMatch =
            templateStrokes;

        std::vector<std::vector<ScreenPoint>> capturedStrokes;
        std::vector<std::vector<uint32_t>> capturedStrokeTimesMs;
        std::vector<std::chrono::steady_clock::time_point> timestamps;
        const bool buttonlessCurrentStrokeMode =
            (triggerButton == "none" && currentStroke && !currentStroke->empty());
        if (buttonlessCurrentStrokeMode) {
            // For buttonless mode there is no physical stroke boundary; for multi-stroke
            // templates flatten into a single ordered polyline to keep matching generic.
            if (templateStrokesForMatch.size() > 1) {
                std::vector<AutomationKeyBinding::GesturePoint> flattenedTemplate =
                    FlattenTemplateStrokePoints(templateStrokesForMatch);
                if (flattenedTemplate.size() < 2) {
                    continue;
                }
                templateStrokesForMatch.clear();
                templateStrokesForMatch.push_back(std::move(flattenedTemplate));
            }
            if (templateStrokesForMatch.size() != 1) {
                continue;
            }
            capturedStrokes.push_back(*currentStroke);
            if (currentStrokeTimesMs && currentStrokeTimesMs->size() == currentStroke->size()) {
                capturedStrokeTimesMs.push_back(*currentStrokeTimesMs);
            }
        } else {
            capturedStrokes.reserve(templateStrokes.size());
            capturedStrokeTimesMs.reserve(templateStrokes.size());
            timestamps.reserve(templateStrokes.size());
            for (auto it = customGestureStrokeHistory_.rbegin();
                 it != customGestureStrokeHistory_.rend() &&
                 capturedStrokes.size() < templateStrokes.size();
                 ++it) {
                if (it->button == button) {
                    capturedStrokes.push_back(it->points);
                    capturedStrokeTimesMs.push_back(it->pointTimesMs);
                    timestamps.push_back(it->timestamp);
                }
            }
        }
        if (capturedStrokes.size() != templateStrokesForMatch.size()) {
            continue;
        }
        std::reverse(capturedStrokes.begin(), capturedStrokes.end());
        std::reverse(capturedStrokeTimesMs.begin(), capturedStrokeTimesMs.end());
        std::reverse(timestamps.begin(), timestamps.end());

        if (!timestamps.empty() &&
            (gestureChainTimingLimit_.maxStepInterval.count() > 0 ||
             gestureChainTimingLimit_.maxTotalInterval.count() > 0)) {
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

        GestureMatchOptions options = BuildGestureMatchOptions(triggerButton == "none");
        options.enableWindowSearch = true;
        options.strictStrokeCount = true;
        options.strictStrokeOrder = true;
        options.minEffectiveStrokeLengthPx = CustomMinEffectiveStrokeLengthPx();
        bool hasAlignedStrokeTimes = capturedStrokeTimesMs.size() == capturedStrokes.size();
        if (hasAlignedStrokeTimes) {
            for (size_t i = 0; i < capturedStrokeTimesMs.size(); ++i) {
                if (capturedStrokeTimesMs[i].empty() ||
                    capturedStrokeTimesMs[i].size() != capturedStrokes[i].size()) {
                    hasAlignedStrokeTimes = false;
                    break;
                }
            }
        }
        const GestureMatchResult result = hasAlignedStrokeTimes
            ? MatchGestureTemplateSimilarity(templateStrokesForMatch, capturedStrokes, capturedStrokeTimesMs, options)
            : MatchGestureTemplateSimilarity(templateStrokesForMatch, capturedStrokes, options);
        if (result.bestScore < 0.0) {
            continue;
        }

        const int threshold = std::clamp(binding.gesturePattern.matchThresholdPercent, 50, 95);
        if (result.bestScore + 1e-6 < static_cast<double>(threshold)) {
            continue;
        }

        const int scopeSpecificity = automation_scope::AppScopeSpecificity(binding.appScopes);
        size_t capturedPointCount = 0;
        for (const auto& stroke : capturedStrokes) {
            capturedPointCount += stroke.size();
        }
        matches.push_back(MatchCandidate{
            &binding,
            templateStrokes.size(),
            result,
            MeasureGestureTemplateProfile(templateStrokesForMatch),
            scopeSpecificity,
            threshold,
            capturedPointCount,
            FlattenStrokePreviewPoints(capturedStrokes),
        });
    }

    MatchCandidate best{};
    for (const MatchCandidate& candidate : matches) {
        if (!best.binding ||
            PreferLeftOverRight(
                Candidate{
                    candidate.result.bestScore,
                    candidate.result.runnerUpScore,
                    candidate.result.bestWindow,
                    candidate.result.candidateCount,
                    candidate.profile,
                    candidate.scopeSpecificity,
                    candidate.threshold,
                    candidate.capturedPointCount,
                },
                Candidate{
                    best.result.bestScore,
                    best.result.runnerUpScore,
                    best.result.bestWindow,
                    best.result.candidateCount,
                    best.profile,
                    best.scopeSpecificity,
                    best.threshold,
                    best.capturedPointCount,
                })) {
            best = candidate;
        }
    }
    if (!best.binding) {
        UpdateGestureDiagnostics(
            "custom_trigger",
            "custom_window_not_matched",
            recognizedGestureId,
            {},
            triggerButton,
            false,
            false,
            true,
            false,
            currentStroke ? currentStroke->size() : 0,
            0,
            nullptr,
            -1.0,
            previewForDiagnostics);
        return false;
    }
    const std::vector<ScreenPoint>* selectedPreviewForDiagnostics =
        !best.previewPoints.empty()
            ? &best.previewPoints
            : previewForDiagnostics;

    std::vector<Candidate> rankedCandidates;
    rankedCandidates.reserve(matches.size());
    for (const MatchCandidate& candidate : matches) {
        if (candidate.binding == best.binding) {
            continue;
        }
        rankedCandidates.push_back(Candidate{
            candidate.result.bestScore,
            candidate.result.runnerUpScore,
            candidate.result.bestWindow,
            candidate.result.candidateCount,
            candidate.profile,
            candidate.scopeSpecificity,
            candidate.threshold,
            candidate.capturedPointCount,
        });
    }
    double runnerUpScore = EffectiveRunnerUpScore(
        Candidate{
            best.result.bestScore,
            best.result.runnerUpScore,
            best.result.bestWindow,
            best.result.candidateCount,
            best.profile,
            best.scopeSpecificity,
            best.threshold,
            best.capturedPointCount,
        },
        rankedCandidates);
    if (runnerUpScore >= 0.0) {
        const double ambiguityMargin = SimilarityAmbiguityMargin(best.result.bestScore);
        if (best.result.bestScore - runnerUpScore < ambiguityMargin) {
            UpdateGestureDiagnostics(
                "custom_trigger",
                "custom_window_ambiguous",
                recognizedGestureId,
                {},
                triggerButton,
                false,
                false,
                true,
                false,
                currentStroke ? currentStroke->size() : 0,
                best.result.candidateCount,
                &best.result.bestWindow,
                runnerUpScore,
                selectedPreviewForDiagnostics);
            return false;
        }
    }

    if (triggerButton == "none" && currentStroke && !currentStroke->empty()) {
        const ButtonlessDispatchGuard guard = EvaluateButtonlessGestureGuard(
            config_,
            MeasureCapturedGesture({*currentStroke}),
            best.result.bestScore,
            runnerUpScore,
            best.threshold);
        if (!guard.accepted) {
            UpdateGestureDiagnostics(
                "custom_trigger",
                guard.reason,
                recognizedGestureId,
                {},
                triggerButton,
                false,
                false,
                true,
                false,
                currentStroke ? currentStroke->size() : 0,
                best.result.candidateCount,
                &best.result.bestWindow,
                runnerUpScore,
                previewForDiagnostics);
            return false;
        }
    }

    const std::string keys = TrimAscii(best.binding->keys);
    if (keys.empty()) {
        UpdateGestureDiagnostics(
            "custom_trigger",
            "custom_mapping_missing_keys",
            recognizedGestureId,
            diagnosticsMatchedGestureId,
            triggerButton,
            true,
            false,
            true,
            false,
            currentStroke ? currentStroke->size() : 0,
            best.result.candidateCount,
            &best.result.bestWindow,
            runnerUpScore,
            selectedPreviewForDiagnostics);
        return false;
    }
    if (!keyboardInjector_->SendChord(keys)) {
        UpdateGestureDiagnostics(
            "custom_trigger",
            "custom_mapping_inject_failed",
            recognizedGestureId,
            diagnosticsMatchedGestureId,
            triggerButton,
            true,
            false,
            true,
            false,
            currentStroke ? currentStroke->size() : 0,
            best.result.candidateCount,
            &best.result.bestWindow,
            runnerUpScore,
            selectedPreviewForDiagnostics);
        return false;
    }
    if (triggerButton != "none") {
        ConsumeRecentCustomGestureStrokes(button, best.strokeCount);
    }
    UpdateGestureDiagnostics(
        "custom_trigger",
        "custom_window_injected",
        recognizedGestureId,
        diagnosticsMatchedGestureId,
        triggerButton,
        true,
        true,
        true,
        false,
        currentStroke ? currentStroke->size() : 0,
        best.result.candidateCount,
        &best.result.bestWindow,
        runnerUpScore,
        selectedPreviewForDiagnostics);
    return true;
}

void InputAutomationEngine::AppendCustomGestureStroke(
    int button,
    const std::vector<ScreenPoint>& points,
    const std::vector<uint32_t>* pointTimesMs) {
    if (button <= 0 || points.empty()) {
        return;
    }

    std::vector<uint32_t> effectiveTimes;
    if (pointTimesMs && pointTimesMs->size() == points.size()) {
        effectiveTimes = *pointTimesMs;
    } else {
        effectiveTimes.reserve(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            effectiveTimes.push_back(static_cast<uint32_t>(i * 12));
        }
    }

    const auto now = std::chrono::steady_clock::now();
    customGestureStrokeHistory_.push_back(CustomGestureStrokeEntry{
        button,
        points,
        std::move(effectiveTimes),
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
