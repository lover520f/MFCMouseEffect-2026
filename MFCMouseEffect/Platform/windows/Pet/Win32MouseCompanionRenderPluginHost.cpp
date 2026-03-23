#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginManifestContract.h"

#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

#include <cstdlib>
#include <mutex>

namespace mousefx::windows {
namespace {

constexpr const char* kPluginModeEnvVar =
    "MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN";
constexpr const char* kPluginManifestEnvVar =
    "MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST";
constexpr const char* kPluginModeAuto = "auto";
constexpr const char* kPluginModeBuiltin = "builtin";
constexpr const char* kPluginModeWasm = "wasm";

void ApplyDefaultLaneDecision(
    Win32MouseCompanionRenderPluginSelection* selection) {
    if (!selection) {
        return;
    }
    selection->defaultLaneCandidate = "builtin";
    selection->defaultLaneSource = "runtime_builtin_default";
    selection->defaultLaneRolloutStatus = "stay_on_builtin";

    const std::string selectionReason = TrimAscii(selection->selectionReason);
    if (selectionReason == "env:explicit_builtin") {
        selection->defaultLaneSource = "env_builtin_forced";
    } else if (
        selectionReason == "env:wasm_requested_fallback_builtin" ||
        selectionReason == "env:invalid_plugin_mode_fallback_builtin") {
        selection->defaultLaneSource = "env_wasm_fallback_builtin";
    }

    const std::string pluginKind = TrimAscii(selection->pluginKind);
    const std::string semanticsMode = TrimAscii(selection->appearanceSemanticsMode);
    const bool pluginReady =
        !pluginKind.empty() &&
        pluginKind != "native_builtin" &&
        TrimAscii(selection->failureReason).empty();
    const bool semanticsReady =
        !semanticsMode.empty() && semanticsMode != "legacy_manifest_compat";
    if (!pluginReady || !semanticsReady) {
        return;
    }

    if (semanticsMode == "wasm_v1" || semanticsMode == "builtin_passthrough") {
        selection->defaultLaneCandidate = semanticsMode;
        selection->defaultLaneSource =
            selectionReason == "env:wasm_requested"
                ? "env_wasm_candidate"
                : "runtime_plugin_candidate";
        selection->defaultLaneRolloutStatus =
            "candidate_pending_manual_confirmation";
    }

    selection->defaultLaneStyleIntent =
        ResolveWin32MouseCompanionRenderPluginDefaultLaneStyleIntent(
            selection->defaultLaneCandidate,
            selection->declaredStyleIntent,
            selection->comboPresetOverride);
}

std::string ReadEnvCopy(const char* key) {
    if (!key || !*key) {
        return {};
    }
    char* raw = nullptr;
    size_t rawSize = 0;
    const errno_t result = _dupenv_s(&raw, &rawSize, key);
    if (result != 0 || !raw || rawSize == 0) {
        if (raw) {
            free(raw);
        }
        return {};
    }
    std::string value(raw);
    free(raw);
    return value;
}

std::string NormalizePluginMode(const std::string& raw) {
    const std::string normalized = ToLowerAscii(TrimAscii(raw));
    if (normalized.empty() || normalized == "default") {
        return kPluginModeAuto;
    }
    return normalized;
}

Gdiplus::Color MakeColor(BYTE a, BYTE r, BYTE g, BYTE b) {
    return Gdiplus::Color(a, r, g, b);
}

void ApplyComboPreset(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics) {
    switch (semantics.comboPreset) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        semantics.frame.headHeightScale *= 1.02f;
        semantics.face.highlightAlphaScale *= 1.10f;
        semantics.face.mouthReactiveScale *= 0.96f;
        semantics.appendage.earScale *= 0.96f;
        semantics.appendage.followTailWidthScale *= 1.04f;
        semantics.motion.followStateLiftScale *= 1.06f;
        semantics.motion.followHeadNodScale *= 1.04f;
        semantics.mood.glowTintMixScale *= 1.08f;
        semantics.mood.shadowAlphaBias -= 3.0f;
        semantics.mood.pedestalAlphaBias -= 2.0f;
        semantics.mood.followTrailAlphaScale *= 1.08f;
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        semantics.frame.bodyHeightScale *= 1.02f;
        semantics.face.pupilFocusScale *= 1.06f;
        semantics.face.whiskerSpreadScale *= 1.06f;
        semantics.appendage.dragHandReachScale *= 1.10f;
        semantics.appendage.followEarSpreadScale *= 1.04f;
        semantics.motion.dragLeanScale *= 1.08f;
        semantics.motion.followTailSwingScale *= 1.06f;
        semantics.mood.shadowTintMixScale *= 1.08f;
        semantics.mood.pedestalTintMixScale *= 1.06f;
        semantics.mood.dragLineAlphaScale *= 1.10f;
        semantics.mood.scrollArcAlphaScale *= 1.06f;
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        semantics.frame.headWidthScale *= 1.03f;
        semantics.face.blushWidthScale *= 1.08f;
        semantics.face.cheekWidthScale *= 1.05f;
        semantics.face.highlightAlphaScale *= 1.04f;
        semantics.appendage.clickEarLiftScale *= 1.08f;
        semantics.motion.clickSquashScale *= 1.08f;
        semantics.motion.holdHeadNodScale *= 0.95f;
        semantics.mood.accentTintMixScale *= 1.10f;
        semantics.mood.clickRingAlphaScale *= 1.12f;
        semantics.mood.holdBandAlphaScale *= 1.08f;
        semantics.mood.glowTintMixScale *= 1.04f;
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        break;
    }
}

void ApplyAppearanceSemanticsTuning(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsTuning& tuning) {
    semantics.motion.followStateLiftScale *= tuning.followLiftScale;
    semantics.motion.clickSquashScale *= tuning.clickSquashScale;
    semantics.motion.dragLeanScale *= tuning.dragLeanScale;
    semantics.motion.followTailSwingScale *= tuning.followTailSwingScale;
    semantics.motion.holdHeadNodScale *= tuning.holdHeadNodScale;
    semantics.motion.scrollTailLiftScale *= tuning.scrollTailLiftScale;
    semantics.motion.followHeadNodScale *= tuning.followHeadNodScale;
    semantics.face.highlightAlphaScale *= tuning.highlightAlphaScale;
}

void ApplyThemeAppearanceSemanticsPatch(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch& patch) {
    if (patch.hasThemeGlowColor) {
        semantics.theme.glowColor = patch.themeGlowColor;
    }
    if (patch.hasThemeBodyStroke) {
        semantics.theme.bodyStroke = patch.themeBodyStroke;
    }
    if (patch.hasThemeHeadFill) {
        semantics.theme.headFill = patch.themeHeadFill;
    }
    if (patch.hasThemeAccentFill) {
        semantics.theme.accentFill = patch.themeAccentFill;
    }
    if (patch.hasThemeAccessoryFill) {
        semantics.theme.accessoryFill = patch.themeAccessoryFill;
    }
    if (patch.hasThemePedestalFill) {
        semantics.theme.pedestalFill = patch.themePedestalFill;
    }
}

void ApplyShapeAppearanceSemanticsPatch(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch& patch) {
    if (patch.hasFrameBodyWidthScale) {
        semantics.frame.bodyWidthScale = patch.frameBodyWidthScale;
    }
    if (patch.hasFrameBodyHeightScale) {
        semantics.frame.bodyHeightScale = patch.frameBodyHeightScale;
    }
    if (patch.hasFrameHeadWidthScale) {
        semantics.frame.headWidthScale = patch.frameHeadWidthScale;
    }
    if (patch.hasFrameHeadHeightScale) {
        semantics.frame.headHeightScale = patch.frameHeadHeightScale;
    }
    if (patch.hasFaceBlushWidthScale) {
        semantics.face.blushWidthScale = patch.faceBlushWidthScale;
    }
    if (patch.hasFaceMuzzleWidthScale) {
        semantics.face.muzzleWidthScale = patch.faceMuzzleWidthScale;
    }
    if (patch.hasFaceForeheadWidthScale) {
        semantics.face.foreheadWidthScale = patch.faceForeheadWidthScale;
    }
    if (patch.hasFacePupilFocusScale) {
        semantics.face.pupilFocusScale = patch.facePupilFocusScale;
    }
    if (patch.hasFaceHighlightAlphaScale) {
        semantics.face.highlightAlphaScale = patch.faceHighlightAlphaScale;
    }
    if (patch.hasFaceWhiskerSpreadScale) {
        semantics.face.whiskerSpreadScale = patch.faceWhiskerSpreadScale;
    }
    if (patch.hasAppendageEarScale) {
        semantics.appendage.earScale = patch.appendageEarScale;
    }
    if (patch.hasAppendageTailWidthScale) {
        semantics.appendage.tailWidthScale = patch.appendageTailWidthScale;
    }
    if (patch.hasAppendageTailHeightScale) {
        semantics.appendage.tailHeightScale = patch.appendageTailHeightScale;
    }
    if (patch.hasAppendageFollowTailWidthScale) {
        semantics.appendage.followTailWidthScale = patch.appendageFollowTailWidthScale;
    }
    if (patch.hasAppendageFollowEarSpreadScale) {
        semantics.appendage.followEarSpreadScale = patch.appendageFollowEarSpreadScale;
    }
    if (patch.hasAppendageClickEarLiftScale) {
        semantics.appendage.clickEarLiftScale = patch.appendageClickEarLiftScale;
    }
}

void ApplyMotionAppearanceSemanticsPatch(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch& patch) {
    if (patch.hasMotionFollowStateLiftScale) {
        semantics.motion.followStateLiftScale = patch.motionFollowStateLiftScale;
    }
    if (patch.hasMotionClickSquashScale) {
        semantics.motion.clickSquashScale = patch.motionClickSquashScale;
    }
    if (patch.hasMotionDragLeanScale) {
        semantics.motion.dragLeanScale = patch.motionDragLeanScale;
    }
    if (patch.hasMotionHoldHeadNodScale) {
        semantics.motion.holdHeadNodScale = patch.motionHoldHeadNodScale;
    }
    if (patch.hasMotionScrollTailLiftScale) {
        semantics.motion.scrollTailLiftScale = patch.motionScrollTailLiftScale;
    }
    if (patch.hasMotionFollowHeadNodScale) {
        semantics.motion.followHeadNodScale = patch.motionFollowHeadNodScale;
    }
}

void ApplyMoodAppearanceSemanticsPatch(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch& patch) {
    if (patch.hasMoodGlowTintMixScale) {
        semantics.mood.glowTintMixScale = patch.moodGlowTintMixScale;
    }
    if (patch.hasMoodAccentTintMixScale) {
        semantics.mood.accentTintMixScale = patch.moodAccentTintMixScale;
    }
    if (patch.hasMoodShadowTintMixScale) {
        semantics.mood.shadowTintMixScale = patch.moodShadowTintMixScale;
    }
    if (patch.hasMoodShadowAlphaBias) {
        semantics.mood.shadowAlphaBias = patch.moodShadowAlphaBias;
    }
    if (patch.hasMoodPedestalAlphaBias) {
        semantics.mood.pedestalAlphaBias = patch.moodPedestalAlphaBias;
    }
    if (patch.hasMoodScrollArcAlphaScale) {
        semantics.mood.scrollArcAlphaScale = patch.moodScrollArcAlphaScale;
    }
    if (patch.hasMoodHoldBandAlphaScale) {
        semantics.mood.holdBandAlphaScale = patch.moodHoldBandAlphaScale;
    }
    if (patch.hasMoodDragLineAlphaScale) {
        semantics.mood.dragLineAlphaScale = patch.moodDragLineAlphaScale;
    }
    if (patch.hasMoodFollowTrailAlphaScale) {
        semantics.mood.followTrailAlphaScale = patch.moodFollowTrailAlphaScale;
    }
}

void ApplyAppearanceSemanticsPatch(
    Win32MouseCompanionRealRendererAppearanceSemantics& semantics,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch& patch) {
    ApplyThemeAppearanceSemanticsPatch(semantics, patch);
    ApplyShapeAppearanceSemanticsPatch(semantics, patch);
    ApplyMotionAppearanceSemanticsPatch(semantics, patch);
    ApplyMoodAppearanceSemanticsPatch(semantics, patch);
}

Win32MouseCompanionRealRendererAppearanceSemantics BuildBuiltinAppearanceSemantics(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsTuning& tuning,
    const Win32MouseCompanionRendererPluginAppearanceSemanticsPatch* patch) {
    Win32MouseCompanionRealRendererAppearanceSemantics semantics{};

    semantics.theme.glowColor = MakeColor(255, 82, 170, 255);
    semantics.theme.baseBodyFill = MakeColor(255, 239, 244, 255);
    semantics.theme.bodyStroke = MakeColor(255, 70, 98, 152);
    semantics.theme.headFill = MakeColor(255, 255, 250, 246);
    semantics.theme.headFillRear = MakeColor(255, 241, 234, 232);
    semantics.theme.earFill = MakeColor(255, 255, 247, 235);
    semantics.theme.earFillRear = MakeColor(255, 235, 226, 214);
    semantics.theme.earInner = MakeColor(255, 255, 201, 214);
    semantics.theme.earInnerRear = MakeColor(255, 235, 178, 194);
    semantics.theme.blushRgb = MakeColor(255, 255, 171, 194);
    semantics.theme.accentFill = MakeColor(255, 111, 219, 255);
    semantics.theme.pedestalFill = MakeColor(105, 59, 77, 115);
    semantics.theme.accessoryFill = MakeColor(255, 255, 221, 97);
    semantics.theme.accessoryStroke = MakeColor(255, 144, 98, 38);

    if (!runtime.assets) {
        if (comboPresetOverride !=
            Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
            semantics.comboPreset = comboPresetOverride;
        }
        ApplyComboPreset(semantics);
        ApplyAppearanceSemanticsTuning(semantics, tuning);
        if (patch) {
            ApplyAppearanceSemanticsPatch(semantics, *patch);
        }
        return semantics;
    }

    const std::string& skinVariantId = runtime.assets->appearanceProfileSkinVariantId;
    semantics.adornment.family =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            runtime.assets->appearanceAccessoryIds);
    semantics.comboPreset =
        ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
            skinVariantId,
            semantics.adornment.family);
    if (skinVariantId == "cream") {
        semantics.theme.glowColor = MakeColor(255, 255, 196, 126);
        semantics.theme.baseBodyFill = MakeColor(255, 255, 243, 212);
        semantics.theme.bodyStroke = MakeColor(255, 136, 112, 84);
        semantics.theme.headFill = MakeColor(255, 255, 248, 232);
        semantics.theme.headFillRear = MakeColor(255, 240, 224, 203);
        semantics.theme.earFill = MakeColor(255, 255, 239, 214);
        semantics.theme.earFillRear = MakeColor(255, 235, 219, 190);
        semantics.theme.earInner = MakeColor(255, 247, 194, 176);
        semantics.theme.earInnerRear = MakeColor(255, 224, 168, 150);
        semantics.theme.blushRgb = MakeColor(255, 243, 176, 152);
        semantics.theme.accentFill = MakeColor(255, 255, 205, 116);
        semantics.theme.pedestalFill = MakeColor(105, 122, 93, 62);
        semantics.theme.accessoryFill = MakeColor(255, 255, 226, 148);
        semantics.theme.accessoryStroke = MakeColor(255, 156, 118, 52);

        semantics.frame.bodyWidthScale = style.creamBodyWidthScale;
        semantics.frame.bodyHeightScale = style.creamBodyHeightScale;
        semantics.frame.headWidthScale = style.creamHeadWidthScale;
        semantics.frame.headHeightScale = style.creamHeadHeightScale;
        semantics.frame.shoulderPatchScale = style.creamShoulderPatchScale;
        semantics.frame.hipPatchScale = style.creamHipPatchScale;

        semantics.face.blushWidthScale = style.creamBlushWidthScale;
        semantics.face.muzzleWidthScale = style.creamMuzzleWidthScale;
        semantics.face.foreheadWidthScale = style.creamForeheadWidthScale;
        semantics.face.browTiltScale = style.creamBrowTiltScale;
        semantics.face.mouthReactiveScale = style.creamMouthReactiveScale;

        semantics.appendage.tailWidthScale = style.creamTailWidthScale;
        semantics.appendage.tailHeightScale = style.creamTailHeightScale;
        semantics.appendage.earScale = style.creamEarScale;
        semantics.appendage.followHandReachScale = style.creamFollowHandReachScale;
        semantics.appendage.holdLegStanceScale = style.creamHoldLegStanceScale;
        semantics.appendage.followTailWidthScale = style.creamFollowTailWidthScale;
        semantics.appendage.clickEarLiftScale = style.creamClickEarLiftScale;

        semantics.motion.followStateLiftScale = style.creamFollowStateLiftScale;
        semantics.motion.clickSquashScale = style.creamClickSquashScale;
    } else if (skinVariantId == "night") {
            semantics.theme.glowColor = MakeColor(255, 96, 120, 255);
            semantics.theme.baseBodyFill = MakeColor(255, 73, 87, 126);
            semantics.theme.bodyStroke = MakeColor(255, 215, 223, 255);
            semantics.theme.headFill = MakeColor(255, 122, 134, 171);
            semantics.theme.headFillRear = MakeColor(255, 93, 104, 140);
            semantics.theme.earFill = MakeColor(255, 113, 125, 163);
            semantics.theme.earFillRear = MakeColor(255, 83, 95, 129);
            semantics.theme.earInner = MakeColor(255, 169, 144, 196);
            semantics.theme.earInnerRear = MakeColor(255, 133, 112, 162);
            semantics.theme.blushRgb = MakeColor(255, 208, 146, 188);
            semantics.theme.accentFill = MakeColor(255, 132, 214, 255);
            semantics.theme.pedestalFill = MakeColor(105, 39, 47, 80);
            semantics.theme.accessoryFill = MakeColor(255, 193, 208, 255);
            semantics.theme.accessoryStroke = MakeColor(255, 90, 104, 158);

            semantics.frame.bodyWidthScale = style.nightBodyWidthScale;
            semantics.frame.bodyHeightScale = style.nightBodyHeightScale;
            semantics.frame.headWidthScale = style.nightHeadWidthScale;
            semantics.frame.headHeightScale = style.nightHeadHeightScale;
            semantics.frame.shoulderPatchScale = style.nightShoulderPatchScale;
            semantics.frame.hipPatchScale = style.nightHipPatchScale;

            semantics.face.jawHeightScale = style.nightJawHeightScale;
            semantics.face.templeHeightScale = style.nightTempleHeightScale;
            semantics.face.muzzleHeightScale = style.nightMuzzleHeightScale;
            semantics.face.browTiltScale = style.nightBrowTiltScale;
            semantics.face.pupilFocusScale = style.nightPupilFocusScale;

            semantics.appendage.tailWidthScale = style.nightTailWidthScale;
            semantics.appendage.tailHeightScale = style.nightTailHeightScale;
            semantics.appendage.earScale = style.nightEarScale;
            semantics.appendage.dragHandReachScale = style.nightDragHandReachScale;
            semantics.appendage.holdLegStanceScale = style.nightHoldLegStanceScale;
            semantics.appendage.scrollTailHeightScale = style.nightScrollTailHeightScale;
            semantics.appendage.followEarSpreadScale = style.nightFollowEarSpreadScale;

            semantics.motion.dragLeanScale = style.nightDragLeanScale;
            semantics.motion.bodyForwardScale = style.nightBodyForwardScale;
            semantics.motion.holdHeadNodScale = style.nightHoldHeadNodScale;
        } else if (skinVariantId == "strawberry") {
            semantics.theme.glowColor = MakeColor(255, 255, 126, 168);
            semantics.theme.baseBodyFill = MakeColor(255, 255, 228, 235);
            semantics.theme.bodyStroke = MakeColor(255, 168, 82, 108);
            semantics.theme.headFill = MakeColor(255, 255, 240, 242);
            semantics.theme.headFillRear = MakeColor(255, 244, 214, 222);
            semantics.theme.earFill = MakeColor(255, 255, 233, 236);
            semantics.theme.earFillRear = MakeColor(255, 241, 206, 214);
            semantics.theme.earInner = MakeColor(255, 255, 170, 188);
            semantics.theme.earInnerRear = MakeColor(255, 236, 142, 164);
            semantics.theme.blushRgb = MakeColor(255, 255, 144, 168);
            semantics.theme.accentFill = MakeColor(255, 255, 112, 158);
            semantics.theme.pedestalFill = MakeColor(105, 112, 48, 72);
            semantics.theme.accessoryFill = MakeColor(255, 255, 202, 110);
            semantics.theme.accessoryStroke = MakeColor(255, 158, 68, 44);

            semantics.frame.bodyWidthScale = style.strawberryBodyWidthScale;
            semantics.frame.bodyHeightScale = style.strawberryBodyHeightScale;
            semantics.frame.headWidthScale = style.strawberryHeadWidthScale;
            semantics.frame.headHeightScale = style.strawberryHeadHeightScale;
            semantics.frame.shoulderPatchScale = style.strawberryShoulderPatchScale;
            semantics.frame.hipPatchScale = style.strawberryHipPatchScale;

            semantics.face.blushWidthScale = style.strawberryBlushWidthScale;
            semantics.face.cheekWidthScale = style.strawberryCheekWidthScale;
            semantics.face.cheekHeightScale = style.strawberryCheekHeightScale;
            semantics.face.foreheadHeightScale = style.strawberryForeheadHeightScale;
            semantics.face.highlightAlphaScale = style.strawberryHighlightAlphaScale;
            semantics.face.whiskerSpreadScale = style.strawberryWhiskerSpreadScale;

            semantics.appendage.tailWidthScale = style.strawberryTailWidthScale;
            semantics.appendage.tailHeightScale = style.strawberryTailHeightScale;
            semantics.appendage.earScale = style.strawberryEarScale;
            semantics.appendage.followHandReachScale = style.strawberryFollowHandReachScale;
            semantics.appendage.followLegStanceScale = style.strawberryFollowLegStanceScale;
            semantics.appendage.followTailWidthScale = style.strawberryFollowTailWidthScale;
            semantics.appendage.followEarSpreadScale = style.strawberryFollowEarSpreadScale;

            semantics.motion.followHeadNodScale = style.strawberryFollowHeadNodScale;
            semantics.motion.followTailSwingScale = style.strawberryFollowTailSwingScale;
            semantics.motion.scrollTailLiftScale = style.strawberryScrollTailLiftScale;
        }

        if (semantics.adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Moon) {
            semantics.theme.accessoryFill = MakeColor(255, 210, 226, 255);
            semantics.theme.accessoryStroke = MakeColor(255, 92, 106, 152);
            semantics.adornment.xOffsetRatio = style.accessoryMoonXOffsetRatio;
            semantics.adornment.yOffsetRatio = style.accessoryMoonYOffsetRatio;
            semantics.adornment.widthScale = 1.02f;
            semantics.adornment.heightScale = 1.04f;
            semantics.adornment.followLiftScale = 1.12f;
            semantics.adornment.scrollLiftScale = 1.08f;
            semantics.face.highlightAlphaScale *= 1.06f;
            semantics.face.pupilFocusScale *= 1.04f;
            semantics.frame.headHeightScale *= 1.02f;
            semantics.appendage.earScale *= 0.97f;
            semantics.appendage.followEarSpreadScale *= 0.96f;
            semantics.motion.followStateLiftScale *= 1.04f;
            semantics.mood.glowTintMixScale = 1.10f;
            semantics.mood.shadowTintMixScale = 0.94f;
            semantics.mood.pedestalTintMixScale = 0.95f;
            semantics.mood.shadowAlphaBias = -4.0f;
            semantics.mood.pedestalAlphaBias = -3.0f;
            semantics.mood.followTrailAlphaScale = 1.08f;
            semantics.mood.scrollArcAlphaScale = 1.04f;
        } else if (semantics.adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Leaf) {
            semantics.theme.accessoryFill = MakeColor(255, 151, 224, 148);
            semantics.theme.accessoryStroke = MakeColor(255, 74, 126, 72);
            semantics.adornment.xOffsetRatio = style.accessoryLeafXOffsetRatio;
            semantics.adornment.yOffsetRatio = style.accessoryLeafYOffsetRatio;
            semantics.adornment.widthScale = 0.98f;
            semantics.adornment.heightScale = 1.06f;
            semantics.adornment.dragShiftScale = 1.16f;
            semantics.adornment.scrollLiftScale = 1.10f;
            semantics.face.whiskerSpreadScale *= 1.06f;
            semantics.frame.bodyHeightScale *= 1.01f;
            semantics.appendage.dragHandReachScale *= 1.08f;
            semantics.appendage.followEarSpreadScale *= 1.05f;
            semantics.motion.dragLeanScale *= 1.05f;
            semantics.motion.followTailSwingScale *= 1.04f;
            semantics.mood.glowTintMixScale = 0.96f;
            semantics.mood.accentTintMixScale = 1.04f;
            semantics.mood.shadowTintMixScale = 1.08f;
            semantics.mood.pedestalTintMixScale = 1.06f;
            semantics.mood.scrollArcAlphaScale = 1.10f;
            semantics.mood.dragLineAlphaScale = 1.10f;
            semantics.mood.followTrailAlphaScale = 1.04f;
        } else if (semantics.adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::RibbonBow) {
            semantics.theme.accessoryFill = MakeColor(255, 255, 138, 170);
            semantics.theme.accessoryStroke = MakeColor(255, 162, 72, 108);
            semantics.adornment.xOffsetRatio = style.accessoryRibbonXOffsetRatio;
            semantics.adornment.yOffsetRatio = style.accessoryRibbonYOffsetRatio;
            semantics.adornment.widthScale = 1.06f;
            semantics.adornment.heightScale = 0.96f;
            semantics.adornment.clickBounceScale = 1.14f;
            semantics.adornment.holdSettleScale = 1.10f;
            semantics.face.blushWidthScale *= 1.08f;
            semantics.face.cheekWidthScale *= 1.04f;
            semantics.frame.headWidthScale *= 1.03f;
            semantics.appendage.clickEarLiftScale *= 1.06f;
            semantics.motion.clickSquashScale *= 1.04f;
            semantics.motion.holdHeadNodScale *= 0.96f;
            semantics.mood.glowTintMixScale = 1.04f;
            semantics.mood.accentTintMixScale = 1.10f;
            semantics.mood.shadowAlphaBias = 3.0f;
            semantics.mood.pedestalAlphaBias = 2.0f;
            semantics.mood.clickRingAlphaScale = 1.12f;
            semantics.mood.holdBandAlphaScale = 1.06f;
        } else if (semantics.adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Star) {
            semantics.adornment.xOffsetRatio = 0.0f;
            semantics.adornment.yOffsetRatio = 0.0f;
        }

        if (comboPresetOverride !=
            Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
            semantics.comboPreset = comboPresetOverride;
        }
        ApplyComboPreset(semantics);
        ApplyAppearanceSemanticsTuning(semantics, tuning);
        if (patch) {
            ApplyAppearanceSemanticsPatch(semantics, *patch);
        }
        return semantics;
    }
class Win32MouseCompanionBuiltinRenderPlugin final : public IWin32MouseCompanionRenderPlugin {
public:
    const char* PluginId() const override {
        return "mousefx.windows.pet.render-plugin.builtin";
    }

    const char* PluginKind() const override {
        return "native_builtin";
    }

    const char* PluginSource() const override {
        return "windows_renderer_host";
    }

    Win32MouseCompanionRealRendererAppearanceSemantics BuildAppearanceSemantics(
        const Win32MouseCompanionRealRendererSceneRuntime& runtime,
        const Win32MouseCompanionRealRendererStyleProfile& style) const override {
        return BuildBuiltinAppearanceSemantics(
            runtime,
            style,
            Win32MouseCompanionRealRendererAppearanceComboPreset::None,
            {},
            nullptr);
    }
};

class Win32MouseCompanionWasmRenderPluginAdapter final : public IWin32MouseCompanionRenderPlugin {
public:
    const char* PluginId() const override {
        return "mousefx.windows.pet.render-plugin.wasm-adapter";
    }

    const char* PluginKind() const override {
        return "wasm";
    }

    const char* PluginSource() const override {
        return "windows_renderer_host";
    }

    Win32MouseCompanionRealRendererAppearanceSemantics BuildAppearanceSemantics(
        const Win32MouseCompanionRealRendererSceneRuntime& runtime,
        const Win32MouseCompanionRealRendererStyleProfile& style) const override {
        return BuildBuiltinAppearanceSemantics(
            runtime,
            style,
            comboPresetOverride_,
            tuning_,
            appearanceSemanticsMode_ == "wasm_v1" ? &appearanceSemanticsPatch_ : nullptr);
    }

    Win32MouseCompanionRenderPluginSelection PrepareSelection(
        const std::string& manifestPathUtf8) const {
        std::lock_guard<std::mutex> guard(mutex_);

        Win32MouseCompanionRenderPluginSelection selection{};
        selection.plugin = this;
        selection.pluginId = PluginId();
        selection.pluginKind = PluginKind();
        selection.pluginSource = "wasm_manifest";
        selection.manifestPath = manifestPathUtf8;

        const std::string trimmedManifestPath = TrimAscii(manifestPathUtf8);
        if (trimmedManifestPath.empty()) {
            selection.failureReason = "wasm_manifest_path_missing";
            return selection;
        }

        if (trimmedManifestPath != attemptedManifestPath_) {
            attemptedManifestPath_ = trimmedManifestPath;
            manifestPluginId_.clear();
            const auto preflight =
                PreflightWin32MouseCompanionWasmRenderPluginManifest(trimmedManifestPath);
            if (!preflight.ok) {
                host_.reset();
                failureReason_ = preflight.failureReason;
                metadataPath_ = preflight.metadataPath;
                metadataSchemaVersion_ = preflight.metadataSchemaVersion;
                appearanceSemanticsMode_ = preflight.appearanceSemanticsMode;
                styleIntent_ = preflight.styleIntent;
                sampleTier_ = preflight.sampleTier;
                comboPresetOverride_ = preflight.comboPresetOverride;
                tuning_ = preflight.tuning;
                appearanceSemanticsPatch_ = preflight.appearanceSemanticsPatch;
                return selection;
            }
            manifestPluginId_ = preflight.pluginId;
            metadataPath_ = preflight.metadataPath;
            metadataSchemaVersion_ = preflight.metadataSchemaVersion;
            appearanceSemanticsMode_ = preflight.appearanceSemanticsMode;
            styleIntent_ = preflight.styleIntent;
            sampleTier_ = preflight.sampleTier;
            comboPresetOverride_ = preflight.comboPresetOverride;
            tuning_ = preflight.tuning;
            appearanceSemanticsPatch_ = preflight.appearanceSemanticsPatch;
            host_ = std::make_unique<wasm::WasmEffectHost>();
            if (!host_->LoadPluginFromManifest(Utf8ToWString(trimmedManifestPath))) {
                failureReason_ = BuildWin32MouseCompanionWasmRenderPluginLoadFailureReason(
                    host_->Diagnostics());
            } else {
                failureReason_.clear();
            }
        }

        if (!host_ || !host_->IsPluginLoaded()) {
            selection.failureReason =
                failureReason_.empty() ? "wasm_render_plugin_not_loaded" : failureReason_;
            if (host_) {
                selection.runtimeBackend = host_->Diagnostics().runtimeBackend;
            }
            selection.metadataPath = metadataPath_;
            selection.metadataSchemaVersion = metadataSchemaVersion_;
            selection.appearanceSemanticsMode = appearanceSemanticsMode_;
            selection.declaredStyleIntent = styleIntent_;
            selection.declaredSampleTier = sampleTier_;
            return selection;
        }

        const auto& diagnostics = host_->Diagnostics();
        selection.pluginId = TrimAscii(diagnostics.activePluginId).empty()
            ? (TrimAscii(manifestPluginId_).empty() ? PluginId() : manifestPluginId_)
            : diagnostics.activePluginId;
        selection.pluginId = TrimAscii(selection.pluginId).empty()
            ? PluginId()
            : selection.pluginId;
        selection.pluginSource = "wasm_manifest_loaded";
        selection.selectionReason = "wasm_skeleton_adapter_active";
        selection.runtimeBackend = diagnostics.runtimeBackend;
        selection.metadataPath = metadataPath_;
        selection.metadataSchemaVersion = metadataSchemaVersion_;
        selection.appearanceSemanticsMode = appearanceSemanticsMode_;
        selection.declaredStyleIntent = styleIntent_;
        selection.declaredSampleTier = sampleTier_;
        selection.comboPresetOverride = comboPresetOverride_;
        selection.tuning = tuning_;
        return selection;
    }

private:
    mutable std::mutex mutex_{};
    mutable std::string attemptedManifestPath_{};
    mutable std::string manifestPluginId_{};
    mutable std::string metadataPath_{};
    mutable uint32_t metadataSchemaVersion_{0};
    mutable std::string appearanceSemanticsMode_{"legacy_manifest_compat"};
    mutable std::string styleIntent_{};
    mutable std::string sampleTier_{};
    mutable Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride_{
        Win32MouseCompanionRealRendererAppearanceComboPreset::None};
    mutable Win32MouseCompanionRendererPluginAppearanceSemanticsTuning tuning_{};
    mutable Win32MouseCompanionRendererPluginAppearanceSemanticsPatch
        appearanceSemanticsPatch_{};
    mutable std::string failureReason_{};
    mutable std::unique_ptr<wasm::WasmEffectHost> host_{};
};

} // namespace

const IWin32MouseCompanionRenderPlugin&
GetDefaultWin32MouseCompanionRenderPlugin() {
    static const Win32MouseCompanionBuiltinRenderPlugin plugin{};
    return plugin;
}

Win32MouseCompanionRenderPluginSelection
ResolveWin32MouseCompanionRenderPluginSelection() {
    Win32MouseCompanionRenderPluginSelection selection{};
    selection.plugin = &GetDefaultWin32MouseCompanionRenderPlugin();
    selection.pluginId = ReadDefaultWin32MouseCompanionRenderPluginId();
    selection.pluginKind = ReadDefaultWin32MouseCompanionRenderPluginKind();
    selection.pluginSource = ReadDefaultWin32MouseCompanionRenderPluginSource();

    const std::string pluginMode = NormalizePluginMode(ReadEnvCopy(kPluginModeEnvVar));
    if (pluginMode.empty() || pluginMode == kPluginModeAuto) {
        selection.selectionReason = "default_builtin";
        ApplyDefaultLaneDecision(&selection);
        return selection;
    }
    if (pluginMode == kPluginModeBuiltin) {
        selection.selectionReason = "env:explicit_builtin";
        selection.pluginSource = "env:builtin";
        ApplyDefaultLaneDecision(&selection);
        return selection;
    }
    if (pluginMode != kPluginModeWasm) {
        selection.selectionReason = "env:invalid_plugin_mode_fallback_builtin";
        selection.failureReason = "unsupported_render_plugin_mode:" + pluginMode;
        selection.pluginSource = "env:fallback_builtin";
        ApplyDefaultLaneDecision(&selection);
        return selection;
    }

    const std::string manifestPath = TrimAscii(ReadEnvCopy(kPluginManifestEnvVar));
    if (manifestPath.empty()) {
        selection.selectionReason = "env:wasm_requested_fallback_builtin";
        selection.failureReason = "wasm_manifest_path_missing";
        selection.pluginSource = "env:fallback_builtin";
        ApplyDefaultLaneDecision(&selection);
        return selection;
    }

    static const Win32MouseCompanionWasmRenderPluginAdapter wasmPlugin{};
    auto wasmSelection = wasmPlugin.PrepareSelection(manifestPath);
    if (!TrimAscii(wasmSelection.failureReason).empty()) {
        selection.selectionReason = "env:wasm_requested_fallback_builtin";
        selection.failureReason = wasmSelection.failureReason;
        selection.manifestPath = manifestPath;
        selection.runtimeBackend = wasmSelection.runtimeBackend;
        selection.metadataPath = wasmSelection.metadataPath;
        selection.metadataSchemaVersion = wasmSelection.metadataSchemaVersion;
        selection.appearanceSemanticsMode = wasmSelection.appearanceSemanticsMode;
        selection.declaredStyleIntent = wasmSelection.declaredStyleIntent;
        selection.declaredSampleTier = wasmSelection.declaredSampleTier;
        selection.comboPresetOverride = wasmSelection.comboPresetOverride;
        selection.pluginSource = "env:fallback_builtin";
        ApplyDefaultLaneDecision(&selection);
        return selection;
    }

    if (wasmSelection.plugin) {
        wasmSelection.selectionReason = "env:wasm_requested";
        ApplyDefaultLaneDecision(&wasmSelection);
        return wasmSelection;
    }

    selection.selectionReason = "env:wasm_requested_fallback_builtin";
    selection.failureReason = "wasm_plugin_selection_failed";
    selection.manifestPath = manifestPath;
    selection.pluginSource = "env:fallback_builtin";
    ApplyDefaultLaneDecision(&selection);
    return selection;
}

Win32MouseCompanionRealRendererAppearanceSemantics
BuildWin32MouseCompanionRenderPluginAppearanceSemantics(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style) {
    const auto selection = ResolveWin32MouseCompanionRenderPluginSelection();
    const IWin32MouseCompanionRenderPlugin* plugin =
        selection.plugin ? selection.plugin : &GetDefaultWin32MouseCompanionRenderPlugin();
    return plugin->BuildAppearanceSemantics(runtime, style);
}

const char* ReadDefaultWin32MouseCompanionRenderPluginId() {
    return GetDefaultWin32MouseCompanionRenderPlugin().PluginId();
}

const char* ReadDefaultWin32MouseCompanionRenderPluginKind() {
    return GetDefaultWin32MouseCompanionRenderPlugin().PluginKind();
}

const char* ReadDefaultWin32MouseCompanionRenderPluginSource() {
    return GetDefaultWin32MouseCompanionRenderPlugin().PluginSource();
}

} // namespace mousefx::windows
