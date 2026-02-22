#include "pch.h"

#include "Platform/windows/Effects/Win32TrailEffectFallback.h"

namespace mousefx {

bool Win32TrailEffectFallback::Create() {
    if (created_) {
        return true;
    }
    created_ = window_.Create();
    return created_;
}

void Win32TrailEffectFallback::Shutdown() {
    window_.Shutdown();
    created_ = false;
}

void Win32TrailEffectFallback::Configure(bool isChromatic, int durationMs, int maxPoints, std::unique_ptr<ITrailRenderer> renderer) {
    window_.SetDurationMs(durationMs);
    window_.SetMaxPoints(maxPoints);
    window_.SetRenderer(std::move(renderer));
    window_.SetChromatic(isChromatic);
}

void Win32TrailEffectFallback::AddPoint(const ScreenPoint& pt) {
    if (!created_) {
        return;
    }
    window_.AddPoint(pt);
}

} // namespace mousefx
