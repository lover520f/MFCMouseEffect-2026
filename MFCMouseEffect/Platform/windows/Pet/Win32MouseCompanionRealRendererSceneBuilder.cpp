#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererActionOverlayBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAdornmentBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppendageBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererFaceBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererFrameBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPaletteBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPaletteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererMotionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererStyleProfile.h"

namespace mousefx::windows {

Win32MouseCompanionRealRendererScene BuildWin32MouseCompanionRealRendererScene(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    int width,
    int height) {
    Win32MouseCompanionRealRendererScene scene{};
    if (width <= 0 || height <= 0 || !runtime.assets) {
        return scene;
    }

    const auto style = BuildWin32MouseCompanionRealRendererStyleProfile();
    const auto profile = BuildWin32MouseCompanionRealRendererMotionProfile(runtime, style);
    const auto palette = BuildWin32MouseCompanionRealRendererPaletteProfile(runtime, style);
    BuildWin32MouseCompanionRealRendererPalette(runtime, profile, style, palette, scene);
    const auto metrics = BuildWin32MouseCompanionRealRendererFrame(runtime, profile, style, width, height, scene);
    BuildWin32MouseCompanionRealRendererAppendages(runtime, profile, style, metrics, scene);
    BuildWin32MouseCompanionRealRendererFace(runtime, profile, style, scene);
    BuildWin32MouseCompanionRealRendererAdornment(runtime, style, metrics, scene);
    BuildWin32MouseCompanionRealRendererActionOverlay(runtime, profile, style, metrics, scene);

    return scene;
}

} // namespace mousefx::windows
