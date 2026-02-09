#pragma once

#include <windows.h>
#include <cstdint>
#include <memory>
#include <string>

#include "MouseFx/Core/GlobalMouseHook.h"
#include "MouseFx/Styles/RippleStyle.h"

namespace mousefx {

class ITrailRenderer;
class IRippleRenderer;
class IOverlayLayer;
class OverlayHostWindow;
class TrailOverlayLayer;
class ParticleTrailOverlayLayer;
class RippleOverlayLayer;
class TextOverlayLayer;
struct RenderParams;
struct TextConfig;

class OverlayHostService final {
public:
    static OverlayHostService& Instance();

    // Preferred backend: auto | dawn | cpu.
    // Runtime will always fall back to cpu when gpu backend is unavailable.
    void SetRenderBackendPreference(const std::string& backend);
    std::string GetRenderBackendPreference() const;
    std::string GetActiveRenderBackend() const;
    std::string GetRenderBackendDetail() const;
    std::string GetRenderPipelineMode() const;
    void SetGpuBridgeModeRequest(const std::string& mode);
    std::string GetGpuBridgeModeRequest() const;
    bool HasGpuHardware() const;
    bool IsGpuBackendAvailable(const std::string& backend) const;
    void RefreshGpuRuntimeProbe();
    std::string ProbeDawnRuntimeNow(bool refreshProbe);

    bool Initialize();
    void Shutdown();

    IOverlayLayer* AttachLayer(std::unique_ptr<IOverlayLayer> layer);
    TrailOverlayLayer* AttachTrailLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, bool isChromatic);
    ParticleTrailOverlayLayer* AttachParticleTrailLayer(bool isChromatic);
    uint64_t ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    uint64_t ShowContinuousRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    void UpdateRipplePosition(uint64_t id, const POINT& pt);
    void StopRipple(uint64_t id);
    bool IsRippleActive(uint64_t id) const;
    void UpdateRippleHoldElapsed(uint64_t id, uint32_t holdMs);
    void UpdateRippleHoldThreshold(uint64_t id, uint32_t thresholdMs);
    void SendRippleCommand(uint64_t id, const std::string& cmd, const std::string& args);
    void BroadcastRippleCommand(const std::string& cmd, const std::string& args);
    bool ShowText(const POINT& pt, const std::wstring& text, Argb color, const TextConfig& config);
    void DetachLayer(IOverlayLayer* layer);

private:
    OverlayHostService() = default;
    ~OverlayHostService() = default;

    OverlayHostService(const OverlayHostService&) = delete;
    OverlayHostService& operator=(const OverlayHostService&) = delete;

    std::unique_ptr<OverlayHostWindow> host_{};
    RippleOverlayLayer* rippleLayer_ = nullptr;
    TextOverlayLayer* textLayer_ = nullptr;
    std::string requestedBackend_ = "auto";
    std::string requestedBridgeMode_ = "host_compat";
    std::string activeBackend_ = "cpu";
    std::string backendDetail_ = "cpu_default";
    std::string pipelineMode_ = "cpu_layered";

    RippleOverlayLayer* EnsureRippleLayer();
    TextOverlayLayer* EnsureTextLayer();
    static std::string NormalizeRenderBackend(std::string backend);
    static std::string NormalizeGpuBridgeMode(std::string mode);
};

} // namespace mousefx
