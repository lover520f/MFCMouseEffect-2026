#pragma once

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile;
struct Win32MouseCompanionRealRendererScene;

void ApplyWin32MouseCompanionRealRendererModelScenePoseProjector(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
