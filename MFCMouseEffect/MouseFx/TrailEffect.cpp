#include "pch.h"
#include "TrailEffect.h"
#include "ThemeStyle.h"
#include "TrailRenderStrategies.h"
#include <algorithm>
#include <cctype>
#include <string>

namespace mousefx {

TrailEffect::TrailEffect(const std::string& themeName, const std::string& type) : window_(std::make_unique<TrailWindow>()) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
    
    // Set appropriate renderer
    if (type == "electric") {
        window_->SetRenderer(std::make_unique<ElectricTrailRenderer>());
    } else if (type == "streamer") {
        window_->SetRenderer(std::make_unique<StreamerTrailRenderer>());
    } else {
        // Default Line
        window_->SetRenderer(std::make_unique<LineTrailRenderer>());
    }
}

TrailEffect::~TrailEffect() {
    Shutdown();
}

bool TrailEffect::Initialize() {
    if (!window_->Create()) return false;
    window_->SetChromatic(isChromatic_);
    return true;
}

// The Initialize method is removed as its logic has been moved to the constructor.
// bool TrailEffect::Initialize() {
//     window_ = std::make_unique<TrailWindow>();
//     if (!window_->Create()) return false;
//     window_->SetChromatic(isChromatic_);
//     return true;
// }

void TrailEffect::Shutdown() {
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void TrailEffect::OnMouseMove(const POINT& pt) {
    if (window_) {
        window_->AddPoint(pt);
    }
}

} // namespace mousefx
