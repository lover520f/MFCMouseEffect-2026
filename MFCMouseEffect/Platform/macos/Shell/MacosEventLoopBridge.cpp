#include "Platform/macos/Shell/MacosEventLoopBridge.h"

#include "Platform/macos/Shell/MacosEventLoopSwiftBridge.h"

namespace mousefx::platform::macos {

void EnsureMacosApplicationReady() {
#if defined(__APPLE__)
    mfx_macos_event_loop_ensure_application_ready_v1();
#endif
}

void RunMacosApplicationEventLoop() {
#if defined(__APPLE__)
    mfx_macos_event_loop_run_application_v1();
#endif
}

void StopMacosApplicationEventLoop() {
#if defined(__APPLE__)
    mfx_macos_event_loop_stop_application_v1();
#endif
}

} // namespace mousefx::platform::macos
