#pragma once

#include "IQuantumHaloPresenterBackend.h"
#include "MouseFx/Renderers/Hold/QuantumHaloGpuV2Presenter.h"

#include <string>

namespace mousefx {

class QuantumHaloDCompPresenterBackend final : public IQuantumHaloPresenterBackend {
public:
    bool Start() override;
    void Shutdown() override;
    bool IsReady() const override;
    const std::string& LastErrorReason() const override;
    bool RenderFrame(const QuantumHaloPresenterFrameArgs& frame) override;

private:
    QuantumHaloGpuV2Presenter presenter_{};
    std::string lastErrorReason_{};
};

} // namespace mousefx
