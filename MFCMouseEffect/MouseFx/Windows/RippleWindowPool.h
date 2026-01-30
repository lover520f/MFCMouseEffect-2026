#pragma once

#include <memory>
#include <vector>

#include "RippleWindow.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"

namespace mousefx {

class RippleWindowPool final {
public:
    RippleWindowPool() = default;
    ~RippleWindowPool() = default;

    RippleWindowPool(const RippleWindowPool&) = delete;
    RippleWindowPool& operator=(const RippleWindowPool&) = delete;

    bool Initialize(size_t count);
    void Shutdown();

    void ShowRipple(const ClickEvent& ev); // Convenience for standard ripple
    void ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    
    RippleWindow* ShowContinuous(const ClickEvent& ev);
    RippleWindow* ShowContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    
    void BroadcastCommand(const std::string& cmd, const std::string& args);

private:
    std::vector<std::unique_ptr<RippleWindow>> windows_;
};

} // namespace mousefx
