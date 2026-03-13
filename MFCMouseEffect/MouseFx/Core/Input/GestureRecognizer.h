#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace mousefx {

struct GestureRecognitionConfig {
    bool enabled = false;
    std::string triggerButton = "right";
    int minStrokeDistancePx = 80;
    int sampleStepPx = 10;
    int maxDirections = 4;
};

// Recognizes a simple directional gesture during a mouse-button drag stroke.
// Output format: "left", "up_right", "down_left_up", etc.
class GestureRecognizer final {
public:
    struct Result {
        std::string gestureId;
        int button = 0;
        std::vector<ScreenPoint> samplePoints;
        std::vector<uint32_t> sampleTimesMs;
        std::vector<ScreenPoint> previewPoints;
    };

    GestureRecognizer() = default;

    void UpdateConfig(const GestureRecognitionConfig& config);
    void Reset();
    void OnButtonDown(const ScreenPoint& pt, int button);
    void OnMouseMove(const ScreenPoint& pt);
    Result OnButtonUp(const ScreenPoint& pt, int button);
    Result Snapshot() const;
    bool IsActive() const { return active_; }

private:
    static bool IsTrackedButton(int button);
    static std::string BuildGestureId(const std::vector<char>& dirs);

    std::vector<ScreenPoint> BuildEvaluationSamples() const;
    std::vector<ScreenPoint> BuildPreviewSamples() const;
    std::vector<uint32_t> BuildEvaluationSampleTimesMs() const;
    std::vector<char> QuantizeDirections() const;
    long long DistanceSquared(const ScreenPoint& a, const ScreenPoint& b) const;

    GestureRecognitionConfig config_{};
    bool active_ = false;
    int activeButton_ = 0;
    int totalDistancePx_ = 0;
    ScreenPoint lastRawPt_{};
    ScreenPoint lastSamplePt_{};
    std::vector<ScreenPoint> samples_{};
    std::vector<ScreenPoint> previewPoints_{};
    std::chrono::steady_clock::time_point startedAt_{};
    std::chrono::steady_clock::time_point lastRawAt_{};
    std::chrono::steady_clock::time_point lastSampleAt_{};
    std::vector<uint32_t> sampleTimesMs_{};
};

} // namespace mousefx
