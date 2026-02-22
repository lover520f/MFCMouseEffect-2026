#include "Platform/macos/Shell/MacosEventLoopService.h"

#include <utility>

namespace mousefx {

int MacosEventLoopService::Run() {
    return loop_.Run();
}

void MacosEventLoopService::RequestExit() {
    loop_.RequestExit();
}

bool MacosEventLoopService::PostTask(std::function<void()> task) {
    return loop_.PostTask(std::move(task));
}

} // namespace mousefx
