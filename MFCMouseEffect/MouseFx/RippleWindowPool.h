#pragma once

#include <memory>
#include <vector>

#include "RippleWindow.h"

namespace mousefx {

class RippleWindowPool final {
public:
    RippleWindowPool() = default;
    ~RippleWindowPool() = default;

    RippleWindowPool(const RippleWindowPool&) = delete;
    RippleWindowPool& operator=(const RippleWindowPool&) = delete;

    bool Initialize(size_t count);
    void Shutdown();

    void ShowRipple(const ClickEvent& ev);
    // Returns active window handle for tracking (to stop it later)
    RippleWindow* ShowContinuous(const ClickEvent& ev);
    void SetDrawMode(RippleWindow::DrawMode mode);

private:
    std::vector<std::unique_ptr<RippleWindow>> windows_;
};

} // namespace mousefx

