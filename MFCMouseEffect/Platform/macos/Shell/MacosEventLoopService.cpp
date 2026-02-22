#include "Platform/macos/Shell/MacosEventLoopService.h"

#include <chrono>
#include <thread>

namespace mousefx {

int MacosEventLoopService::Run() {
    using namespace std::chrono_literals;
    exitRequested_.store(false, std::memory_order_release);
    while (!exitRequested_.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(10ms);
    }
    return 0;
}

void MacosEventLoopService::RequestExit() {
    exitRequested_.store(true, std::memory_order_release);
}

} // namespace mousefx
