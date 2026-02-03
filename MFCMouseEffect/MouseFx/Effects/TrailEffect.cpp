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

namespace {

struct TrailWindowProfile {
    int durationMs = 300;
    int maxPoints = 32;
};

TrailWindowProfile GetTrailWindowProfile(const std::string& type) {
    // Key point: renderer visual identity depends on having enough history.
    // TrailWindow must keep points long enough for the selected renderer.
    if (type == "electric")  return {280, 24};
    if (type == "streamer")  return {420, 46};
    if (type == "meteor")    return {520, 60};
    if (type == "tubes" || type == "scifi") return {350, 40};
    return {300, 32};
}

} // namespace

TrailEffect::TrailEffect(const std::string& themeName, const std::string& type) : window_(std::make_unique<TrailWindow>()), type_(type) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
    const auto profile = GetTrailWindowProfile(type_);
    window_->SetDurationMs(profile.durationMs);
    window_->SetMaxPoints(profile.maxPoints);
    
    // Set appropriate renderer
    if (type == "electric") {
        window_->SetRenderer(std::make_unique<ElectricTrailRenderer>(profile.durationMs));
    } else if (type == "streamer") {
        window_->SetRenderer(std::make_unique<StreamerTrailRenderer>(profile.durationMs));
    } else if (type == "tubes" || type == "scifi") {
        window_->SetRenderer(std::make_unique<TubesRenderer>());
    } else if (type == "meteor") {
        window_->SetRenderer(std::make_unique<MeteorRenderer>(profile.durationMs));
    } else {
        // Default Line
        window_->SetRenderer(std::make_unique<LineTrailRenderer>(profile.durationMs));
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
