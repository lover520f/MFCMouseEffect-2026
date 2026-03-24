#pragma once

#include <array>
#include <string>
#include <vector>

#include <gdiplus.h>

namespace mousefx::windows {

enum class Win32MouseCompanionRealRendererAccessoryShape {
    None = 0,
    Star,
    Moon,
    Leaf,
    RibbonBow,
};

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

struct Win32MouseCompanionRealRendererSceneGraphNode final {
    uint32_t nodeIndex{0};
    std::string nodeName;
    std::string nodePath;
    Gdiplus::RectF bounds{};
    Gdiplus::Color fill{};
    bool highlighted{false};
};

struct Win32MouseCompanionRealRendererSceneGraphEdge final {
    uint32_t fromNodeIndex{0};
    uint32_t toNodeIndex{0};
    Gdiplus::PointF start{};
    Gdiplus::PointF end{};
    float alpha{96.0f};
};

struct Win32MouseCompanionRealRendererSceneGraphLink final {
    std::string logicalNode;
    Gdiplus::PointF start{};
    Gdiplus::PointF end{};
    Gdiplus::Color color{};
    float alpha{168.0f};
};

struct Win32MouseCompanionRealRendererModelProxyNode final {
    std::string logicalNode;
    Gdiplus::RectF bounds{};
    Gdiplus::Color fill{};
    float alpha{164.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererModelProxyLink final {
    std::string logicalNode;
    Gdiplus::PointF start{};
    Gdiplus::PointF end{};
    Gdiplus::Color color{};
    float alpha{136.0f};
};

struct Win32MouseCompanionRealRendererModelProxySurface final {
    std::string surfaceKey;
    std::vector<Gdiplus::PointF> polygon{};
    Gdiplus::Color fill{};
    float alpha{112.0f};
};

struct Win32MouseCompanionRealRendererModelProxySilhouette final {
    std::string logicalNode;
    Gdiplus::RectF bounds{};
    Gdiplus::Color fill{};
    float alpha{104.0f};
};

struct Win32MouseCompanionRealRendererScene final {
    float centerX{0.0f};
    float centerY{0.0f};
    Gdiplus::PointF bodyAnchor{};
    Gdiplus::PointF headAnchor{};
    Gdiplus::PointF appendageAnchor{};
    Gdiplus::PointF overlayAnchor{};
    Gdiplus::PointF groundingAnchor{};
    float bodyAnchorScale{1.0f};
    float headAnchorScale{1.0f};
    float appendageAnchorScale{1.0f};
    float overlayAnchorScale{1.0f};
    float groundingAnchorScale{1.0f};
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
    Gdiplus::Color earStroke{};
    Gdiplus::Color earStrokeRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color earInnerRear{};
    Gdiplus::Color earRootCuffFill{};
    Gdiplus::Color earRootCuffFillRear{};
    Gdiplus::Color eyeFill{};
    Gdiplus::Color mouthFill{};
    Gdiplus::Color blushFill{};
    Gdiplus::Color tailFill{};
    Gdiplus::Color tailFillRear{};
    Gdiplus::Color tailMidFill{};
    Gdiplus::Color tailTipFill{};
    Gdiplus::Color tailStroke{};
    Gdiplus::Color accentFill{};
    Gdiplus::Color shadowFill{};
    Gdiplus::Color pedestalFill{};
    Gdiplus::Color badgeReadyFill{};
    Gdiplus::Color badgePendingFill{};
    Gdiplus::Color accessoryFill{};
    Gdiplus::Color accessoryStroke{};
    float shadowAlphaScale{1.0f};
    float pedestalAlphaScale{1.0f};
    float poseBadgeAlpha{255.0f};
    float accessoryAlphaScale{1.0f};
    float accessoryStrokeWidth{1.0f};
    float bodyStrokeWidth{1.8f};
    float headStrokeWidth{1.8f};
    float earStrokeWidth{1.45f};
    float earStrokeWidthRear{1.15f};
    float earInnerBaseInsetPx{3.0f};
    float earInnerBaseInsetPxRear{4.0f};
    float earInnerMidInsetPx{4.0f};
    float earInnerMidInsetPxRear{5.5f};
    float earInnerTipInsetPx{8.0f};
    float earInnerTipInsetPxRear{10.0f};
    float earOcclusionCapAlpha{224.0f};
    float limbStrokeWidth{1.2f};
    float tailStrokeWidth{1.2f};
    float chestStrokeWidth{1.1f};
    float chestFillAlpha{255.0f};
    Gdiplus::RectF glowRect{};
    Gdiplus::RectF shadowRect{};
    Gdiplus::RectF bodyRect{};
    Gdiplus::RectF chestRect{};
    Gdiplus::RectF neckBridgeRect{};
    Gdiplus::RectF leftHeadShoulderBridgeRect{};
    Gdiplus::RectF rightHeadShoulderBridgeRect{};
    Gdiplus::RectF leftShoulderPatchRect{};
    Gdiplus::RectF rightShoulderPatchRect{};
    Gdiplus::RectF leftHipPatchRect{};
    Gdiplus::RectF rightHipPatchRect{};
    Gdiplus::RectF bellyContourRect{};
    Gdiplus::RectF sternumContourRect{};
    Gdiplus::RectF upperTorsoContourRect{};
    Gdiplus::RectF leftTorsoCadenceBridgeRect{};
    Gdiplus::RectF rightTorsoCadenceBridgeRect{};
    Gdiplus::RectF leftBackContourRect{};
    Gdiplus::RectF rightBackContourRect{};
    Gdiplus::RectF leftFlankContourRect{};
    Gdiplus::RectF rightFlankContourRect{};
    Gdiplus::RectF leftTailHaunchBridgeRect{};
    Gdiplus::RectF rightTailHaunchBridgeRect{};
    Gdiplus::RectF headRect{};
    Gdiplus::RectF tailRect{};
    Gdiplus::RectF tailRootCuffRect{};
    Gdiplus::RectF tailBridgeRect{};
    Gdiplus::RectF tailMidContourRect{};
    Gdiplus::RectF tailTipBridgeRect{};
    Gdiplus::RectF tailTipRect{};
    Gdiplus::RectF leftEarRootCuffRect{};
    Gdiplus::RectF rightEarRootCuffRect{};
    Gdiplus::RectF leftEarOcclusionCapRect{};
    Gdiplus::RectF rightEarOcclusionCapRect{};
    std::array<Gdiplus::PointF, 4> leftEar{};
    std::array<Gdiplus::PointF, 4> rightEar{};
    Gdiplus::RectF leftHandRect{};
    Gdiplus::RectF rightHandRect{};
    Gdiplus::RectF leftLegRect{};
    Gdiplus::RectF rightLegRect{};
    Gdiplus::RectF leftHandRootCuffRect{};
    Gdiplus::RectF rightHandRootCuffRect{};
    Gdiplus::RectF leftLegRootCuffRect{};
    Gdiplus::RectF rightLegRootCuffRect{};
    Gdiplus::RectF leftHandSilhouetteBridgeRect{};
    Gdiplus::RectF rightHandSilhouetteBridgeRect{};
    Gdiplus::RectF leftLegSilhouetteBridgeRect{};
    Gdiplus::RectF rightLegSilhouetteBridgeRect{};
    Gdiplus::RectF leftHandCadenceBridgeRect{};
    Gdiplus::RectF rightHandCadenceBridgeRect{};
    Gdiplus::RectF leftLegCadenceBridgeRect{};
    Gdiplus::RectF rightLegCadenceBridgeRect{};
    Gdiplus::RectF leftHandPadRect{};
    Gdiplus::RectF rightHandPadRect{};
    Gdiplus::RectF leftLegPadRect{};
    Gdiplus::RectF rightLegPadRect{};
    Gdiplus::RectF leftEyeRect{};
    Gdiplus::RectF rightEyeRect{};
    Gdiplus::RectF leftPupilRect{};
    Gdiplus::RectF rightPupilRect{};
    Gdiplus::RectF leftEyeHighlightRect{};
    Gdiplus::RectF rightEyeHighlightRect{};
    std::array<Gdiplus::PointF, 3> leftWhiskerStart{};
    std::array<Gdiplus::PointF, 3> leftWhiskerEnd{};
    std::array<Gdiplus::PointF, 3> rightWhiskerStart{};
    std::array<Gdiplus::PointF, 3> rightWhiskerEnd{};
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
    Gdiplus::RectF leftCheekContourRect{};
    Gdiplus::RectF rightCheekContourRect{};
    Gdiplus::RectF jawContourRect{};
    Gdiplus::RectF muzzlePadRect{};
    Gdiplus::RectF foreheadPadRect{};
    Gdiplus::RectF crownPadRect{};
    Gdiplus::RectF leftParietalBridgeRect{};
    Gdiplus::RectF rightParietalBridgeRect{};
    Gdiplus::RectF leftEarSkullBridgeRect{};
    Gdiplus::RectF rightEarSkullBridgeRect{};
    Gdiplus::RectF leftOccipitalContourRect{};
    Gdiplus::RectF rightOccipitalContourRect{};
    Gdiplus::RectF leftTempleContourRect{};
    Gdiplus::RectF rightTempleContourRect{};
    Gdiplus::RectF leftUnderEyeContourRect{};
    Gdiplus::RectF rightUnderEyeContourRect{};
    Gdiplus::RectF noseBridgeRect{};
    float eyeHighlightAlpha{190.0f};
    float whiskerStrokeWidth{1.0f};
    Gdiplus::RectF pedestalRect{};
    std::array<Gdiplus::RectF, 3> laneBadgeRects{};
    std::array<bool, 3> laneReady{};
    bool poseBadgeVisible{false};
    Gdiplus::RectF poseBadgeRect{};
    bool accessoryVisible{false};
    Win32MouseCompanionRealRendererAccessoryShape accessoryShape{
        Win32MouseCompanionRealRendererAccessoryShape::None};
    Gdiplus::RectF accessoryBounds{};
    std::array<Gdiplus::PointF, 5> accessoryStar{};
    std::array<Gdiplus::PointF, 6> accessoryMoon{};
    Gdiplus::RectF accessoryMoonInsetRect{};
    std::array<Gdiplus::PointF, 4> accessoryLeaf{};
    Gdiplus::PointF accessoryLeafVeinStart{};
    Gdiplus::PointF accessoryLeafVeinEnd{};
    std::array<Gdiplus::PointF, 4> accessoryRibbonLeft{};
    std::array<Gdiplus::PointF, 4> accessoryRibbonRight{};
    Gdiplus::RectF accessoryRibbonCenter{};
    Gdiplus::PointF accessoryRibbonLeftFoldStart{};
    Gdiplus::PointF accessoryRibbonLeftFoldEnd{};
    Gdiplus::PointF accessoryRibbonRightFoldStart{};
    Gdiplus::PointF accessoryRibbonRightFoldEnd{};
    Win32MouseCompanionRealRendererActionOverlay actionOverlay{};
    bool modelSceneGraphVisible{false};
    Gdiplus::RectF modelSceneGraphBounds{};
    std::vector<Win32MouseCompanionRealRendererSceneGraphNode> modelSceneGraphNodes{};
    std::vector<Win32MouseCompanionRealRendererSceneGraphEdge> modelSceneGraphEdges{};
    std::vector<Win32MouseCompanionRealRendererSceneGraphLink> modelSceneGraphLinks{};
    bool modelProxyVisible{false};
    std::vector<Win32MouseCompanionRealRendererModelProxySilhouette> modelProxySilhouettes{};
    std::vector<Win32MouseCompanionRealRendererModelProxySurface> modelProxySurfaces{};
    std::vector<Win32MouseCompanionRealRendererModelProxyNode> modelProxyNodes{};
    std::vector<Win32MouseCompanionRealRendererModelProxyLink> modelProxyLinks{};
    std::vector<Gdiplus::PointF> modelProxyHull{};
};

} // namespace mousefx::windows
