#include "pch.h"

#include "WasmEventInvokeExecutor.h"

#include <memory>
#include <vector>

#include "MouseFx/Utils/StringUtils.h"
#include "WasmDynamicTextLabelScope.h"

namespace mousefx::wasm {

namespace {

IWasmCommandRenderer& DefaultCommandRenderer() {
    static std::unique_ptr<IWasmCommandRenderer> renderer = CreatePlatformWasmCommandRenderer();
    return *renderer;
}

} // namespace

EventDispatchExecutionResult InvokeEventAndRender(
    WasmEffectHost& host,
    const EventInvokeInput& input,
    const EffectConfig& config) {
    return InvokeEventAndRender(host, input, config, DefaultCommandRenderer());
}

EventDispatchExecutionResult InvokeEventAndRender(
    WasmEffectHost& host,
    const EventInvokeInput& input,
    const EffectConfig& config,
    IWasmCommandRenderer& renderer) {
    EventDispatchExecutionResult result{};
    result.routeActive = host.Enabled() && host.IsPluginLoaded();
    if (!result.routeActive) {
        return result;
    }

    std::unique_ptr<ScopedWasmDynamicTextLabel> scopedLabel;
    if (!input.dynamicTextLabelUtf8.empty()) {
        const bool fromIndicatorEvent =
            input.kind == EventKind::IndicatorClick ||
            input.kind == EventKind::IndicatorScroll ||
            input.kind == EventKind::IndicatorKey;
        scopedLabel = std::make_unique<ScopedWasmDynamicTextLabel>(
            Utf8ToWString(input.dynamicTextLabelUtf8),
            fromIndicatorEvent);
    }

    std::vector<uint8_t> commandBuffer;
    result.invokeOk = host.InvokeInput(input, &commandBuffer);
    if (result.invokeOk && !commandBuffer.empty()) {
        result.render = renderer.Execute(
            commandBuffer.data(),
            commandBuffer.size(),
            config,
            host.Diagnostics().activeManifestPath);
    }

    host.RecordRenderExecution(
        result.render.renderedAny,
        result.render.executedTextCommands,
        result.render.executedImageCommands,
        result.render.executedPulseCommands,
        result.render.executedPolylineCommands,
        result.render.executedPathStrokeCommands,
        result.render.executedPathFillCommands,
        result.render.executedGlowBatchCommands,
        result.render.executedSpriteBatchCommands,
        result.render.executedGlowEmitterCommands,
        result.render.executedGlowEmitterRemoveCommands,
        result.render.executedSpriteEmitterCommands,
        result.render.executedSpriteEmitterRemoveCommands,
        result.render.executedParticleEmitterCommands,
        result.render.executedParticleEmitterRemoveCommands,
        result.render.executedRibbonTrailCommands,
        result.render.executedRibbonTrailRemoveCommands,
        result.render.executedQuadFieldCommands,
        result.render.executedQuadFieldRemoveCommands,
        result.render.executedGroupRemoveCommands,
        result.render.executedGroupPresentationCommands,
        result.render.executedGroupClipRectCommands,
        result.render.executedGroupLayerCommands,
        result.render.executedGroupTransformCommands,
        result.render.executedGroupLocalOriginCommands,
        result.render.executedGroupMaterialCommands,
        result.render.executedGroupPassCommands,
        result.render.throttledCommands,
        result.render.throttledByCapacityCommands,
        result.render.throttledByIntervalCommands,
        result.render.droppedCommands,
        result.render.lastError);
    return result;
}

EventDispatchExecutionResult InvokeFrameAndRender(
    WasmEffectHost& host,
    const FrameInvokeInput& input,
    const EffectConfig& config) {
    return InvokeFrameAndRender(host, input, config, DefaultCommandRenderer());
}

EventDispatchExecutionResult InvokeFrameAndRender(
    WasmEffectHost& host,
    const FrameInvokeInput& input,
    const EffectConfig& config,
    IWasmCommandRenderer& renderer) {
    EventDispatchExecutionResult result{};
    result.routeActive = host.Enabled() && host.IsPluginLoaded();
    if (!result.routeActive) {
        return result;
    }

    std::vector<uint8_t> commandBuffer;
    result.invokeOk = host.InvokeFrame(input, &commandBuffer);
    if (result.invokeOk && !commandBuffer.empty()) {
        result.render = renderer.Execute(
            commandBuffer.data(),
            commandBuffer.size(),
            config,
            host.Diagnostics().activeManifestPath);
    }

    host.RecordRenderExecution(
        result.render.renderedAny,
        result.render.executedTextCommands,
        result.render.executedImageCommands,
        result.render.executedPulseCommands,
        result.render.executedPolylineCommands,
        result.render.executedPathStrokeCommands,
        result.render.executedPathFillCommands,
        result.render.executedGlowBatchCommands,
        result.render.executedSpriteBatchCommands,
        result.render.executedGlowEmitterCommands,
        result.render.executedGlowEmitterRemoveCommands,
        result.render.executedSpriteEmitterCommands,
        result.render.executedSpriteEmitterRemoveCommands,
        result.render.executedParticleEmitterCommands,
        result.render.executedParticleEmitterRemoveCommands,
        result.render.executedRibbonTrailCommands,
        result.render.executedRibbonTrailRemoveCommands,
        result.render.executedQuadFieldCommands,
        result.render.executedQuadFieldRemoveCommands,
        result.render.executedGroupRemoveCommands,
        result.render.executedGroupPresentationCommands,
        result.render.executedGroupClipRectCommands,
        result.render.executedGroupLayerCommands,
        result.render.executedGroupTransformCommands,
        result.render.executedGroupLocalOriginCommands,
        result.render.executedGroupMaterialCommands,
        result.render.executedGroupPassCommands,
        result.render.throttledCommands,
        result.render.throttledByCapacityCommands,
        result.render.throttledByIntervalCommands,
        result.render.droppedCommands,
        result.render.lastError);
    return result;
}

} // namespace mousefx::wasm
