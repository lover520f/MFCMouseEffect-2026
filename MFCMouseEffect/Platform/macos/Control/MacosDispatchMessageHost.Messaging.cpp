#include "pch.h"

#include "Platform/macos/Control/MacosDispatchMessageHost.h"

#include "MouseFx/Core/Control/IDispatchMessageHandler.h"

#include <future>
#include <utility>

namespace mousefx {

intptr_t MacosDispatchMessageHost::SendSync(uint32_t msg, uintptr_t wParam, intptr_t lParam) {
    if (!IsCreated()) {
        lastError_.store(kErrorInvalidState, std::memory_order_release);
        return 0;
    }
    if (IsOwnerThread()) {
        return DispatchMessageOnWorker(msg, wParam, lParam);
    }

    auto promise = std::make_shared<std::promise<intptr_t>>();
    std::future<intptr_t> future = promise->get_future();
    PendingMessage pending{};
    pending.msg = msg;
    pending.wParam = wParam;
    pending.lParam = lParam;
    pending.syncResult = promise;
    if (!EnqueueMessage(std::move(pending))) {
        return 0;
    }

    return future.get();
}

bool MacosDispatchMessageHost::PostAsync(uint32_t msg, uintptr_t wParam, intptr_t lParam) {
    if (!IsCreated()) {
        lastError_.store(kErrorInvalidState, std::memory_order_release);
        return false;
    }
    PendingMessage pending{};
    pending.msg = msg;
    pending.wParam = wParam;
    pending.lParam = lParam;
    return EnqueueMessage(std::move(pending));
}

void MacosDispatchMessageHost::WorkerLoop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        workerThreadId_ = std::this_thread::get_id();
    }

    while (running_.load(std::memory_order_acquire)) {
        PendingMessage message{};
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() {
                return !running_.load(std::memory_order_acquire) || !queue_.empty();
            });
            if (!running_.load(std::memory_order_acquire) && queue_.empty()) {
                return;
            }
            message = std::move(queue_.front());
            queue_.pop_front();
        }

        const intptr_t result = DispatchMessageOnWorker(message.msg, message.wParam, message.lParam);
        if (message.syncResult) {
            message.syncResult->set_value(result);
        }
    }
}

intptr_t MacosDispatchMessageHost::DispatchMessageOnWorker(uint32_t msg, uintptr_t wParam, intptr_t lParam) {
    IDispatchMessageHandler* handler = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handler = handler_;
    }
    if (!handler) {
        lastError_.store(kErrorInvalidState, std::memory_order_release);
        return 0;
    }
    lastError_.store(kErrorSuccess, std::memory_order_release);
    return handler->OnDispatchMessage(NativeHandle(), msg, wParam, lParam);
}

bool MacosDispatchMessageHost::EnqueueMessage(PendingMessage message) {
    if (!running_.load(std::memory_order_acquire)) {
        lastError_.store(kErrorInvalidState, std::memory_order_release);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(message));
    }
    cv_.notify_one();
    return true;
}

} // namespace mousefx
