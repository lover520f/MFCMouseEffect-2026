#pragma once

namespace mousefx {

// Platform event-loop abstraction.
class IEventLoopService {
public:
    virtual ~IEventLoopService() = default;

    virtual int Run() = 0;
    virtual void RequestExit() = 0;
};

} // namespace mousefx
