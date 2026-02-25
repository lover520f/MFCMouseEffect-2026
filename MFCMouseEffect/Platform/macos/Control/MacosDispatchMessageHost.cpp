#include "pch.h"

#include "Platform/macos/Control/MacosDispatchMessageHost.h"

namespace mousefx {

MacosDispatchMessageHost::~MacosDispatchMessageHost() {
    Destroy();
}

bool MacosDispatchMessageHost::Create(IDispatchMessageHandler* handler) {
    if (IsCreated()) {
        return true;
    }
    if (!handler) {
        lastError_.store(kErrorInvalidParameter, std::memory_order_release);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        handler_ = handler;
        queue_.clear();
    }

    running_.store(true, std::memory_order_release);
    workerThread_ = std::thread([this]() { WorkerLoop(); });
    lastError_.store(kErrorSuccess, std::memory_order_release);
    return true;
}

void MacosDispatchMessageHost::Destroy() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        return;
    }

    StopAllTimers();
    cv_.notify_all();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
        handler_ = nullptr;
        workerThreadId_ = std::thread::id{};
    }

    lastError_.store(kErrorSuccess, std::memory_order_release);
}

bool MacosDispatchMessageHost::IsCreated() const {
    return running_.load(std::memory_order_acquire) && workerThread_.joinable();
}

bool MacosDispatchMessageHost::IsOwnerThread() const {
    return IsCreated() && std::this_thread::get_id() == workerThreadId_;
}

uintptr_t MacosDispatchMessageHost::NativeHandle() const {
    return reinterpret_cast<uintptr_t>(this);
}

uint32_t MacosDispatchMessageHost::LastError() const {
    return lastError_.load(std::memory_order_acquire);
}

} // namespace mousefx
