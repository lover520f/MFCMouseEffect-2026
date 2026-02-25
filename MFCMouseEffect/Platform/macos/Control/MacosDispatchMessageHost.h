#pragma once

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace mousefx {

class IDispatchMessageHandler;

// macOS dispatch host backed by a dedicated worker thread.
class MacosDispatchMessageHost final : public IDispatchMessageHost {
public:
    MacosDispatchMessageHost() = default;
    ~MacosDispatchMessageHost() override;

    MacosDispatchMessageHost(const MacosDispatchMessageHost&) = delete;
    MacosDispatchMessageHost& operator=(const MacosDispatchMessageHost&) = delete;

    bool Create(IDispatchMessageHandler* handler) override;
    void Destroy() override;
    bool IsCreated() const override;
    bool IsOwnerThread() const override;
    uintptr_t NativeHandle() const override;
    uint32_t LastError() const override;
    intptr_t SendSync(uint32_t msg, uintptr_t wParam, intptr_t lParam) override;
    bool PostAsync(uint32_t msg, uintptr_t wParam, intptr_t lParam) override;
    bool SetTimer(uintptr_t timerId, uint32_t intervalMs) override;
    void KillTimer(uintptr_t timerId) override;

private:
    static constexpr uint32_t kErrorSuccess = 0;
    static constexpr uint32_t kErrorInvalidParameter = 22;
    static constexpr uint32_t kErrorInvalidState = 125;

private:
    struct PendingMessage {
        uint32_t msg = 0;
        uintptr_t wParam = 0;
        intptr_t lParam = 0;
        std::shared_ptr<std::promise<intptr_t>> syncResult;
    };

    struct TimerThreadSlot {
        std::shared_ptr<std::atomic<bool>> running;
        std::thread thread;
    };

    void WorkerLoop();
    intptr_t DispatchMessageOnWorker(uint32_t msg, uintptr_t wParam, intptr_t lParam);
    bool EnqueueMessage(PendingMessage message);
    void StopAllTimers();

private:
    mutable std::mutex mutex_{};
    std::condition_variable cv_{};
    std::deque<PendingMessage> queue_{};
    std::thread workerThread_{};
    std::thread::id workerThreadId_{};
    IDispatchMessageHandler* handler_ = nullptr;
    std::atomic<bool> running_{false};
    std::atomic<uint32_t> lastError_{0};

    std::mutex timerMutex_{};
    std::unordered_map<uintptr_t, TimerThreadSlot> timers_{};
};

} // namespace mousefx
