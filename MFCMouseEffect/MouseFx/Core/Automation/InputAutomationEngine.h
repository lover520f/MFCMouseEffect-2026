#pragma once

#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"
#include "MouseFx/Core/Input/GestureRecognizer.h"
#include "MouseFx/Core/Input/GestureSimilarity.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

namespace mousefx {

// Maps mouse actions and recognized gestures to keyboard shortcuts.
class InputAutomationEngine final {
public:
    struct GestureRouteEvent final {
        struct PreviewPoint final {
            int x = 0;
            int y = 0;
        };
        uint64_t seq = 0;
        uint64_t timestampMs = 0;
        std::string stage{};
        std::string reason{};
        std::string gestureId{};
        std::string recognizedGestureId{};
        std::string matchedGestureId{};
        std::string triggerButton{};
        bool matched = false;
        bool injected = false;
        bool usedCustom = false;
        bool usedPreset = false;
        int samplePointCount = 0;
        int candidateCount = 0;
        int bestWindowStart = -1;
        int bestWindowEnd = -1;
        double runnerUpScore = -1.0;
        uint64_t previewPathHash = 0;
        std::vector<PreviewPoint> previewPoints{};
        InputModifierState modifiers{};
    };

    struct Diagnostics final {
        bool automationEnabled = false;
        bool gestureEnabled = false;
        bool buttonlessGestureEnabled = false;
        bool pointerButtonDown = false;
        uint64_t gestureMappingCount = 0;
        uint64_t buttonlessGestureMappingCount = 0;
        std::string lastStage{};
        std::string lastReason{};
        std::string lastGestureId{};
        std::string lastRecognizedGestureId{};
        std::string lastMatchedGestureId{};
        std::string lastTriggerButton{};
        bool lastMatched = false;
        bool lastInjected = false;
        bool lastUsedCustom = false;
        bool lastUsedPreset = false;
        int lastSamplePointCount = 0;
        int lastCandidateCount = 0;
        int lastBestWindowStart = -1;
        int lastBestWindowEnd = -1;
        double lastRunnerUpScore = -1.0;
        uint64_t lastPreviewPathHash = 0;
        std::vector<GestureRouteEvent::PreviewPoint> lastPreviewPoints{};
        InputModifierState lastModifiers{};
        uint64_t lastEventSeq = 0;
        std::vector<GestureRouteEvent> recentEvents{};
    };

    InputAutomationEngine() = default;
    ~InputAutomationEngine() = default;

    InputAutomationEngine(const InputAutomationEngine&) = delete;
    InputAutomationEngine& operator=(const InputAutomationEngine&) = delete;

    void UpdateConfig(const InputAutomationConfig& config);
    void Reset();

    void OnMouseMove(const ScreenPoint& pt);
    void OnButtonDown(const ScreenPoint& pt, int button);
    void OnButtonUp(const ScreenPoint& pt, int button);
    void OnClick(const ClickEvent& ev);
    void OnScroll(short delta);
    void OnKey(const KeyEvent& ev);
    void SetForegroundProcessService(IForegroundProcessService* service);
    void SetKeyboardInjector(IKeyboardInjector* injector);
    void SetDiagnosticsEnabled(bool enabled);
    bool DiagnosticsEnabled() const;
    Diagnostics ReadDiagnostics() const;

private:
    struct CustomGestureStrokeEntry final {
        int button = 0;
        std::vector<ScreenPoint> points;
        std::vector<uint32_t> pointTimesMs;
        std::chrono::steady_clock::time_point timestamp{};
    };

    using ActionHistoryItem = automation_match::ActionHistoryEntry;
    using ChainTimingLimit = automation_match::ChainTimingLimit;

    static std::string ClickActionId(MouseButton button);
    static std::string ClickActionIdFromButtonCode(int button);
    static std::string ScrollActionId(short delta);
    static ChainTimingLimit BuildMouseChainTimingLimit();
    static ChainTimingLimit BuildGestureChainTimingLimit();
    static InputModifierState ModifierStateFromKeyEvent(const KeyEvent& ev);

    bool TriggerMouseAction(const std::string& actionId);
    bool TriggerGesture(
        const std::string& gestureId,
        int button,
        const std::vector<ScreenPoint>* currentStroke = nullptr,
        const std::vector<uint32_t>* currentStrokeTimesMs = nullptr,
        const std::vector<ScreenPoint>* previewStroke = nullptr);
    bool TriggerCustomGesture(
        const std::string& recognizedGestureId,
        int button,
        const std::string& triggerButton,
        const std::vector<ScreenPoint>* currentStroke = nullptr,
        const std::vector<uint32_t>* currentStrokeTimesMs = nullptr,
        const std::vector<ScreenPoint>* previewStroke = nullptr);
    void AppendCustomGestureStroke(
        int button,
        const std::vector<ScreenPoint>& points,
        const std::vector<uint32_t>* pointTimesMs = nullptr);
    void ConsumeRecentCustomGestureStrokes(int button, size_t count);
    void SetButtonState(int button, bool down);
    bool AnyPointerButtonDown() const;
    bool HasNoButtonGestureMappings() const;
    uint64_t CountNoButtonGestureMappings() const;
    void UpdateButtonlessGestureConfig();
    void ResetButtonlessGestureState();
    void HandleButtonlessGestureMove(const ScreenPoint& pt);
    void SetDiagnosticsConfigSnapshot();
    void UpdateGestureDiagnostics(
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
        size_t candidateCount = 0,
        const GestureMatchWindow* bestWindow = nullptr,
        double runnerUpScore = -1.0,
        const std::vector<ScreenPoint>* previewPoints = nullptr);
    bool ShouldAppendGestureRouteEventLocked(const GestureRouteEvent& event) const;

    InputAutomationConfig config_{};
    GestureRecognizer gestureRecognizer_{};
    GestureRecognizer buttonlessGestureRecognizer_{};
    IKeyboardInjector* keyboardInjector_ = nullptr;
    std::string suppressNextClickActionId_{};
    std::vector<ActionHistoryItem> mouseActionHistory_{};
    std::vector<ActionHistoryItem> gestureHistory_{};
    IForegroundProcessService* foregroundProcessService_ = nullptr;
    InputModifierState currentModifiers_{};
    size_t mouseChainCap_ = 1;
    size_t gestureChainCap_ = 1;
    size_t customGestureStrokeCap_ = 1;
    ChainTimingLimit mouseChainTimingLimit_{};
    ChainTimingLimit gestureChainTimingLimit_{};
    std::vector<CustomGestureStrokeEntry> customGestureStrokeHistory_{};
    bool leftButtonDown_ = false;
    bool rightButtonDown_ = false;
    bool middleButtonDown_ = false;
    bool buttonlessGestureEnabled_ = false;
    bool buttonlessGestureTriggered_ = false;
    std::string buttonlessLastGestureId_{};
    std::vector<ScreenPoint> buttonlessPreviewTrail_{};
    bool buttonlessHasLastMovePoint_ = false;
    ScreenPoint buttonlessLastMovePoint_{};
    std::chrono::steady_clock::time_point buttonlessLastMoveAt_{};
    std::chrono::steady_clock::time_point lastPressedGestureResultAt_{};
    std::atomic<bool> diagnosticsEnabled_{false};
    mutable std::mutex diagnosticsMutex_{};
    Diagnostics diagnostics_{};
    uint64_t diagnosticsEventSeq_ = 0;
};

} // namespace mousefx
