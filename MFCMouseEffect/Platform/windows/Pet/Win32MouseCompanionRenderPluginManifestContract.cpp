#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginManifestContract.h"

#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmPluginManifest.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

#include <filesystem>
#include <fstream>
#include <sstream>

namespace mousefx::windows {
namespace {

constexpr const char* kRendererMetadataLane = "mouse_companion_renderer";
constexpr const char* kAppearanceSemanticsModeLegacyManifestCompat =
    "legacy_manifest_compat";
constexpr const char* kAppearanceSemanticsModeBuiltinPassthrough =
    "builtin_passthrough";
constexpr const char* kAppearanceSemanticsModeWasmV1 = "wasm_v1";

bool IsSupportedDeclaredStyleIntent(const std::string& value) {
    return value == "style_candidate:none" ||
        value == "style_candidate:builtin_passthrough_baseline" ||
        value == "style_candidate:balanced_default_candidate" ||
        value == "style_candidate:agile_follow_drag" ||
        value == "style_candidate:dreamy_follow_scroll" ||
        value == "style_candidate:charming_click_hold" ||
        value == "style_candidate:single_selected_wasm_v1";
}

bool IsSupportedDeclaredSampleTier(const std::string& value) {
    return value == "baseline_reference" ||
        value == "ship_default_candidate" ||
        value == "experimental_style_candidate";
}

std::filesystem::path ResolveRendererMetadataPath(
    const std::filesystem::path& manifestPath) {
    std::filesystem::path metadataPath = manifestPath;
    metadataPath.replace_extension(".mouse_companion_renderer.json");
    return metadataPath;
}

std::string ReadUtf8TextFile(
    const std::filesystem::path& filePath,
    std::string* outError) {
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open()) {
        if (outError) {
            *outError = "Cannot open renderer plugin metadata.";
        }
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (input.bad()) {
        if (outError) {
            *outError = "Failed reading renderer plugin metadata.";
        }
        return {};
    }
    return buffer.str();
}

std::string ClassifyRendererPluginManifestLoadFailure(const std::string& message) {
    const std::string lowered = ToLowerAscii(message);
    if (lowered.find("does not exist") != std::string::npos ||
        lowered.find("cannot open") != std::string::npos ||
        lowered.find("failed reading") != std::string::npos ||
        lowered.find("file is empty") != std::string::npos) {
        return "renderer_plugin_manifest_io_error";
    }
    if (lowered.find("json parse error") != std::string::npos) {
        return "renderer_plugin_manifest_json_parse_error";
    }
    return "renderer_plugin_manifest_invalid";
}

std::string ClassifyRendererPluginMetadataLoadFailure(const std::string& message) {
    const std::string lowered = ToLowerAscii(message);
    if (lowered.find("cannot open") != std::string::npos ||
        lowered.find("failed reading") != std::string::npos ||
        lowered.find("does not exist") != std::string::npos) {
        return "renderer_plugin_metadata_io_error";
    }
    if (lowered.find("json parse error") != std::string::npos) {
        return "renderer_plugin_metadata_json_parse_error";
    }
    return "renderer_plugin_metadata_invalid";
}

bool TryReadTuningFloat(
    const json& root,
    const char* key,
    float* outValue,
    std::string* outFailureReason) {
    if (!outValue) {
        return false;
    }
    if (!root.contains(key)) {
        return true;
    }
    if (!root[key].is_number()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_invalid";
        }
        return false;
    }
    const float value = root[key].get<float>();
    if (value < 0.5f || value > 1.5f) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_tuning_out_of_range";
        }
        return false;
    }
    *outValue = value;
    return true;
}

bool TryParseHexByte(char c, BYTE* outValue) {
    if (!outValue) {
        return false;
    }
    if (c >= '0' && c <= '9') {
        *outValue = static_cast<BYTE>(c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        *outValue = static_cast<BYTE>(10 + (c - 'a'));
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        *outValue = static_cast<BYTE>(10 + (c - 'A'));
        return true;
    }
    return false;
}

bool TryParseHexColor(
    const std::string& raw,
    Gdiplus::Color* outColor) {
    if (!outColor) {
        return false;
    }
    std::string text = TrimAscii(raw);
    if (text.empty()) {
        return false;
    }
    if (!text.empty() && text.front() == '#') {
        text.erase(text.begin());
    }
    if (text.size() != 6u && text.size() != 8u) {
        return false;
    }

    BYTE parts[4]{255u, 0u, 0u, 0u};
    size_t srcOffset = 0u;
    if (text.size() == 8u) {
        srcOffset = 0u;
    } else {
        srcOffset = 1u;
    }

    for (size_t i = 0; i < text.size() / 2u; ++i) {
        BYTE high = 0u;
        BYTE low = 0u;
        if (!TryParseHexByte(text[i * 2u], &high) ||
            !TryParseHexByte(text[i * 2u + 1u], &low)) {
            return false;
        }
        parts[srcOffset + i] = static_cast<BYTE>((high << 4u) | low);
    }
    *outColor = Gdiplus::Color(parts[0], parts[1], parts[2], parts[3]);
    return true;
}

bool TryReadBoundedFloat(
    const json& root,
    const char* key,
    float minValue,
    float maxValue,
    float* outValue,
    std::string* outFailureReason) {
    if (!outValue) {
        return false;
    }
    if (!root.contains(key)) {
        return true;
    }
    if (!root[key].is_number()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_appearance_payload_invalid";
        }
        return false;
    }
    const float value = root[key].get<float>();
    if (value < minValue || value > maxValue) {
        if (outFailureReason) {
            *outFailureReason =
                "renderer_plugin_metadata_appearance_payload_out_of_range";
        }
        return false;
    }
    *outValue = value;
    return true;
}

bool TryReadOptionalColor(
    const json& root,
    const char* key,
    bool* outHasValue,
    Gdiplus::Color* outColor,
    std::string* outFailureReason) {
    if (!outHasValue || !outColor) {
        return false;
    }
    if (!root.contains(key)) {
        return true;
    }
    if (!root[key].is_string() ||
        !TryParseHexColor(root[key].get<std::string>(), outColor)) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_appearance_payload_invalid";
        }
        return false;
    }
    *outHasValue = true;
    return true;
}

bool ValidateAppearanceSemanticsPayload(
    const json& root,
    Win32MouseCompanionRendererPluginAppearanceSemanticsPatch* outPatch,
    std::string* outFailureReason) {
    if (!outPatch) {
        return false;
    }
    *outPatch = {};

    if (!root.contains("appearance_semantics")) {
        if (outFailureReason) {
            *outFailureReason =
                "renderer_plugin_metadata_missing_appearance_semantics_payload";
        }
        return false;
    }
    const json& payload = root["appearance_semantics"];
    if (!payload.is_object()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_appearance_payload_invalid";
        }
        return false;
    }

    if (payload.contains("theme")) {
        if (!payload["theme"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& theme = payload["theme"];
        if (!TryReadOptionalColor(
                theme,
                "glow_color",
                &outPatch->hasThemeGlowColor,
                &outPatch->themeGlowColor,
                outFailureReason) ||
            !TryReadOptionalColor(
                theme,
                "body_stroke",
                &outPatch->hasThemeBodyStroke,
                &outPatch->themeBodyStroke,
                outFailureReason) ||
            !TryReadOptionalColor(
                theme,
                "head_fill",
                &outPatch->hasThemeHeadFill,
                &outPatch->themeHeadFill,
                outFailureReason) ||
            !TryReadOptionalColor(
                theme,
                "accent_fill",
                &outPatch->hasThemeAccentFill,
                &outPatch->themeAccentFill,
                outFailureReason) ||
            !TryReadOptionalColor(
                theme,
                "accessory_fill",
                &outPatch->hasThemeAccessoryFill,
                &outPatch->themeAccessoryFill,
                outFailureReason) ||
            !TryReadOptionalColor(
                theme,
                "pedestal_fill",
                &outPatch->hasThemePedestalFill,
                &outPatch->themePedestalFill,
                outFailureReason)) {
            return false;
        }
    }

    if (payload.contains("frame")) {
        if (!payload["frame"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& frame = payload["frame"];
        if (frame.contains("body_width_scale")) {
            outPatch->hasFrameBodyWidthScale = true;
            if (!TryReadBoundedFloat(
                    frame,
                    "body_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->frameBodyWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (frame.contains("body_height_scale")) {
            outPatch->hasFrameBodyHeightScale = true;
            if (!TryReadBoundedFloat(
                    frame,
                    "body_height_scale",
                    0.5f,
                    1.5f,
                    &outPatch->frameBodyHeightScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (frame.contains("head_width_scale")) {
            outPatch->hasFrameHeadWidthScale = true;
            if (!TryReadBoundedFloat(
                    frame,
                    "head_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->frameHeadWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (frame.contains("head_height_scale")) {
            outPatch->hasFrameHeadHeightScale = true;
            if (!TryReadBoundedFloat(
                    frame,
                    "head_height_scale",
                    0.5f,
                    1.5f,
                    &outPatch->frameHeadHeightScale,
                    outFailureReason)) {
                return false;
            }
        }
    }

    if (payload.contains("face")) {
        if (!payload["face"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& face = payload["face"];
        if (face.contains("blush_width_scale")) {
            outPatch->hasFaceBlushWidthScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "blush_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->faceBlushWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (face.contains("muzzle_width_scale")) {
            outPatch->hasFaceMuzzleWidthScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "muzzle_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->faceMuzzleWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (face.contains("forehead_width_scale")) {
            outPatch->hasFaceForeheadWidthScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "forehead_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->faceForeheadWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (face.contains("pupil_focus_scale")) {
            outPatch->hasFacePupilFocusScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "pupil_focus_scale",
                    0.5f,
                    1.5f,
                    &outPatch->facePupilFocusScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (face.contains("highlight_alpha_scale")) {
            outPatch->hasFaceHighlightAlphaScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "highlight_alpha_scale",
                    0.5f,
                    1.5f,
                    &outPatch->faceHighlightAlphaScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (face.contains("whisker_spread_scale")) {
            outPatch->hasFaceWhiskerSpreadScale = true;
            if (!TryReadBoundedFloat(
                    face,
                    "whisker_spread_scale",
                    0.5f,
                    1.5f,
                    &outPatch->faceWhiskerSpreadScale,
                    outFailureReason)) {
                return false;
            }
        }
    }

    if (payload.contains("appendage")) {
        if (!payload["appendage"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& appendage = payload["appendage"];
        if (appendage.contains("ear_scale")) {
            outPatch->hasAppendageEarScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "ear_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageEarScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (appendage.contains("tail_width_scale")) {
            outPatch->hasAppendageTailWidthScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "tail_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageTailWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (appendage.contains("tail_height_scale")) {
            outPatch->hasAppendageTailHeightScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "tail_height_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageTailHeightScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (appendage.contains("follow_tail_width_scale")) {
            outPatch->hasAppendageFollowTailWidthScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "follow_tail_width_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageFollowTailWidthScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (appendage.contains("follow_ear_spread_scale")) {
            outPatch->hasAppendageFollowEarSpreadScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "follow_ear_spread_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageFollowEarSpreadScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (appendage.contains("click_ear_lift_scale")) {
            outPatch->hasAppendageClickEarLiftScale = true;
            if (!TryReadBoundedFloat(
                    appendage,
                    "click_ear_lift_scale",
                    0.5f,
                    1.5f,
                    &outPatch->appendageClickEarLiftScale,
                    outFailureReason)) {
                return false;
            }
        }
    }

    if (payload.contains("motion")) {
        if (!payload["motion"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& motion = payload["motion"];
        if (motion.contains("follow_state_lift_scale")) {
            outPatch->hasMotionFollowStateLiftScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "follow_state_lift_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionFollowStateLiftScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (motion.contains("click_squash_scale")) {
            outPatch->hasMotionClickSquashScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "click_squash_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionClickSquashScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (motion.contains("drag_lean_scale")) {
            outPatch->hasMotionDragLeanScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "drag_lean_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionDragLeanScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (motion.contains("hold_head_nod_scale")) {
            outPatch->hasMotionHoldHeadNodScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "hold_head_nod_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionHoldHeadNodScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (motion.contains("scroll_tail_lift_scale")) {
            outPatch->hasMotionScrollTailLiftScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "scroll_tail_lift_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionScrollTailLiftScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (motion.contains("follow_head_nod_scale")) {
            outPatch->hasMotionFollowHeadNodScale = true;
            if (!TryReadBoundedFloat(
                    motion,
                    "follow_head_nod_scale",
                    0.5f,
                    1.5f,
                    &outPatch->motionFollowHeadNodScale,
                    outFailureReason)) {
                return false;
            }
        }
    }

    if (payload.contains("mood")) {
        if (!payload["mood"].is_object()) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_payload_invalid";
            }
            return false;
        }
        const json& mood = payload["mood"];
        if (mood.contains("glow_tint_mix_scale")) {
            outPatch->hasMoodGlowTintMixScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "glow_tint_mix_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodGlowTintMixScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("accent_tint_mix_scale")) {
            outPatch->hasMoodAccentTintMixScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "accent_tint_mix_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodAccentTintMixScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("shadow_tint_mix_scale")) {
            outPatch->hasMoodShadowTintMixScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "shadow_tint_mix_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodShadowTintMixScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("shadow_alpha_bias")) {
            outPatch->hasMoodShadowAlphaBias = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "shadow_alpha_bias",
                    -24.0f,
                    24.0f,
                    &outPatch->moodShadowAlphaBias,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("pedestal_alpha_bias")) {
            outPatch->hasMoodPedestalAlphaBias = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "pedestal_alpha_bias",
                    -24.0f,
                    24.0f,
                    &outPatch->moodPedestalAlphaBias,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("scroll_arc_alpha_scale")) {
            outPatch->hasMoodScrollArcAlphaScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "scroll_arc_alpha_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodScrollArcAlphaScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("hold_band_alpha_scale")) {
            outPatch->hasMoodHoldBandAlphaScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "hold_band_alpha_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodHoldBandAlphaScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("drag_line_alpha_scale")) {
            outPatch->hasMoodDragLineAlphaScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "drag_line_alpha_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodDragLineAlphaScale,
                    outFailureReason)) {
                return false;
            }
        }
        if (mood.contains("follow_trail_alpha_scale")) {
            outPatch->hasMoodFollowTrailAlphaScale = true;
            if (!TryReadBoundedFloat(
                    mood,
                    "follow_trail_alpha_scale",
                    0.5f,
                    1.5f,
                    &outPatch->moodFollowTrailAlphaScale,
                    outFailureReason)) {
                return false;
            }
        }
    }

    return true;
}

bool ValidateOptionalRendererMetadata(
    const std::filesystem::path& manifestPath,
    std::string* outMetadataPathUtf8,
    uint32_t* outMetadataSchemaVersion,
    std::string* outAppearanceSemanticsMode,
    std::string* outStyleIntent,
    std::string* outSampleTier,
    Win32MouseCompanionRealRendererAppearanceComboPreset* outComboPresetOverride,
    Win32MouseCompanionRendererPluginAppearanceSemanticsTuning* outTuning,
    Win32MouseCompanionRendererPluginAppearanceSemanticsPatch* outAppearanceSemanticsPatch,
    std::string* outFailureReason) {
    if (outMetadataPathUtf8) {
        outMetadataPathUtf8->clear();
    }
    if (outMetadataSchemaVersion) {
        *outMetadataSchemaVersion = 0;
    }
    if (outAppearanceSemanticsMode) {
        *outAppearanceSemanticsMode = kAppearanceSemanticsModeLegacyManifestCompat;
    }
    if (outStyleIntent) {
        outStyleIntent->clear();
    }
    if (outSampleTier) {
        outSampleTier->clear();
    }
    if (outComboPresetOverride) {
        *outComboPresetOverride = Win32MouseCompanionRealRendererAppearanceComboPreset::None;
    }
    if (outTuning) {
        *outTuning = {};
    }
    if (outAppearanceSemanticsPatch) {
        *outAppearanceSemanticsPatch = {};
    }
    if (outFailureReason) {
        outFailureReason->clear();
    }

    const std::filesystem::path metadataPath =
        ResolveRendererMetadataPath(manifestPath);
    std::error_code ec;
    if (!std::filesystem::exists(metadataPath, ec) || ec) {
        return true;
    }
    if (outMetadataPathUtf8) {
        *outMetadataPathUtf8 = Utf16ToUtf8(metadataPath.wstring().c_str());
    }

    std::string readError;
    const std::string body = ReadUtf8TextFile(metadataPath, &readError);
    if (!readError.empty()) {
        if (outFailureReason) {
            *outFailureReason = ClassifyRendererPluginMetadataLoadFailure(readError);
        }
        return false;
    }

    json root;
    try {
        root = json::parse(body);
    } catch (const std::exception&) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_json_parse_error";
        }
        return false;
    }
    if (!root.is_object()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_invalid";
        }
        return false;
    }
    if (!root.contains("schema_version") || !root["schema_version"].is_number_unsigned()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_invalid";
        }
        return false;
    }
    if (outMetadataSchemaVersion) {
        *outMetadataSchemaVersion = root["schema_version"].get<uint32_t>();
    }
    if (!root.contains("renderer_lane") || !root["renderer_lane"].is_string()) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_invalid";
        }
        return false;
    }
    const std::string rendererLane = TrimAscii(root["renderer_lane"].get<std::string>());
    if (rendererLane != kRendererMetadataLane) {
        if (outFailureReason) {
            *outFailureReason = "renderer_plugin_metadata_lane_mismatch";
        }
        return false;
    }
    if (!root.contains("supports_appearance_semantics") ||
        !root["supports_appearance_semantics"].is_boolean() ||
        !root["supports_appearance_semantics"].get<bool>()) {
        if (outFailureReason) {
            *outFailureReason =
                "renderer_plugin_metadata_missing_appearance_semantics";
        }
        return false;
    }
    if (root.contains("appearance_semantics_mode")) {
        if (!root["appearance_semantics_mode"].is_string()) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_invalid";
            }
            return false;
        }
        const std::string mode = TrimAscii(root["appearance_semantics_mode"].get<std::string>());
        if (mode != kAppearanceSemanticsModeBuiltinPassthrough &&
            mode != kAppearanceSemanticsModeWasmV1) {
            if (outFailureReason) {
                *outFailureReason =
                    "renderer_plugin_metadata_appearance_mode_unsupported";
            }
            return false;
        }
        if (outAppearanceSemanticsMode) {
            *outAppearanceSemanticsMode = mode;
        }
    } else if (outAppearanceSemanticsMode) {
        *outAppearanceSemanticsMode = kAppearanceSemanticsModeBuiltinPassthrough;
    }
    if (root.contains("style_intent")) {
        if (!root["style_intent"].is_string()) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_invalid";
            }
            return false;
        }
        const std::string styleIntent = TrimAscii(root["style_intent"].get<std::string>());
        if (!IsSupportedDeclaredStyleIntent(styleIntent)) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_style_intent_unsupported";
            }
            return false;
        }
        if (outStyleIntent) {
            *outStyleIntent = styleIntent;
        }
    }
    if (root.contains("sample_tier")) {
        if (!root["sample_tier"].is_string()) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_invalid";
            }
            return false;
        }
        const std::string sampleTier = TrimAscii(root["sample_tier"].get<std::string>());
        if (!IsSupportedDeclaredSampleTier(sampleTier)) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_sample_tier_unsupported";
            }
            return false;
        }
        if (outSampleTier) {
            *outSampleTier = sampleTier;
        }
    }
    if (root.contains("combo_preset_override")) {
        if (!root["combo_preset_override"].is_string()) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_invalid";
            }
            return false;
        }
        Win32MouseCompanionRealRendererAppearanceComboPreset parsedPreset =
            Win32MouseCompanionRealRendererAppearanceComboPreset::None;
        if (!TryParseWin32MouseCompanionRealRendererAppearanceComboPreset(
                root["combo_preset_override"].get<std::string>(),
                &parsedPreset)) {
            if (outFailureReason) {
                *outFailureReason = "renderer_plugin_metadata_combo_preset_unsupported";
            }
            return false;
        }
        if (outComboPresetOverride) {
            *outComboPresetOverride = parsedPreset;
        }
    }
    if (outTuning) {
        if (!TryReadTuningFloat(
                root,
                "follow_lift_scale",
                &outTuning->followLiftScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "click_squash_scale",
                &outTuning->clickSquashScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "drag_lean_scale",
                &outTuning->dragLeanScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "highlight_alpha_scale",
                &outTuning->highlightAlphaScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "follow_tail_swing_scale",
                &outTuning->followTailSwingScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "hold_head_nod_scale",
                &outTuning->holdHeadNodScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "scroll_tail_lift_scale",
                &outTuning->scrollTailLiftScale,
                outFailureReason)) {
            return false;
        }
        if (!TryReadTuningFloat(
                root,
                "follow_head_nod_scale",
                &outTuning->followHeadNodScale,
                outFailureReason)) {
            return false;
        }
    }
    if (outAppearanceSemanticsMode &&
        *outAppearanceSemanticsMode == kAppearanceSemanticsModeWasmV1 &&
        outAppearanceSemanticsPatch) {
        if (!ValidateAppearanceSemanticsPayload(
                root,
                outAppearanceSemanticsPatch,
                outFailureReason)) {
            return false;
        }
    }

    return true;
}

} // namespace

Win32MouseCompanionWasmRenderPluginManifestPreflight
PreflightWin32MouseCompanionWasmRenderPluginManifest(
    const std::string& manifestPathUtf8) {
    Win32MouseCompanionWasmRenderPluginManifestPreflight preflight{};
    const wasm::PluginManifestLoadResult load =
        wasm::WasmPluginManifest::LoadFromFile(Utf8ToWString(manifestPathUtf8));
    if (!load.ok) {
        preflight.failureReason = ClassifyRendererPluginManifestLoadFailure(load.error);
        return preflight;
    }

    std::string metadataFailureReason;
    if (!ValidateOptionalRendererMetadata(
            std::filesystem::path(Utf8ToWString(manifestPathUtf8)),
            &preflight.metadataPath,
            &preflight.metadataSchemaVersion,
            &preflight.appearanceSemanticsMode,
            &preflight.styleIntent,
            &preflight.sampleTier,
            &preflight.comboPresetOverride,
            &preflight.tuning,
            &preflight.appearanceSemanticsPatch,
            &metadataFailureReason)) {
        preflight.failureReason = metadataFailureReason;
        return preflight;
    }

    preflight.pluginId = load.manifest.id;

    if ((load.manifest.surfaceKindsMask & wasm::kManifestSurfaceEffectsBit) == 0u) {
        preflight.failureReason = "renderer_plugin_manifest_missing_effects_surface";
        return preflight;
    }
    if (!load.manifest.enableFrameTick) {
        preflight.failureReason = "renderer_plugin_manifest_requires_frame_tick";
        return preflight;
    }

    preflight.ok = true;
    return preflight;
}

std::string BuildWin32MouseCompanionWasmRenderPluginLoadFailureReason(
    const wasm::HostDiagnostics& diagnostics) {
    if (!TrimAscii(diagnostics.lastError).empty()) {
        return diagnostics.lastError;
    }
    if (!TrimAscii(diagnostics.lastLoadFailureCode).empty()) {
        if (!TrimAscii(diagnostics.lastLoadFailureStage).empty()) {
            return diagnostics.lastLoadFailureStage + ":" + diagnostics.lastLoadFailureCode;
        }
        return diagnostics.lastLoadFailureCode;
    }
    if (!TrimAscii(diagnostics.runtimeFallbackReason).empty()) {
        return diagnostics.runtimeFallbackReason;
    }
    return "wasm_render_plugin_load_failed";
}

} // namespace mousefx::windows
