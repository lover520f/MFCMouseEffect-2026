#pragma once

#include <cstdint>

namespace mousefx::wasm {

constexpr uint32_t kPluginApiVersionV2 = 2;
constexpr uint32_t kPluginApiVersionCurrent = kPluginApiVersionV2;

enum class CommandKind : uint16_t {
    SpawnText = 1,
    SpawnImage = 2,
    SpawnImageAffine = 3,
    SpawnPulse = 4,
    SpawnPolyline = 5,
    SpawnGlowBatch = 6,
    SpawnSpriteBatch = 7,
    UpsertGlowEmitter = 8,
    RemoveGlowEmitter = 9,
    SpawnPathStroke = 10,
    SpawnPathFill = 11,
    UpsertSpriteEmitter = 12,
    RemoveSpriteEmitter = 13,
    UpsertParticleEmitter = 14,
    RemoveParticleEmitter = 15,
    SpawnQuadBatch = 16,
    SpawnRibbonStrip = 17,
    UpsertRibbonTrail = 18,
    RemoveRibbonTrail = 19,
    UpsertQuadField = 20,
    RemoveQuadField = 21,
    RemoveGroup = 22,
    UpsertGroupPresentation = 23,
    UpsertGroupClipRect = 24,
    UpsertGroupLayer = 25,
    UpsertGroupTransform = 26,
    UpsertGroupLocalOrigin = 27,
    UpsertGroupMaterial = 28,
    UpsertGroupPass = 29,
};

// Unified input event kinds for mfx_plugin_on_input entry.
enum class InputEventKind : uint8_t {
    Click = 1,
    Move = 2,
    Scroll = 3,
    HoldStart = 4,
    HoldUpdate = 5,
    HoldEnd = 6,
    HoverStart = 7,
    HoverEnd = 8,
    IndicatorClick = 9,
    IndicatorScroll = 10,
    IndicatorKey = 11,
};

constexpr uint8_t kEventFlagScrollHorizontal = 0x01u;
constexpr uint32_t kTextIdEventLabel = 0xFFFFFFFFu;
constexpr uint8_t kIndicatorModifierCtrl = 0x01u;
constexpr uint8_t kIndicatorModifierShift = 0x02u;
constexpr uint8_t kIndicatorModifierAlt = 0x04u;
constexpr uint8_t kIndicatorModifierMeta = 0x08u;
constexpr uint8_t kIndicatorDetailFlagKeySystem = 0x01u;

enum class SpawnPulseKind : uint32_t {
    Ripple = 0,
    Star = 1,
};

constexpr uint16_t kSpawnPolylineFlagClosed = 0x0001u;
constexpr uint16_t kSpawnGlowBatchFlagScreenBlend = 0x0001u;
constexpr uint16_t kSpawnSpriteBatchFlagScreenBlend = 0x0001u;
constexpr uint16_t kSpawnQuadBatchFlagScreenBlend = 0x0001u;
constexpr uint16_t kSpawnRibbonStripFlagClosed = 0x0001u;
constexpr uint16_t kUpsertRibbonTrailFlagClosed = 0x0001u;
constexpr uint16_t kUpsertRibbonTrailFlagUseGroupLocalOrigin = 0x0002u;
constexpr uint16_t kUpsertQuadFieldFlagScreenBlend = 0x0001u;
constexpr uint16_t kUpsertQuadFieldFlagUseGroupLocalOrigin = 0x0002u;
constexpr uint16_t kUpsertGroupPresentationFlagVisible = 0x0001u;
constexpr uint16_t kUpsertGroupClipRectFlagEnabled = 0x0001u;
constexpr uint16_t kUpsertGroupLayerFlagBlendOverrideEnabled = 0x0001u;
constexpr uint16_t kUpsertGroupMaterialFlagTintEnabled = 0x0001u;

constexpr uint8_t kGroupClipMaskShapeRect = 0u;
constexpr uint8_t kGroupClipMaskShapeRoundRect = 1u;
constexpr uint8_t kGroupClipMaskShapeEllipse = 2u;
constexpr uint8_t kGroupMaterialStyleNone = 0u;
constexpr uint8_t kGroupMaterialStyleSoftBloomLike = 1u;
constexpr uint8_t kGroupMaterialStyleAfterimageLike = 2u;
constexpr uint8_t kGroupMaterialFeedbackModeDirectional = 0u;
constexpr uint8_t kGroupMaterialFeedbackModeTangential = 1u;
constexpr uint8_t kGroupMaterialFeedbackModeSwirl = 2u;
constexpr uint8_t kGroupPassKindNone = 0u;
constexpr uint8_t kGroupPassKindSoftBloomLike = 1u;
constexpr uint8_t kGroupPassKindAfterimageLike = 2u;
constexpr uint8_t kGroupPassKindEchoLike = 3u;
constexpr uint8_t kGroupPassModeDirectional = 0u;
constexpr uint8_t kGroupPassModeTangential = 1u;
constexpr uint8_t kGroupPassModeSwirl = 2u;
constexpr uint8_t kGroupPassBlendModeMultiply = 0u;
constexpr uint8_t kGroupPassBlendModeLerp = 1u;
constexpr uint8_t kGroupPassTemporalModeExponential = 0u;
constexpr uint8_t kGroupPassTemporalModeLinear = 1u;
constexpr uint8_t kGroupPassTemporalModePulse = 2u;
constexpr uint8_t kGroupPassRouteGlow = 0x01u;
constexpr uint8_t kGroupPassRouteSprite = 0x02u;
constexpr uint8_t kGroupPassRouteParticle = 0x04u;
constexpr uint8_t kGroupPassRouteRibbon = 0x08u;
constexpr uint8_t kGroupPassRouteQuad = 0x10u;
constexpr uint8_t kGroupPassRouteAll = 0x1Fu;
constexpr uint16_t kUpsertGlowEmitterFlagScreenBlend = 0x0001u;
constexpr uint16_t kUpsertGlowEmitterFlagUseGroupLocalOrigin = 0x0002u;
constexpr uint16_t kUpsertSpriteEmitterFlagScreenBlend = 0x0001u;
constexpr uint16_t kUpsertSpriteEmitterFlagUseGroupLocalOrigin = 0x0002u;
constexpr uint8_t kParticleEmitterStyleSoftGlow = 0u;
constexpr uint8_t kParticleEmitterStyleSolidDisc = 1u;
constexpr uint8_t kParticleEmitterEmissionModeCone = 0u;
constexpr uint8_t kParticleEmitterEmissionModeRadial = 1u;
constexpr uint8_t kParticleEmitterSpawnShapePoint = 0u;
constexpr uint8_t kParticleEmitterSpawnShapeDisc = 1u;
constexpr uint8_t kParticleEmitterSpawnShapeRing = 2u;
constexpr uint8_t kUpsertParticleEmitterFlagScreenBlend = 0x01u;
constexpr uint8_t kUpsertParticleEmitterFlagUseGroupLocalOrigin = 0x02u;
constexpr uint8_t kPathStrokeNodeOpMoveTo = 0u;
constexpr uint8_t kPathStrokeNodeOpLineTo = 1u;
constexpr uint8_t kPathStrokeNodeOpQuadTo = 2u;
constexpr uint8_t kPathStrokeNodeOpCubicTo = 3u;
constexpr uint8_t kPathStrokeNodeOpClose = 4u;
constexpr uint8_t kPathStrokeLineJoinMiter = 0u;
constexpr uint8_t kPathStrokeLineJoinRound = 1u;
constexpr uint8_t kPathStrokeLineJoinBevel = 2u;
constexpr uint8_t kPathStrokeLineCapButt = 0u;
constexpr uint8_t kPathStrokeLineCapRound = 1u;
constexpr uint8_t kPathStrokeLineCapSquare = 2u;
constexpr uint8_t kPathFillRuleNonZero = 0u;
constexpr uint8_t kPathFillRuleEvenOdd = 1u;
constexpr uint8_t kCommandBlendModeNormal = 0u;
constexpr uint8_t kCommandBlendModeScreen = 1u;
constexpr uint8_t kCommandBlendModeAdd = 2u;

#pragma pack(push, 1)

struct CommandHeaderV1 final {
    uint16_t kind = 0;
    uint16_t sizeBytes = 0;
};

struct SpawnTextCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    float alpha = 1.0f;
    uint32_t colorRgba = 0xFFFFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint32_t textId = 0;
};

struct SpawnImageCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    float alpha = 1.0f;
    uint32_t tintRgba = 0xFFFFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint32_t imageId = 0;
};

// Generic event input for mfx_plugin_on_input.
// - For Click: button is valid; delta/holdMs/flags are optional.
// - For Move: x/y are valid.
// - For Scroll: delta + flags(horizontal bit) are valid.
// - For Hold*: holdMs + button + x/y are valid.
// - For Hover*: x/y are valid.
struct EventInputV2 final {
    int32_t x = 0;
    int32_t y = 0;
    int32_t delta = 0;
    uint32_t holdMs = 0;
    uint8_t kind = static_cast<uint8_t>(InputEventKind::Click);
    uint8_t button = 0;
    uint8_t flags = 0;
    uint8_t reserved0 = 0;
    uint64_t eventTickMs = 0;
};

// Optional tail for indicator events (append after EventInputV2).
struct IndicatorEventTailV1 final {
    uint16_t sizePx = 0;
    uint16_t durationMs = 0;
};

// Optional indicator context tail (append after IndicatorEventTailV1 when available).
// - Click: primaryCode mirrors button, streak carries click streak.
// - Scroll: primaryCode carries absolute wheel magnitude, streak carries wheel streak.
// - Key: primaryCode carries vkCode, modifierMask carries Ctrl/Shift/Alt/Meta bits,
//        detailFlags exposes key-specific flags like systemKey.
struct IndicatorEventContextTailV2 final {
    uint32_t primaryCode = 0;
    uint16_t streak = 0;
    uint8_t modifierMask = 0;
    uint8_t detailFlags = 0;
};

// Frame tick input for mfx_plugin_on_frame.
// - cursorX/cursorY are valid only when pointerValid != 0.
// - holdActive is non-zero while hold button is down.
struct FrameInputV2 final {
    int32_t cursorX = 0;
    int32_t cursorY = 0;
    uint32_t frameDeltaMs = 0;
    uint8_t pointerValid = 0;
    uint8_t holdActive = 0;
    uint8_t reserved0 = 0;
    uint8_t reserved1 = 0;
    uint64_t frameTickMs = 0;
};

// Forward-compatible image command variant that keeps SpawnImageCommandV1 as prefix
// and appends affine transform metadata.
struct SpawnImageAffineCommandV1 final {
    SpawnImageCommandV1 base{};
    float affineM11 = 1.0f;
    float affineM12 = 0.0f;
    float affineM21 = 0.0f;
    float affineM22 = 1.0f;
    float affineDx = 0.0f;
    float affineDy = 0.0f;
    uint32_t affineAnchorMode = 0;
    uint32_t affineEnabled = 0;
};

struct SpawnPulseCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float startRadiusPx = 0.0f;
    float endRadiusPx = 40.0f;
    float strokeWidthPx = 2.5f;
    float alpha = 1.0f;
    uint32_t fillArgb = 0x594FC3F7u;
    uint32_t strokeArgb = 0xFF0288D1u;
    uint32_t glowArgb = 0x660288D1u;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint32_t pulseKind = static_cast<uint32_t>(SpawnPulseKind::Ripple);
};

struct PolylinePointV1 final {
    float x = 0.0f;
    float y = 0.0f;
};

struct SpawnPolylineCommandV1 final {
    CommandHeaderV1 header{};
    float lineWidthPx = 4.0f;
    float alpha = 1.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t pointCount = 0;
    uint16_t flags = 0;
};

struct SpawnPathStrokeCommandV1 final {
    CommandHeaderV1 header{};
    float lineWidthPx = 4.0f;
    float alpha = 1.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t nodeCount = 0;
    uint8_t lineJoin = kPathStrokeLineJoinRound;
    uint8_t lineCap = kPathStrokeLineCapRound;
};

struct SpawnPathFillCommandV1 final {
    CommandHeaderV1 header{};
    float alpha = 1.0f;
    float glowWidthPx = 10.0f;
    uint32_t fillArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t nodeCount = 0;
    uint8_t fillRule = kPathFillRuleNonZero;
    uint8_t flags = 0u;
};

struct PathStrokeNodeV1 final {
    uint8_t opcode = kPathStrokeNodeOpMoveTo;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    float x3 = 0.0f;
    float y3 = 0.0f;
};

struct GlowBatchItemV1 final {
    float x = 0.0f;
    float y = 0.0f;
    float radiusPx = 6.0f;
    float alpha = 1.0f;
    uint32_t colorArgb = 0xFFFFFFFFu;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
};

struct SpawnGlowBatchCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t itemCount = 0;
    uint16_t flags = 0;
};

struct SpriteBatchItemV1 final {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    float alpha = 1.0f;
    uint32_t tintArgb = 0xFFFFFFFFu;
    uint32_t imageId = 0;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
};

struct SpawnSpriteBatchCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t itemCount = 0;
    uint16_t flags = 0;
};

struct QuadBatchItemV1 final {
    float x = 0.0f;
    float y = 0.0f;
    float widthPx = 64.0f;
    float heightPx = 64.0f;
    float alpha = 1.0f;
    float rotation = 0.0f;
    uint32_t tintArgb = 0xFFFFFFFFu;
    uint32_t imageId = 0u;
    float srcU0 = 0.0f;
    float srcV0 = 0.0f;
    float srcU1 = 1.0f;
    float srcV1 = 1.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
};

struct SpawnQuadBatchCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint16_t itemCount = 0;
    uint16_t flags = 0;
};

struct RibbonStripPointV1 final {
    float x = 0.0f;
    float y = 0.0f;
    float widthPx = 12.0f;
};

struct SpawnRibbonStripCommandV1 final {
    CommandHeaderV1 header{};
    float alpha = 1.0f;
    float glowWidthPx = 8.0f;
    uint32_t fillArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0u;
    uint32_t lifeMs = 0u;
    uint16_t pointCount = 0u;
    uint16_t flags = 0u;
};

struct UpsertRibbonTrailCommandV1 final {
    CommandHeaderV1 header{};
    float alpha = 1.0f;
    float glowWidthPx = 8.0f;
    uint32_t fillArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t ttlMs = 640u;
    uint32_t trailId = 1u;
    uint16_t pointCount = 0u;
    uint16_t flags = 0u;
};

struct RemoveRibbonTrailCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t trailId = 0u;
};

struct UpsertQuadFieldCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t ttlMs = 640u;
    uint32_t fieldId = 1u;
    uint16_t itemCount = 0u;
    uint16_t flags = 0u;
};

struct RemoveQuadFieldCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t fieldId = 0u;
};

struct RemoveGroupCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t groupId = 0u;
};

struct UpsertGroupPresentationCommandV1 final {
    CommandHeaderV1 header{};
    float alphaMultiplier = 1.0f;
    uint32_t groupId = 0u;
    uint16_t flags = kUpsertGroupPresentationFlagVisible;
    uint16_t reserved0 = 0u;
};

struct UpsertGroupClipRectCommandV1 final {
    CommandHeaderV1 header{};
    float leftPx = 0.0f;
    float topPx = 0.0f;
    float widthPx = 0.0f;
    float heightPx = 0.0f;
    uint32_t groupId = 0u;
    uint16_t flags = 0u;
    uint16_t reserved0 = 0u;
};

struct GroupClipMaskTailV1 final {
    uint8_t shapeKind = kGroupClipMaskShapeRect;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float cornerRadiusPx = 0.0f;
};

struct UpsertGroupLayerCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t groupId = 0u;
    uint16_t flags = 0u;
    uint8_t blendMode = kCommandBlendModeNormal;
    uint8_t reserved0 = 0u;
    int32_t sortBias = 0;
};

struct UpsertGroupTransformCommandV1 final {
    CommandHeaderV1 header{};
    float offsetXPx = 0.0f;
    float offsetYPx = 0.0f;
    uint32_t groupId = 0u;
};

struct GroupTransformTailV1 final {
    float rotationRad = 0.0f;
    float uniformScale = 1.0f;
};

struct GroupTransformPivotTailV1 final {
    float pivotXPx = 0.0f;
    float pivotYPx = 0.0f;
};

struct GroupTransformScale2DTailV1 final {
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

struct UpsertGroupLocalOriginCommandV1 final {
    CommandHeaderV1 header{};
    float originXPx = 0.0f;
    float originYPx = 0.0f;
    uint32_t groupId = 0u;
};

struct UpsertGroupMaterialCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t groupId = 0u;
    uint32_t tintArgb = 0xFFFFFFFFu;
    float intensityMultiplier = 1.0f;
    uint16_t flags = 0u;
    uint16_t reserved0 = 0u;
};

struct GroupMaterialStyleTailV1 final {
    uint8_t styleKind = kGroupMaterialStyleNone;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float styleAmount = 0.0f;
};

struct GroupMaterialResponseTailV1 final {
    float diffusionAmount = 0.0f;
    float persistenceAmount = 0.0f;
};

struct GroupMaterialFeedbackTailV1 final {
    float echoAmount = 0.0f;
    float echoDriftPx = 0.0f;
};

struct GroupMaterialFeedbackModeTailV1 final {
    uint8_t feedbackMode = kGroupMaterialFeedbackModeDirectional;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float phaseRad = 0.0f;
};

struct GroupMaterialFeedbackStackTailV1 final {
    uint8_t layerCount = 1u;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float layerFalloff = 0.5f;
};

struct UpsertGroupPassCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t groupId = 0u;
    uint8_t passKind = kGroupPassKindNone;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float passAmount = 0.0f;
    float responseAmount = 0.0f;
};

struct GroupPassModeTailV1 final {
    uint8_t passMode = kGroupPassModeDirectional;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float phaseRad = 0.0f;
};

struct GroupPassStackTailV1 final {
    uint8_t layerCount = 1u;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float layerFalloff = 0.5f;
};

struct GroupPassPipelineTailV1 final {
    uint8_t secondaryPassKind = kGroupPassKindNone;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float secondaryPassAmount = 0.0f;
    float secondaryResponseAmount = 0.0f;
};

struct GroupPassBlendTailV1 final {
    uint8_t blendMode = kGroupPassBlendModeMultiply;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float blendWeight = 1.0f;
};

struct GroupPassRoutingTailV1 final {
    uint8_t routeMask = kGroupPassRouteAll;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    uint32_t reserved2 = 0u;
};

struct GroupPassLaneResponseTailV1 final {
    float glowResponse = 1.0f;
    float spriteResponse = 1.0f;
    float particleResponse = 1.0f;
    float ribbonResponse = 1.0f;
    float quadResponse = 1.0f;
};

struct GroupPassTemporalTailV1 final {
    float phaseRateRadPerSec = 0.0f;
    float secondaryDecayPerSec = 0.0f;
    float secondaryDecayFloor = 1.0f;
};

struct GroupPassTemporalModeTailV1 final {
    uint8_t temporalMode = kGroupPassTemporalModeExponential;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float temporalStrength = 1.0f;
};

struct GroupPassTertiaryTailV1 final {
    uint8_t tertiaryPassKind = kGroupPassKindNone;
    uint8_t tertiaryBlendMode = kGroupPassBlendModeMultiply;
    uint16_t reserved0 = 0u;
    float tertiaryPassAmount = 0.0f;
    float tertiaryResponseAmount = 0.0f;
    float tertiaryBlendWeight = 1.0f;
};

struct GroupPassTertiaryRoutingTailV1 final {
    uint8_t tertiaryRouteMask = kGroupPassRouteAll;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    uint32_t reserved2 = 0u;
};

struct GroupPassTertiaryLaneResponseTailV1 final {
    float glowResponse = 1.0f;
    float spriteResponse = 1.0f;
    float particleResponse = 1.0f;
    float ribbonResponse = 1.0f;
    float quadResponse = 1.0f;
};

struct GroupPassTertiaryTemporalTailV1 final {
    float phaseRateRadPerSec = 0.0f;
    float tertiaryDecayPerSec = 0.0f;
    float tertiaryDecayFloor = 1.0f;
};

struct GroupPassTertiaryTemporalModeTailV1 final {
    uint8_t temporalMode = kGroupPassTemporalModeExponential;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float temporalStrength = 1.0f;
};

struct GroupPassTertiaryStackTailV1 final {
    uint8_t layerCount = 1u;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    float layerFalloff = 0.5f;
};

struct UpsertGlowEmitterCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float emissionRatePerSec = 96.0f;
    float directionRad = 0.0f;
    float spreadRad = 1.0471976f;
    float speedMin = 140.0f;
    float speedMax = 260.0f;
    float radiusMinPx = 3.0f;
    float radiusMaxPx = 9.0f;
    float alphaMin = 0.18f;
    float alphaMax = 0.90f;
    uint32_t colorArgb = 0xFFFFD54Fu;
    float accelerationX = 0.0f;
    float accelerationY = 220.0f;
    uint32_t emitterId = 1u;
    uint32_t emitterTtlMs = 420u;
    uint32_t particleLifeMs = 900u;
    uint16_t maxParticles = 160u;
    uint16_t flags = 0u;
};

struct RemoveGlowEmitterCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t emitterId = 0u;
};

struct UpsertSpriteEmitterCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float emissionRatePerSec = 84.0f;
    float directionRad = 0.0f;
    float spreadRad = 1.0471976f;
    float speedMin = 120.0f;
    float speedMax = 240.0f;
    float sizeMinPx = 24.0f;
    float sizeMaxPx = 72.0f;
    float alphaMin = 0.20f;
    float alphaMax = 0.92f;
    uint32_t tintArgb = 0u;
    float rotationMinRad = -0.35f;
    float rotationMaxRad = 0.35f;
    float accelerationX = 0.0f;
    float accelerationY = 120.0f;
    uint32_t emitterId = 1u;
    uint32_t emitterTtlMs = 520u;
    uint32_t particleLifeMs = 920u;
    uint32_t imageId = 0u;
    uint16_t maxParticles = 120u;
    uint16_t flags = 0u;
};

struct RemoveSpriteEmitterCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t emitterId = 0u;
};

struct UpsertParticleEmitterCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float emissionRatePerSec = 96.0f;
    float directionRad = 0.0f;
    float spreadRad = 1.0471976f;
    float speedMin = 140.0f;
    float speedMax = 260.0f;
    float radiusMinPx = 3.0f;
    float radiusMaxPx = 9.0f;
    float alphaMin = 0.18f;
    float alphaMax = 0.90f;
    uint32_t colorArgb = 0xFFFFD54Fu;
    float accelerationX = 0.0f;
    float accelerationY = 220.0f;
    uint32_t emitterId = 1u;
    uint32_t emitterTtlMs = 420u;
    uint32_t particleLifeMs = 900u;
    uint16_t maxParticles = 160u;
    uint8_t particleStyle = kParticleEmitterStyleSoftGlow;
    uint8_t flags = 0u;
};

struct RemoveParticleEmitterCommandV1 final {
    CommandHeaderV1 header{};
    uint32_t emitterId = 0u;
};

struct ParticleEmitterSpawnTailV1 final {
    uint8_t emissionMode = kParticleEmitterEmissionModeCone;
    uint8_t spawnShape = kParticleEmitterSpawnShapePoint;
    uint16_t reserved0 = 0u;
    float spawnRadiusX = 0.0f;
    float spawnRadiusY = 0.0f;
    float spawnInnerRatio = 0.0f;
};

struct ParticleEmitterDynamicsTailV1 final {
    float dragPerSecond = 0.0f;
    float turbulenceAccel = 0.0f;
    float turbulenceFrequencyHz = 0.0f;
    float turbulencePhaseJitter = 1.0f;
};

struct ParticleEmitterLifeTailV1 final {
    float sizeStartScale = 1.0f;
    float sizeEndScale = 1.0f;
    float alphaStartScale = 1.0f;
    float alphaEndScale = 1.0f;
    uint32_t colorStartArgb = 0xFFFFFFFFu;
    uint32_t colorEndArgb = 0xFFFFFFFFu;
};

struct CommandRenderSemanticsTailV1 final {
    uint8_t blendMode = kCommandBlendModeNormal;
    uint8_t reserved0 = 0u;
    uint16_t reserved1 = 0u;
    int32_t sortKey = 0;
    uint32_t groupId = 0u;
};

struct CommandClipRectTailV1 final {
    float leftPx = 0.0f;
    float topPx = 0.0f;
    float widthPx = 0.0f;
    float heightPx = 0.0f;
};

#pragma pack(pop)

static_assert(sizeof(EventInputV2) == 28, "EventInputV2 layout drifted.");
static_assert(sizeof(IndicatorEventTailV1) == 4, "IndicatorEventTailV1 layout drifted.");
static_assert(sizeof(IndicatorEventContextTailV2) == 8, "IndicatorEventContextTailV2 layout drifted.");
static_assert(sizeof(FrameInputV2) == 24, "FrameInputV2 layout drifted.");
static_assert(sizeof(CommandHeaderV1) == 4, "CommandHeaderV1 layout drifted.");
static_assert(sizeof(SpawnImageAffineCommandV1) == 88, "SpawnImageAffineCommandV1 layout drifted.");
static_assert(sizeof(SpawnPulseCommandV1) == 52, "SpawnPulseCommandV1 layout drifted.");
static_assert(sizeof(PolylinePointV1) == 8, "PolylinePointV1 layout drifted.");
static_assert(sizeof(SpawnPolylineCommandV1) == 32, "SpawnPolylineCommandV1 layout drifted.");
static_assert(sizeof(SpawnPathStrokeCommandV1) == 32, "SpawnPathStrokeCommandV1 layout drifted.");
static_assert(sizeof(SpawnPathFillCommandV1) == 32, "SpawnPathFillCommandV1 layout drifted.");
static_assert(sizeof(PathStrokeNodeV1) == 28, "PathStrokeNodeV1 layout drifted.");
static_assert(sizeof(GlowBatchItemV1) == 36, "GlowBatchItemV1 layout drifted.");
static_assert(sizeof(SpawnGlowBatchCommandV1) == 16, "SpawnGlowBatchCommandV1 layout drifted.");
static_assert(sizeof(SpriteBatchItemV1) == 44, "SpriteBatchItemV1 layout drifted.");
static_assert(sizeof(SpawnSpriteBatchCommandV1) == 16, "SpawnSpriteBatchCommandV1 layout drifted.");
static_assert(sizeof(QuadBatchItemV1) == 64, "QuadBatchItemV1 layout drifted.");
static_assert(sizeof(SpawnQuadBatchCommandV1) == 16, "SpawnQuadBatchCommandV1 layout drifted.");
static_assert(sizeof(RibbonStripPointV1) == 12, "RibbonStripPointV1 layout drifted.");
static_assert(sizeof(SpawnRibbonStripCommandV1) == 32, "SpawnRibbonStripCommandV1 layout drifted.");
static_assert(sizeof(UpsertRibbonTrailCommandV1) == 32, "UpsertRibbonTrailCommandV1 layout drifted.");
static_assert(sizeof(RemoveRibbonTrailCommandV1) == 8, "RemoveRibbonTrailCommandV1 layout drifted.");
static_assert(sizeof(UpsertQuadFieldCommandV1) == 16, "UpsertQuadFieldCommandV1 layout drifted.");
static_assert(sizeof(RemoveQuadFieldCommandV1) == 8, "RemoveQuadFieldCommandV1 layout drifted.");
static_assert(sizeof(RemoveGroupCommandV1) == 8, "RemoveGroupCommandV1 layout drifted.");
static_assert(sizeof(UpsertGroupPresentationCommandV1) == 16, "UpsertGroupPresentationCommandV1 layout drifted.");
static_assert(sizeof(UpsertGroupClipRectCommandV1) == 28, "UpsertGroupClipRectCommandV1 layout drifted.");
static_assert(sizeof(GroupClipMaskTailV1) == 8, "GroupClipMaskTailV1 layout drifted.");
static_assert(sizeof(UpsertGroupLayerCommandV1) == 16, "UpsertGroupLayerCommandV1 layout drifted.");
static_assert(sizeof(UpsertGroupTransformCommandV1) == 16, "UpsertGroupTransformCommandV1 layout drifted.");
static_assert(sizeof(GroupTransformTailV1) == 8, "GroupTransformTailV1 layout drifted.");
static_assert(sizeof(GroupTransformPivotTailV1) == 8, "GroupTransformPivotTailV1 layout drifted.");
static_assert(sizeof(GroupTransformScale2DTailV1) == 8, "GroupTransformScale2DTailV1 layout drifted.");
static_assert(sizeof(UpsertGroupLocalOriginCommandV1) == 16, "UpsertGroupLocalOriginCommandV1 layout drifted.");
static_assert(sizeof(UpsertGroupMaterialCommandV1) == 20, "UpsertGroupMaterialCommandV1 layout drifted.");
static_assert(sizeof(GroupMaterialStyleTailV1) == 8, "GroupMaterialStyleTailV1 layout drifted.");
static_assert(sizeof(GroupMaterialResponseTailV1) == 8, "GroupMaterialResponseTailV1 layout drifted.");
static_assert(sizeof(GroupMaterialFeedbackTailV1) == 8, "GroupMaterialFeedbackTailV1 layout drifted.");
static_assert(sizeof(GroupMaterialFeedbackModeTailV1) == 8, "GroupMaterialFeedbackModeTailV1 layout drifted.");
static_assert(sizeof(GroupMaterialFeedbackStackTailV1) == 8, "GroupMaterialFeedbackStackTailV1 layout drifted.");
static_assert(sizeof(UpsertGroupPassCommandV1) == 20, "UpsertGroupPassCommandV1 layout drifted.");
static_assert(sizeof(GroupPassModeTailV1) == 8, "GroupPassModeTailV1 layout drifted.");
static_assert(sizeof(GroupPassStackTailV1) == 8, "GroupPassStackTailV1 layout drifted.");
static_assert(sizeof(GroupPassPipelineTailV1) == 12, "GroupPassPipelineTailV1 layout drifted.");
static_assert(sizeof(GroupPassBlendTailV1) == 8, "GroupPassBlendTailV1 layout drifted.");
static_assert(sizeof(GroupPassRoutingTailV1) == 8, "GroupPassRoutingTailV1 layout drifted.");
static_assert(sizeof(GroupPassLaneResponseTailV1) == 20, "GroupPassLaneResponseTailV1 layout drifted.");
static_assert(sizeof(GroupPassTemporalTailV1) == 12, "GroupPassTemporalTailV1 layout drifted.");
static_assert(sizeof(GroupPassTemporalModeTailV1) == 8, "GroupPassTemporalModeTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryTailV1) == 16, "GroupPassTertiaryTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryRoutingTailV1) == 8, "GroupPassTertiaryRoutingTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryLaneResponseTailV1) == 20, "GroupPassTertiaryLaneResponseTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryTemporalTailV1) == 12, "GroupPassTertiaryTemporalTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryTemporalModeTailV1) == 8, "GroupPassTertiaryTemporalModeTailV1 layout drifted.");
static_assert(sizeof(GroupPassTertiaryStackTailV1) == 8, "GroupPassTertiaryStackTailV1 layout drifted.");
static_assert(sizeof(UpsertGlowEmitterCommandV1) == 76, "UpsertGlowEmitterCommandV1 layout drifted.");
static_assert(sizeof(RemoveGlowEmitterCommandV1) == 8, "RemoveGlowEmitterCommandV1 layout drifted.");
static_assert(sizeof(UpsertSpriteEmitterCommandV1) == 88, "UpsertSpriteEmitterCommandV1 layout drifted.");
static_assert(sizeof(RemoveSpriteEmitterCommandV1) == 8, "RemoveSpriteEmitterCommandV1 layout drifted.");
static_assert(sizeof(UpsertParticleEmitterCommandV1) == 76, "UpsertParticleEmitterCommandV1 layout drifted.");
static_assert(sizeof(RemoveParticleEmitterCommandV1) == 8, "RemoveParticleEmitterCommandV1 layout drifted.");
static_assert(sizeof(ParticleEmitterSpawnTailV1) == 16, "ParticleEmitterSpawnTailV1 layout drifted.");
static_assert(sizeof(ParticleEmitterDynamicsTailV1) == 16, "ParticleEmitterDynamicsTailV1 layout drifted.");
static_assert(sizeof(ParticleEmitterLifeTailV1) == 24, "ParticleEmitterLifeTailV1 layout drifted.");
static_assert(sizeof(CommandRenderSemanticsTailV1) == 12, "CommandRenderSemanticsTailV1 layout drifted.");
static_assert(sizeof(CommandClipRectTailV1) == 16, "CommandClipRectTailV1 layout drifted.");

} // namespace mousefx::wasm
