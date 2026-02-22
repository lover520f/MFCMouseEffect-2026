#include "pch.h"
#include "TrailEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Layers/TrailOverlayLayer.h"
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
    durationMs_ = durationMs;
    maxPoints_ = maxPoints;
    params_ = params;
}

TrailEffect::~TrailEffect() {
    Shutdown();
}

bool TrailEffect::Initialize() {
    hostLayer_ = OverlayHostService::Instance().AttachTrailLayer(
        CreateRenderer(), durationMs_, maxPoints_, isChromatic_);
    if (hostLayer_) return true;

    if (!window_->Create()) return false;
    window_->SetDurationMs(durationMs_);
    window_->SetMaxPoints(maxPoints_);
    window_->SetRenderer(CreateRenderer());
    window_->SetChromatic(isChromatic_);
    return true;
}

void TrailEffect::Shutdown() {
    if (hostLayer_) {
        OverlayHostService::Instance().DetachLayer(hostLayer_);
        hostLayer_ = nullptr;
    }
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void TrailEffect::OnMouseMove(const ScreenPoint& pt) {
    if (hostLayer_) {
        hostLayer_->AddPoint(pt);
        return;
    }
    if (window_) {
        window_->AddPoint(pt);
    }
}

std::unique_ptr<ITrailRenderer> TrailEffect::CreateRenderer() const {
    if (type_ == "electric") {
        return std::make_unique<ElectricTrailRenderer>(durationMs_, params_);
    }
    if (type_ == "streamer") {
        return std::make_unique<StreamerTrailRenderer>(durationMs_, params_);
    }
    if (type_ == "tubes" || type_ == "scifi") {
        return std::make_unique<TubesRenderer>();
    }
    if (type_ == "meteor") {
        return std::make_unique<MeteorRenderer>(durationMs_, params_);
    }
    return std::make_unique<LineTrailRenderer>(durationMs_, params_);
}

} // namespace mousefx
