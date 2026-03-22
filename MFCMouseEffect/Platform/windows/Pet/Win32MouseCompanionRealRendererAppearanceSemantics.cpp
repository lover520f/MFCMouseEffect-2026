#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx::windows {
namespace {
bool HasAccessoryId(
    const std::vector<std::string>& accessoryIds,
    const char* needle) {
    return std::any_of(
        accessoryIds.begin(),
        accessoryIds.end(),
        [needle](const std::string& id) { return id.find(needle) != std::string::npos; });
}

} // namespace

Win32MouseCompanionRealRendererAppearanceAccessoryFamily
ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
    const std::vector<std::string>& accessoryIds) {
    if (HasAccessoryId(accessoryIds, "moon")) {
        return Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Moon;
    }
    if (HasAccessoryId(accessoryIds, "leaf")) {
        return Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Leaf;
    }
    if (HasAccessoryId(accessoryIds, "ribbon") || HasAccessoryId(accessoryIds, "bow")) {
        return Win32MouseCompanionRealRendererAppearanceAccessoryFamily::RibbonBow;
    }
    if (!accessoryIds.empty()) {
        return Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Star;
    }
    return Win32MouseCompanionRealRendererAppearanceAccessoryFamily::None;
}

Win32MouseCompanionRealRendererAppearanceComboPreset
ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
    const std::string& skinVariantId,
    Win32MouseCompanionRealRendererAppearanceAccessoryFamily family) {
    if (skinVariantId == "cream" && family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Moon) {
        return Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy;
    }
    if (skinVariantId == "night" && family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Leaf) {
        return Win32MouseCompanionRealRendererAppearanceComboPreset::Agile;
    }
    if (skinVariantId == "strawberry" && family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::RibbonBow) {
        return Win32MouseCompanionRealRendererAppearanceComboPreset::Charming;
    }
    return Win32MouseCompanionRealRendererAppearanceComboPreset::None;
}

const char* ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
    Win32MouseCompanionRealRendererAppearanceAccessoryFamily family) {
    switch (family) {
    case Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Star:
        return "star";
    case Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Moon:
        return "moon";
    case Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Leaf:
        return "leaf";
    case Win32MouseCompanionRealRendererAppearanceAccessoryFamily::RibbonBow:
        return "ribbon_bow";
    case Win32MouseCompanionRealRendererAppearanceAccessoryFamily::None:
        return "none";
    }
    return "none";
}

const char* ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
    Win32MouseCompanionRealRendererAppearanceComboPreset preset) {
    switch (preset) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        return "dreamy";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        return "agile";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        return "charming";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        return "none";
    }
    return "none";
}

bool TryParseWin32MouseCompanionRealRendererAppearanceComboPreset(
    const std::string& raw,
    Win32MouseCompanionRealRendererAppearanceComboPreset* outPreset) {
    if (!outPreset) {
        return false;
    }
    const std::string normalized = ToLowerAscii(TrimAscii(raw));
    if (normalized.empty() || normalized == "none") {
        *outPreset = Win32MouseCompanionRealRendererAppearanceComboPreset::None;
        return true;
    }
    if (normalized == "dreamy") {
        *outPreset = Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy;
        return true;
    }
    if (normalized == "agile") {
        *outPreset = Win32MouseCompanionRealRendererAppearanceComboPreset::Agile;
        return true;
    }
    if (normalized == "charming") {
        *outPreset = Win32MouseCompanionRealRendererAppearanceComboPreset::Charming;
        return true;
    }
    return false;
}

Win32MouseCompanionRealRendererAppearanceSemantics BuildWin32MouseCompanionRealRendererAppearanceSemantics(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style) {
    return BuildWin32MouseCompanionRenderPluginAppearanceSemantics(runtime, style);
}

} // namespace mousefx::windows
