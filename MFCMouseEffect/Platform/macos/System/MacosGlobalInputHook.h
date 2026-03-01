#pragma once

#include "MouseFx/Core/System/IGlobalMouseHook.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace mousefx {

class IDispatchMessageHost;

class MacosGlobalInputHook final : public IGlobalMouseHook {
public:
    MacosGlobalInputHook() = default;
    ~MacosGlobalInputHook() override;

    MacosGlobalInputHook(const MacosGlobalInputHook&) = delete;
    MacosGlobalInputHook& operator=(const MacosGlobalInputHook&) = delete;

    bool Start(IDispatchMessageHost* dispatchHost) override;
    void Stop() override;
    uint32_t LastError() const override;
    bool ConsumeLatestMove(ScreenPoint& outPt) override;
    void SetKeyboardCaptureExclusive(bool enabled) override;

private:
    static constexpr uint32_t kErrorSuccess = 0;
    static constexpr uint32_t kErrorInvalidParameter = 22;
    static constexpr uint32_t kErrorPermissionDenied = 13;
    static constexpr uint32_t kErrorTapCreateFailed = 1001;
    static constexpr uint32_t kErrorSourceCreateFailed = 1002;
#if defined(__APPLE__)
    static constexpr CFTimeInterval kPermissionProbeIntervalSeconds = 0.5;
#endif
    static constexpr uint32_t kPermissionSimulationPollIntervalMs = 120;

private:
    void RunEventTapLoop();
    void NotifyInitResult(bool ok, uint32_t errorCode);
    bool RunPermissionSimulationLoop(const std::string& simulationFilePath, bool trusted);
    CGEventMask ComputeEventTapMask() const;
    void OnPermissionProbeTimer();
    void HandleTapDisabledEvent();
    void HandleMouseMoveEvent(CGEventRef event);
    void HandleScrollEvent(CGEventRef event);
    void HandleMouseDownEvent(CGEventRef event);
    void HandleMouseUpEvent(CGEventRef event);
    CGEventRef HandleKeyDownEvent(CGEventRef event);
    ScreenPoint SanitizeMouseMovePoint(const ScreenPoint& rawPt);
    void RememberStableMovePoint(const ScreenPoint& pt);
    bool TryReadStableMovePoint(ScreenPoint* outPt) const;
#if defined(__APPLE__)
    static CGEventRef EventTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo);
    static void PermissionProbeTimerCallback(CFRunLoopTimerRef timer, void* userInfo);
#endif

private:
    IDispatchMessageHost* dispatchHost_ = nullptr;
    std::thread tapThread_{};
    std::atomic<bool> running_{false};
    std::atomic<uint32_t> lastError_{0};
    std::atomic<bool> keyboardCaptureExclusive_{false};
    std::atomic<bool> movePending_{false};
    std::atomic<int32_t> latestMoveX_{0};
    std::atomic<int32_t> latestMoveY_{0};
    std::atomic<bool> hasStableMovePoint_{false};
    std::atomic<int32_t> stableMoveX_{0};
    std::atomic<int32_t> stableMoveY_{0};
    std::atomic<uint32_t> originMoveStreak_{0};

    std::mutex initMutex_{};
    std::condition_variable initCv_{};
    bool initDone_ = false;
    bool initOk_ = false;

    mutable std::mutex runLoopMutex_{};
    void* runLoopRef_ = nullptr;
    void* tapRef_ = nullptr;
    void* sourceRef_ = nullptr;
};

} // namespace mousefx
