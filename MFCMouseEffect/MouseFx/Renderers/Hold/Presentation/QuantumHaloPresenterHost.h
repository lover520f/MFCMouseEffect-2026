#pragma once

#include "IQuantumHaloPresenterBackend.h"
#include "QuantumHaloPresenterBackendRegistry.h"

#include <memory>
#include <string>
#include <vector>

namespace mousefx {

class QuantumHaloPresenterHost final {
public:
    bool Start();
    void Shutdown();
    bool IsReady() const;
    const std::string& LastErrorReason() const;
    const std::string& ActiveBackendName() const;

    bool RenderFrame(
        int cursorScreenX,
        int cursorScreenY,
        int sizePx,
        float t,
        uint64_t elapsedMs,
        uint32_t holdMs,
        const RippleStyle& style);

private:
    bool EnsureBackendStarted();
    void DropBackend();
    static std::string ComposeError(
        const std::string& prefix,
        const std::string& backendName,
        const std::string& detail);

    bool started_ = false;
    std::vector<QuantumHaloPresenterBackendRegistry::Descriptor> backends_{};
    size_t nextBackendIndex_ = 0;
    std::unique_ptr<IQuantumHaloPresenterBackend> backend_{};
    std::string activeBackendName_{};
    std::string lastErrorReason_{};
};

} // namespace mousefx
