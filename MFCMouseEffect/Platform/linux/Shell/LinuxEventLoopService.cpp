#include "Platform/linux/Shell/LinuxEventLoopService.h"

#include <utility>

namespace mousefx {

int LinuxEventLoopService::Run() {
    return loop_.Run();
}

void LinuxEventLoopService::RequestExit() {
    loop_.RequestExit();
}

bool LinuxEventLoopService::PostTask(std::function<void()> task) {
    return loop_.PostTask(std::move(task));
}

} // namespace mousefx
