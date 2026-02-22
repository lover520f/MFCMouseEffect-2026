#pragma once

#include "MouseFx/Core/Shell/IEventLoopService.h"

#include <functional>
#include <mutex>
#include <queue>

namespace mousefx {

class Win32EventLoopService final : public IEventLoopService {
public:
    int Run() override;
    void RequestExit() override;
    bool PostTask(std::function<void()> task) override;

private:
    static constexpr unsigned int kTaskMessage = 0x8000u + 0x31u;

    void DrainTaskQueue();

private:
    std::mutex taskMutex_{};
    std::queue<std::function<void()>> taskQueue_{};
    unsigned long loopThreadId_ = 0;
};

} // namespace mousefx
