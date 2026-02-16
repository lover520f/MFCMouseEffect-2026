#pragma once

#include <windows.h>

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
    GestureRecognizer() = default;

    void UpdateConfig(const GestureRecognitionConfig& config);
    void Reset();
    void OnButtonDown(const POINT& pt, int button);
    void OnMouseMove(const POINT& pt);
    std::string OnButtonUp(const POINT& pt, int button);

private:
    static std::string NormalizeButtonName(std::string button);
    static bool IsTrackedButton(const std::string& triggerButton, int button);
    static std::string BuildGestureId(const std::vector<char>& dirs);

    std::vector<char> QuantizeDirections() const;
    long long DistanceSquared(const POINT& a, const POINT& b) const;

    GestureRecognitionConfig config_{};
    bool active_ = false;
    int activeButton_ = 0;
    int totalDistancePx_ = 0;
    POINT lastRawPt_{};
    POINT lastSamplePt_{};
    std::vector<POINT> samples_{};
};

} // namespace mousefx
