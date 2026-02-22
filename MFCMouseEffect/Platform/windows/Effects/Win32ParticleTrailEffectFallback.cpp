#include "pch.h"

#include "Platform/windows/Effects/Win32ParticleTrailEffectFallback.h"

namespace mousefx {

bool Win32ParticleTrailEffectFallback::Create() {
    if (created_) {
        return true;
    }
    created_ = window_.Create();
    return created_;
}

void Win32ParticleTrailEffectFallback::Shutdown() {
    window_.Shutdown();
    created_ = false;
}

void Win32ParticleTrailEffectFallback::SetChromatic(bool chromatic) {
    window_.SetChromatic(chromatic);
}

void Win32ParticleTrailEffectFallback::UpdateCursor(const ScreenPoint& pt) {
    if (!created_) {
        return;
    }
    window_.UpdateCursor(pt);
}

} // namespace mousefx
