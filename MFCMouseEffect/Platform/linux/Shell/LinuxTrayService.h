#pragma once

#include "MouseFx/Core/Shell/ITrayService.h"

namespace mousefx {

class LinuxTrayService final : public ITrayService {
public:
    bool Start(IAppShellHost* host, bool showTrayIcon) override;
    void Stop() override;
    void RequestExit() override;
};

} // namespace mousefx
