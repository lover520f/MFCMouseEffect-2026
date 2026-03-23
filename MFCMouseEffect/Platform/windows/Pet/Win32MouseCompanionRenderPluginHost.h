#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginManifestContract.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"

namespace mousefx::windows {

struct Win32MouseCompanionRenderPluginSelection final {
    const class IWin32MouseCompanionRenderPlugin* plugin{nullptr};
    std::string pluginId;
    std::string pluginKind;
    std::string pluginSource;
    std::string selectionReason;
    std::string failureReason;
    std::string manifestPath;
    std::string runtimeBackend;
    std::string metadataPath;
    uint32_t metadataSchemaVersion{0};
    std::string appearanceSemanticsMode{"legacy_manifest_compat"};
    std::string defaultLaneCandidate{"builtin"};
    std::string defaultLaneSource{"runtime_builtin_default"};
    std::string defaultLaneRolloutStatus{"stay_on_builtin"};
    std::string defaultLaneStyleIntent{"style_candidate:none"};
    std::string declaredStyleIntent;
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride{
        Win32MouseCompanionRealRendererAppearanceComboPreset::None};
    Win32MouseCompanionRendererPluginAppearanceSemanticsTuning tuning{};
};

class IWin32MouseCompanionRenderPlugin {
public:
    virtual ~IWin32MouseCompanionRenderPlugin() = default;

    virtual const char* PluginId() const = 0;
    virtual const char* PluginKind() const = 0;
    virtual const char* PluginSource() const = 0;
    virtual Win32MouseCompanionRealRendererAppearanceSemantics BuildAppearanceSemantics(
        const Win32MouseCompanionRealRendererSceneRuntime& runtime,
        const Win32MouseCompanionRealRendererStyleProfile& style) const = 0;
};

const IWin32MouseCompanionRenderPlugin&
GetDefaultWin32MouseCompanionRenderPlugin();

Win32MouseCompanionRenderPluginSelection
ResolveWin32MouseCompanionRenderPluginSelection();

Win32MouseCompanionRealRendererAppearanceSemantics
BuildWin32MouseCompanionRenderPluginAppearanceSemantics(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style);

const char* ReadDefaultWin32MouseCompanionRenderPluginId();
const char* ReadDefaultWin32MouseCompanionRenderPluginKind();
const char* ReadDefaultWin32MouseCompanionRenderPluginSource();

} // namespace mousefx::windows
