#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererMotionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererStyleProfile.h"

namespace mousefx::windows {

void BuildWin32MouseCompanionRealRendererFace(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
