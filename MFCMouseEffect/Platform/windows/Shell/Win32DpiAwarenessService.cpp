#include "pch.h"

#include "Platform/windows/Shell/Win32DpiAwarenessService.h"

namespace mousefx {

void Win32DpiAwarenessService::EnableForScreenCoords() {
#ifndef DPI_AWARENESS_CONTEXT
    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

    enum PROCESS_DPI_AWARENESS_LOCAL {
        ProcessDpiUnaware = 0,
        ProcessSystemDpiAware = 1,
        ProcessPerMonitorDpiAware = 2
    };

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
        auto* setContext = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
            GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (setContext && setContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            using SetThreadDpiAwarenessContextFn = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);
            auto* setThreadContext = reinterpret_cast<SetThreadDpiAwarenessContextFn>(
                GetProcAddress(user32, "SetThreadDpiAwarenessContext"));
            if (setThreadContext) {
                setThreadContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
            return;
        }
    }

    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (shcore) {
        using SetProcessDpiAwarenessFn = HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS_LOCAL);
        auto* setAwareness = reinterpret_cast<SetProcessDpiAwarenessFn>(
            GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (setAwareness) {
            const HRESULT hr = setAwareness(ProcessPerMonitorDpiAware);
            FreeLibrary(shcore);
            if (SUCCEEDED(hr)) {
                return;
            }
        } else {
            FreeLibrary(shcore);
        }
    }

    if (user32) {
        using SetProcessDPIAwareFn = BOOL(WINAPI*)();
        auto* setAware = reinterpret_cast<SetProcessDPIAwareFn>(GetProcAddress(user32, "SetProcessDPIAware"));
        if (setAware) {
            setAware();
        }
    }
}

} // namespace mousefx
