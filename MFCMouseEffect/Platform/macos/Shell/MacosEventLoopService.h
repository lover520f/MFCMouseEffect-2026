#pragma once

#include "MouseFx/Core/Shell/IEventLoopService.h"
#include "Platform/posix/Shell/PosixBlockingEventLoop.h"

namespace mousefx {

class MacosEventLoopService final : public IEventLoopService {
public:
    int Run() override;
    void RequestExit() override;
    bool PostTask(std::function<void()> task) override;

private:
    PosixBlockingEventLoop loop_{};
};

} // namespace mousefx
