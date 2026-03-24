#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageStrokeProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

float Blend(float current, float target, float mix) {
    return current + (target - current) * mix;
}

float ResolveActionBias(const Win32MouseCompanionRealRendererScene& scene) {
    float bias = 0.0f;
    if (scene.actionOverlay.holdBandVisible) {
        bias += 0.24f;
    }
    if (scene.actionOverlay.dragLineVisible) {
        bias += 0.28f;
    }
    if (scene.actionOverlay.followTrailVisible) {
        bias += 0.20f;
    }
    if (scene.actionOverlay.scrollArcVisible) {
        bias += 0.08f;
    }
    return std::clamp(bias, 0.0f, 0.64f);
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyAppendageStrokeProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.previewTailStrokeScale = 1.0f;
    scene.previewHandStrokeScale = 1.0f;
    scene.previewLegStrokeScale = 1.0f;

    if (!scene.modelProxyAppendageLayer.visible) {
        return;
    }

    const float dominance = std::clamp(scene.proxyDominance, 0.0f, 1.0f);
    const float actionBias = ResolveActionBias(scene);
    const float tailMix = std::clamp(dominance * 0.84f + actionBias * 0.54f, 0.0f, 1.0f);
    const float handMix = std::clamp(dominance * 0.78f + actionBias * 0.58f, 0.0f, 1.0f);
    const float legMix = std::clamp(dominance * 0.80f + actionBias * 0.52f, 0.0f, 1.0f);

    scene.previewTailStrokeScale = Blend(1.0f, 0.54f, tailMix);
    scene.previewHandStrokeScale = Blend(1.0f, 0.60f, handMix);
    scene.previewLegStrokeScale = Blend(1.0f, 0.64f, legMix);
}

} // namespace mousefx::windows
