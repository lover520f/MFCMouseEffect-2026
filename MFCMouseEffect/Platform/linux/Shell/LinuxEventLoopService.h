#pragma once

#include "MouseFx/Core/Shell/IEventLoopService.h"

#include <atomic>

namespace mousefx {

class LinuxEventLoopService final : public IEventLoopService {
public:
    int Run() override;
    void RequestExit() override;

private:
    std::atomic<bool> exitRequested_{false};
};

} // namespace mousefx
