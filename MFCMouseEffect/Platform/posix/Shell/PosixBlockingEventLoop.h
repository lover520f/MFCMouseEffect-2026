#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace mousefx {

// Shared POSIX event-loop primitive: block until RequestExit() arrives.
class PosixBlockingEventLoop final {
public:
    int Run();
    void RequestExit();
    bool PostTask(std::function<void()> task);

private:
    std::mutex mutex_{};
    std::condition_variable cv_{};
    bool exitRequested_ = false;
    bool running_ = false;
    std::queue<std::function<void()>> taskQueue_{};
};

} // namespace mousefx
