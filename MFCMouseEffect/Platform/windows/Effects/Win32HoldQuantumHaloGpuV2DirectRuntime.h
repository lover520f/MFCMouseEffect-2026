#pragma once

#include "MouseFx/Interfaces/IHoldRuntime.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

namespace mousefx {

class Win32HoldQuantumHaloGpuV2DirectRuntime final : public IHoldRuntime {
public:
    Win32HoldQuantumHaloGpuV2DirectRuntime() = default;
    ~Win32HoldQuantumHaloGpuV2DirectRuntime();

    Win32HoldQuantumHaloGpuV2DirectRuntime(const Win32HoldQuantumHaloGpuV2DirectRuntime&) = delete;
    Win32HoldQuantumHaloGpuV2DirectRuntime& operator=(const Win32HoldQuantumHaloGpuV2DirectRuntime&) = delete;

    bool Start(const RippleStyle& style, const ScreenPoint& startPt) override;
    void Update(uint32_t holdMs, const ScreenPoint& pt) override;
    void Stop() override;
    bool IsRunning() const override;

private:
    void WorkerMain();

    RippleStyle style_{};
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    std::atomic<uint32_t> holdMs_{0u};
    std::atomic<int> cursorX_{0};
    std::atomic<int> cursorY_{0};
    std::atomic<uint64_t> submitCount_{0ull};
    std::atomic<uint64_t> missCount_{0ull};
    void* stopEvent_ = nullptr;
    std::thread worker_{};
};

} // namespace mousefx
