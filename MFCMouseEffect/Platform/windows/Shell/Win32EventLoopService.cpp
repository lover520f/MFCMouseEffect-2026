#include "pch.h"

#include "Platform/windows/Shell/Win32EventLoopService.h"

namespace mousefx {

int Win32EventLoopService::Run() {
    loopThreadId_ = GetCurrentThreadId();
    MSG msg{};
    PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE);

    DrainTaskQueue();

    for (;;) {
        const BOOL r = GetMessageW(&msg, nullptr, 0, 0);
        if (r == 0) {
            loopThreadId_ = 0;
            return static_cast<int>(msg.wParam);
        }
        if (r == -1) {
            loopThreadId_ = 0;
            return -1;
        }

        if (msg.message == kTaskMessage) {
            DrainTaskQueue();
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32EventLoopService::RequestExit() {
    PostQuitMessage(0);
}

bool Win32EventLoopService::PostTask(std::function<void()> task) {
    if (!task) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        taskQueue_.push(std::move(task));
    }

    if (loopThreadId_ != 0) {
        PostThreadMessageW(loopThreadId_, kTaskMessage, 0, 0);
    }
    return true;
}

void Win32EventLoopService::DrainTaskQueue() {
    std::queue<std::function<void()>> pending{};
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        if (taskQueue_.empty()) {
            return;
        }
        pending.swap(taskQueue_);
    }

    while (!pending.empty()) {
        auto task = std::move(pending.front());
        pending.pop();
        if (task) {
            task();
        }
    }
}

} // namespace mousefx
