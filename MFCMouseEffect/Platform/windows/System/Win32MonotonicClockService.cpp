#include "pch.h"

#include "Platform/windows/System/Win32MonotonicClockService.h"

#include <windows.h>

namespace mousefx {

uint64_t Win32MonotonicClockService::NowMs() const {
    return static_cast<uint64_t>(GetTickCount64());
}

} // namespace mousefx
