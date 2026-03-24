#pragma once

#include <gdiplus.h>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererScene;

void TranslateWin32MouseCompanionRealRendererBodyCluster(
    Win32MouseCompanionRealRendererScene& scene,
    float dx,
    float dy);
void TranslateWin32MouseCompanionRealRendererHeadCluster(
    Win32MouseCompanionRealRendererScene& scene,
    float dx,
    float dy);
void TranslateWin32MouseCompanionRealRendererAppendageCluster(
    Win32MouseCompanionRealRendererScene& scene,
    float dx,
    float dy);
void TranslateWin32MouseCompanionRealRendererOverlayCluster(
    Win32MouseCompanionRealRendererScene& scene,
    float dx,
    float dy);
void TranslateWin32MouseCompanionRealRendererGroundingCluster(
    Win32MouseCompanionRealRendererScene& scene,
    float dx,
    float dy);
void UpdateWin32MouseCompanionRealRendererGraphLinkStart(
    const char* logicalNode,
    const Gdiplus::PointF& anchor,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
