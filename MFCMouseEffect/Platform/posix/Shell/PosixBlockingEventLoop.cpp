#include "Platform/posix/Shell/PosixBlockingEventLoop.h"

#include <utility>

namespace mousefx {

int PosixBlockingEventLoop::Run() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (running_) {
        return -1;
    }

    running_ = true;
    for (;;) {
        cv_.wait(lock, [this]() { return exitRequested_ || !taskQueue_.empty(); });

        if (!taskQueue_.empty()) {
            auto task = std::move(taskQueue_.front());
            taskQueue_.pop();
            lock.unlock();
            if (task) {
                task();
            }
            lock.lock();
            continue;
        }

        if (exitRequested_) {
            break;
        }
    }

    exitRequested_ = false;
    running_ = false;
    return 0;
}

void PosixBlockingEventLoop::RequestExit() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        exitRequested_ = true;
    }
    cv_.notify_all();
}

bool PosixBlockingEventLoop::PostTask(std::function<void()> task) {
    if (!task) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        taskQueue_.push(std::move(task));
    }
    cv_.notify_all();
    return true;
}

} // namespace mousefx
