#include "pch.h"

#include "MouseFx/Core/System/GdiPlusSession.h"

#include <gdiplus.h>

namespace mousefx {

bool GdiPlusSession::Startup() {
    if (started_) {
        return true;
    }

    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR nativeToken = 0;
    if (Gdiplus::GdiplusStartup(&nativeToken, &input, nullptr) != Gdiplus::Ok) {
        token_ = 0;
        return false;
    }

    token_ = static_cast<std::uintptr_t>(nativeToken);
    started_ = true;
    return true;
}

void GdiPlusSession::Shutdown() {
    if (!started_) {
        return;
    }

    Gdiplus::GdiplusShutdown(static_cast<ULONG_PTR>(token_));
    token_ = 0;
    started_ = false;
}

} // namespace mousefx

