#pragma once

#include "MouseFx/Core/Shell/IEventLoopService.h"

namespace mousefx {

class Win32EventLoopService final : public IEventLoopService {
public:
    int Run() override;
    void RequestExit() override;
};

} // namespace mousefx
