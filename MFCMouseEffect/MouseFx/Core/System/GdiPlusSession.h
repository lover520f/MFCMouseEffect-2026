#pragma once

#include <cstdint>

// Small RAII wrapper for GDI+ initialization.
namespace mousefx {

class GdiPlusSession final {
public:
    GdiPlusSession() = default;
    ~GdiPlusSession() { Shutdown(); }

    GdiPlusSession(const GdiPlusSession&) = delete;
    GdiPlusSession& operator=(const GdiPlusSession&) = delete;

    bool Startup();

    void Shutdown();

private:
    std::uintptr_t token_ = 0;
    bool started_ = false;
};

} // namespace mousefx

