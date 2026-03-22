#pragma once

#include <array>

#include <gdiplus.h>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererActionOverlay final {
    Gdiplus::Color accentColor{};
    bool clickRingVisible{false};
    Gdiplus::RectF clickRingRect{};
    float clickRingStrokeWidth{2.2f};
    float clickRingAlpha{210.0f};
    bool holdBandVisible{false};
    Gdiplus::RectF holdBandRect{};
    float holdBandAlpha{150.0f};
    bool scrollArcVisible{false};
    Gdiplus::RectF scrollArcRect{};
    float scrollArcStartDeg{0.0f};
    float scrollArcSweepDeg{0.0f};
    float scrollArcStrokeWidth{3.0f};
    float scrollArcAlpha{220.0f};
    bool dragLineVisible{false};
    Gdiplus::PointF dragLineStart{};
    Gdiplus::PointF dragLineEnd{};
    float dragLineStrokeWidth{2.4f};
    float dragLineAlpha{210.0f};
    bool followTrailVisible{false};
    std::array<Gdiplus::RectF, 3> followTrailRects{};
    float followTrailBaseAlpha{150.0f};
};

struct Win32MouseCompanionRealRendererScene final {
    float centerX{0.0f};
    float centerY{0.0f};
    float bodyTiltDeg{0.0f};
    float facingSign{1.0f};
    float glowAlpha{0.0f};
    Gdiplus::Color glowColor{};
    Gdiplus::Color bodyFill{};
    Gdiplus::Color bodyFillRear{};
    Gdiplus::Color bodyStroke{};
    Gdiplus::Color headFill{};
    Gdiplus::Color headFillRear{};
    Gdiplus::Color earFill{};
    Gdiplus::Color earFillRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color eyeFill{};
    Gdiplus::Color mouthFill{};
    Gdiplus::Color blushFill{};
    Gdiplus::Color tailFill{};
    Gdiplus::Color accentFill{};
    Gdiplus::Color shadowFill{};
    Gdiplus::Color pedestalFill{};
    Gdiplus::Color badgeReadyFill{};
    Gdiplus::Color badgePendingFill{};
    Gdiplus::Color accessoryFill{};
    Gdiplus::Color accessoryStroke{};
    float bodyStrokeWidth{1.8f};
    float headStrokeWidth{1.8f};
    float limbStrokeWidth{1.2f};
    float tailStrokeWidth{1.2f};
    float chestStrokeWidth{1.1f};
    float chestFillAlpha{255.0f};
    Gdiplus::RectF glowRect{};
    Gdiplus::RectF shadowRect{};
    Gdiplus::RectF bodyRect{};
    Gdiplus::RectF chestRect{};
    Gdiplus::RectF headRect{};
    Gdiplus::RectF tailRect{};
    std::array<Gdiplus::PointF, 4> leftEar{};
    std::array<Gdiplus::PointF, 4> rightEar{};
    Gdiplus::RectF leftHandRect{};
    Gdiplus::RectF rightHandRect{};
    Gdiplus::RectF leftLegRect{};
    Gdiplus::RectF rightLegRect{};
    Gdiplus::RectF leftEyeRect{};
    Gdiplus::RectF rightEyeRect{};
    Gdiplus::RectF leftPupilRect{};
    Gdiplus::RectF rightPupilRect{};
    Gdiplus::RectF leftEyeHighlightRect{};
    Gdiplus::RectF rightEyeHighlightRect{};
    Gdiplus::PointF leftBrowStart{};
    Gdiplus::PointF leftBrowEnd{};
    Gdiplus::PointF rightBrowStart{};
    Gdiplus::PointF rightBrowEnd{};
    Gdiplus::RectF mouthRect{};
    float mouthStartDeg{10.0f};
    float mouthSweepDeg{160.0f};
    float mouthStrokeWidth{1.4f};
    Gdiplus::RectF noseRect{};
    Gdiplus::RectF leftBlushRect{};
    Gdiplus::RectF rightBlushRect{};
    float eyeHighlightAlpha{190.0f};
    Gdiplus::RectF pedestalRect{};
    std::array<Gdiplus::RectF, 3> laneBadgeRects{};
    std::array<bool, 3> laneReady{};
    bool poseBadgeVisible{false};
    Gdiplus::RectF poseBadgeRect{};
    bool accessoryVisible{false};
    std::array<Gdiplus::PointF, 5> accessoryStar{};
    Win32MouseCompanionRealRendererActionOverlay actionOverlay{};
};

} // namespace mousefx::windows
