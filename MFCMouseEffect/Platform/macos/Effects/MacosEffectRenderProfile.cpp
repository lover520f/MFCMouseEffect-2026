#include "pch.h"

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.Shared.h"

namespace mousefx::macos_effect_profile {
namespace {

const TestProfileTuning& ResolveCachedTestProfileTuning() {
    static const TestProfileTuning kCached = detail::ResolveTestProfileTuningFromEnv();
    return kCached;
}

} // namespace

TestProfileTuning ResolveTestProfileTuning() {
    return ResolveCachedTestProfileTuning();
}

} // namespace mousefx::macos_effect_profile
