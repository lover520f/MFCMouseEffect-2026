#pragma once

namespace mousefx {

class IAppShellHost;

// Platform tray abstraction. Concrete implementations live in platform packages.
class ITrayService {
public:
    virtual ~ITrayService() = default;

    virtual bool Start(IAppShellHost* host, bool showTrayIcon) = 0;
    virtual void Stop() = 0;
    virtual void RequestExit() = 0;
};

} // namespace mousefx
