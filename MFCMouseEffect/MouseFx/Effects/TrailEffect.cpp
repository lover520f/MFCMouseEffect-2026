#include "pch.h"
#include "TrailEffect.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Interfaces/TrailRenderStrategies.h"
#include "MouseFx/Renderers/Trail/TubesRenderer.h"
#include "MouseFx/Renderers/Trail/MeteorRenderer.h"
#include <algorithm>
#include <cctype>
#include <string>

namespace mousefx {

TrailEffect::TrailEffect(const std::string& themeName, const std::string& type, int durationMs, int maxPoints, const TrailRendererParamsConfig& params)
    : window_(std::make_unique<TrailWindow>()), type_(type) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
    window_->SetDurationMs(durationMs);
    window_->SetMaxPoints(maxPoints);
    
    // Set appropriate renderer
    if (type == "electric") {
        window_->SetRenderer(std::make_unique<ElectricTrailRenderer>(durationMs, params));
    } else if (type == "streamer") {
        window_->SetRenderer(std::make_unique<StreamerTrailRenderer>(durationMs, params));
    } else if (type == "tubes" || type == "scifi") {
        window_->SetRenderer(std::make_unique<TubesRenderer>());
    } else if (type == "meteor") {
        window_->SetRenderer(std::make_unique<MeteorRenderer>(durationMs, params));
    } else {
        // Default Line
        window_->SetRenderer(std::make_unique<LineTrailRenderer>(durationMs, params));
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
