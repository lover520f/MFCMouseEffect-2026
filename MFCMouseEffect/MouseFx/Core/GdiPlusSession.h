#pragma once

#include <windows.h>
#include <gdiplus.h>

// Small RAII wrapper for GDI+ initialization.
namespace mousefx {

class GdiPlusSession final {
public:
    GdiPlusSession() = default;
    ~GdiPlusSession() { Shutdown(); }

    GdiPlusSession(const GdiPlusSession&) = delete;
    GdiPlusSession& operator=(const GdiPlusSession&) = delete;

    bool Startup() {
        if (started_) return true;
        Gdiplus::GdiplusStartupInput input;
        if (Gdiplus::GdiplusStartup(&token_, &input, nullptr) != Gdiplus::Ok) {
            token_ = 0;
            return false;
        }
        started_ = true;
        return true;
    }

    void Shutdown() {
        if (!started_) return;
        Gdiplus::GdiplusShutdown(token_);
        token_ = 0;
        started_ = false;
    }

private:
    ULONG_PTR token_ = 0;
    bool started_ = false;
};

} // namespace mousefx

