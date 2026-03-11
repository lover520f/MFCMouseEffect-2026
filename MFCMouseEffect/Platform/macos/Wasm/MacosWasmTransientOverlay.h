#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Interfaces/RenderSemantics.h"

namespace mousefx::platform::macos {

enum class WasmOverlayRenderResult : uint8_t {
    Rendered = 0,
    ThrottledByCapacity = 1,
    ThrottledByInterval = 2,
    Failed = 3,
};

struct WasmImageOverlayRequest final {
    ScreenPoint screenPt{};
    std::wstring assetPath{};
    uint32_t tintArgb = 0;
    float scale = 1.0f;
    float alpha = 1.0f;
    uint32_t lifeMs = 0;
    uint32_t delayMs = 0;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
    float rotationRad = 0.0f;
    bool applyTint = false;
};

struct WasmPulseOverlayRequest final {
    ScreenPoint screenPt{};
    std::string normalizedType{"ripple"};
    int sizePx = 138;
    float alpha = 1.0f;
    float startRadiusPx = 0.0f;
    float endRadiusPx = 40.0f;
    float strokeWidthPx = 2.5f;
    uint32_t fillArgb = 0x594FC3F7u;
    uint32_t strokeArgb = 0xFF0288D1u;
    uint32_t glowArgb = 0x660288D1u;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
};

struct WasmPolylineOverlayRequest final {
    int frameLeftPx = 0;
    int frameTopPx = 0;
    int squareSizePx = 64;
    std::vector<float> localPointsXY{};
    float lineWidthPx = 4.0f;
    float alpha = 1.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    bool closed = false;
};

struct WasmPathStrokeOverlayNode final {
    uint8_t opcode = 0u;
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    float x3 = 0.0f;
    float y3 = 0.0f;
};

struct WasmPathStrokeOverlayRequest final {
    int frameLeftPx = 0;
    int frameTopPx = 0;
    int squareSizePx = 64;
    std::vector<WasmPathStrokeOverlayNode> nodes{};
    float lineWidthPx = 4.0f;
    float alpha = 1.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint8_t lineJoin = 0u;
    uint8_t lineCap = 0u;
    RenderSemantics semantics{};
};

struct WasmPathFillOverlayRequest final {
    int frameLeftPx = 0;
    int frameTopPx = 0;
    int squareSizePx = 64;
    std::vector<WasmPathStrokeOverlayNode> nodes{};
    float alpha = 1.0f;
    float glowWidthPx = 10.0f;
    uint32_t fillArgb = 0xFFFFFFFFu;
    uint32_t glowArgb = 0x66FFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint8_t fillRule = 0u;
    RenderSemantics semantics{};
};

struct WasmGlowBatchOverlayParticle final {
    float localX = 0.0f;
    float localY = 0.0f;
    float radiusPx = 6.0f;
    float alpha = 1.0f;
    uint32_t colorArgb = 0xFFFFFFFFu;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
};

struct WasmGlowBatchOverlayRequest final {
    int frameLeftPx = 0;
    int frameTopPx = 0;
    int squareSizePx = 64;
    std::vector<WasmGlowBatchOverlayParticle> particles{};
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    RenderSemantics semantics{};
};

struct WasmSpriteBatchOverlaySprite final {
    std::wstring assetPath{};
    float localX = 0.0f;
    float localY = 0.0f;
    float widthPx = 24.0f;
    float heightPx = 24.0f;
    float alpha = 1.0f;
    float rotationRad = 0.0f;
    uint32_t tintArgb = 0xFFFFFFFFu;
    bool applyTint = false;
    float srcU0 = 0.0f;
    float srcV0 = 0.0f;
    float srcU1 = 1.0f;
    float srcV1 = 1.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
};

struct WasmSpriteBatchOverlayRequest final {
    int frameLeftPx = 0;
    int frameTopPx = 0;
    int squareSizePx = 64;
    std::vector<WasmSpriteBatchOverlaySprite> sprites{};
    uint32_t delayMs = 0;
    uint32_t lifeMs = 360;
    RenderSemantics semantics{};
};

enum class WasmTextOverlayChannel : uint8_t {
    Effects = 0,
    IndicatorLabel = 1,
};

WasmOverlayRenderResult ShowWasmImageOverlay(const WasmImageOverlayRequest& request);
WasmOverlayRenderResult ShowWasmPulseOverlay(const WasmPulseOverlayRequest& request);
WasmOverlayRenderResult ShowWasmPolylineOverlay(const WasmPolylineOverlayRequest& request);
WasmOverlayRenderResult ShowWasmPathStrokeOverlay(const WasmPathStrokeOverlayRequest& request);
WasmOverlayRenderResult ShowWasmPathFillOverlay(const WasmPathFillOverlayRequest& request);
WasmOverlayRenderResult ShowWasmGlowBatchOverlay(const WasmGlowBatchOverlayRequest& request);
WasmOverlayRenderResult ShowWasmSpriteBatchOverlay(const WasmSpriteBatchOverlayRequest& request);

WasmOverlayRenderResult ShowWasmImagePulseOverlay(
    const ScreenPoint& screenPt,
    uint32_t tintArgb,
    float scale,
    float alpha,
    uint32_t lifeMs);

WasmOverlayRenderResult ShowWasmTextOverlay(
    const ScreenPoint& screenPt,
    const std::wstring& text,
    uint32_t argb,
    const TextConfig& textConfig,
    WasmTextOverlayChannel channel = WasmTextOverlayChannel::Effects);

void CloseAllWasmOverlays();

} // namespace mousefx::platform::macos
