#pragma once

#include <functional>

namespace mousefx {

// Platform event-loop abstraction.
class IEventLoopService {
public:
    virtual ~IEventLoopService() = default;

    virtual int Run() = 0;
    virtual void RequestExit() = 0;
    virtual bool PostTask(std::function<void()> task) = 0;
};

} // namespace mousefx
