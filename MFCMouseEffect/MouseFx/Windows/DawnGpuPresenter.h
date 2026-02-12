#pragma once

#include <atomic>
#include <mutex>
#include <string>

#include "MouseFx/Interfaces/IOverlayPresenter.h"
#include "MouseFx/Gpu/DawnSurfaceInterop.h"

namespace mousefx {

class DawnGpuPresenter final : public IOverlayPresenter {
public:
    DawnGpuPresenter() = default;
    ~DawnGpuPresenter() override;

    bool Present(const OverlayPresentFrame& frame) override;
    std::string LastDetail() const;

private:
    std::atomic<uint64_t> attempts_{0};
    mutable std::mutex detailMutex_{};
    std::mutex surfaceMutex_{};
    gpu::DawnSurfaceInteropState surfaceState_{};
    std::string lastDetail_ = "not_called";
};

} // namespace mousefx
