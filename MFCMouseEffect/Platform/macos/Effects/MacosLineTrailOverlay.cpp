#include "pch.h"

#include "Platform/macos/Effects/MacosLineTrailOverlay.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosLineTrailOverlaySwiftBridge.h"

#include <algorithm>
#include <mutex>

namespace mousefx::macos_line_trail {

#if defined(__APPLE__)
namespace {

class LineTrailBridgeState final {
public:
    ~LineTrailBridgeState() {
        Release();
    }

    void Update(const ScreenPoint& overlayPt, const LineTrailConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        EnsureHandleLocked();
        if (handle_ == nullptr) {
            return;
        }

        const int durationMs = std::max(1, config.durationMs);
        const float lineWidth = std::max(0.2f, config.lineWidth);
        const int idleFadeStartMs = std::max(0, config.idleFade.startMs);
        const int idleFadeEndMs = std::max(idleFadeStartMs + 1, config.idleFade.endMs);

        mfx_macos_line_trail_update_v1(
            handle_,
            overlayPt.x,
            overlayPt.y,
            durationMs,
            lineWidth,
            config.strokeArgb,
            idleFadeStartMs,
            idleFadeEndMs);
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (handle_ == nullptr) {
            return;
        }
        mfx_macos_line_trail_reset_v1(handle_);
    }

    LineTrailRuntimeSnapshot Snapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        LineTrailRuntimeSnapshot snapshot{};
        if (handle_ == nullptr) {
            return snapshot;
        }
        snapshot.active = (mfx_macos_line_trail_is_active_v1(handle_) != 0);
        snapshot.pointCount = std::max(0, mfx_macos_line_trail_point_count_v1(handle_));
        return snapshot;
    }

private:
    void EnsureHandleLocked() {
        if (handle_ != nullptr) {
            return;
        }
        handle_ = mfx_macos_line_trail_create_v1();
    }

    void Release() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (handle_ == nullptr) {
            return;
        }
        mfx_macos_line_trail_release_v1(handle_);
        handle_ = nullptr;
    }

    std::mutex mutex_{};
    void* handle_ = nullptr;
};

LineTrailBridgeState& BridgeState() {
    static LineTrailBridgeState state{};
    return state;
}

} // namespace
#endif

void UpdateLineTrail(const ScreenPoint& screenPt, const LineTrailConfig& config) {
#if !defined(__APPLE__)
    (void)screenPt;
    (void)config;
#else
    const ScreenPoint overlayPt = ScreenToOverlayPoint(screenPt);
    BridgeState().Update(overlayPt, config);
#endif
}

void ResetLineTrail() {
#if defined(__APPLE__)
    BridgeState().Reset();
#endif
}

LineTrailRuntimeSnapshot ReadLineTrailRuntimeSnapshot() {
    LineTrailRuntimeSnapshot snapshot{};
#if defined(__APPLE__)
    snapshot = BridgeState().Snapshot();
#endif
    return snapshot;
}

} // namespace mousefx::macos_line_trail
