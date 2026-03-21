#pragma once

#include <array>

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderActionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAccessory.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAdornment.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderExpression.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderGait.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPalette.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPosture.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRhythm.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderSilhouette.h"
#include "Platform/windows/Pet/Win32MouseCompanionVisualState.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderScene {
    float centerX{0.0f};
    float facingSign{1.0f};
    float bodyLeanPx{0.0f};
    float bodyTiltDeg{0.0f};
    Gdiplus::Color bodyFill{};
    Gdiplus::Color bodyStroke{};
    Gdiplus::Color headFill{};
    Gdiplus::Color headFillRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color earInnerRear{};
    Gdiplus::Color accent{};
    float glowAlpha{0.0f};
    Gdiplus::RectF shadowRect{};
    Gdiplus::RectF bodyRect{};
    Gdiplus::RectF headRect{};
    Gdiplus::RectF tailRect{};
    std::array<Gdiplus::PointF, 4> leftEar{};
    std::array<Gdiplus::PointF, 4> rightEar{};
    Gdiplus::RectF leftHandRect{};
    Gdiplus::RectF rightHandRect{};
    Gdiplus::RectF leftLegRect{};
    Gdiplus::RectF rightLegRect{};
    bool modelAssetAvailable{false};
    bool actionLibraryAvailable{false};
    bool poseBadgeVisible{false};
    bool accessoryVisible{false};
    Gdiplus::Color eyeColor{};
    Gdiplus::Color mouthColor{};
    Gdiplus::Color blushColor{};
    Gdiplus::Color tailFill{};
    Gdiplus::Color shadowColor{};
    float eyeHeight{0.0f};
    Gdiplus::RectF leftEyeRect{};
    Gdiplus::RectF rightEyeRect{};
    Gdiplus::RectF mouthRect{};
    Gdiplus::RectF noseRect{};
    Gdiplus::RectF leftBlushRect{};
    Gdiplus::RectF rightBlushRect{};
    Gdiplus::RectF chestRect{};
    Gdiplus::Color chestFill{};
    Gdiplus::Color accentGlow{};
    Gdiplus::RectF tailTipRect{};
    Gdiplus::Color tailTipFill{};
    Gdiplus::RectF leftPawPadRect{};
    Gdiplus::RectF rightPawPadRect{};
    Gdiplus::Color pawPadFill{};
    std::array<Gdiplus::PointF, 2> leftWhiskerTop{};
    std::array<Gdiplus::PointF, 2> leftWhiskerBottom{};
    std::array<Gdiplus::PointF, 2> rightWhiskerTop{};
    std::array<Gdiplus::PointF, 2> rightWhiskerBottom{};
    bool frontSideIsLeft{false};
    Win32MouseCompanionPlaceholderMotion motion{};
    Win32MouseCompanionPlaceholderExpression expression{};
    Win32MouseCompanionPlaceholderAdornment adornment{};
    Win32MouseCompanionPlaceholderActionProfile actionProfile{};
    Win32MouseCompanionPlaceholderGait gait{};
    Win32MouseCompanionPlaceholderAccessory accessory{};
    Win32MouseCompanionPlaceholderPalette palette{};
    Win32MouseCompanionPlaceholderPosture posture{};
    Win32MouseCompanionPlaceholderRhythm rhythm{};
    Win32MouseCompanionPlaceholderSilhouette silhouette{};
};

Win32MouseCompanionPlaceholderScene BuildWin32MouseCompanionPlaceholderScene(
    const Win32MouseCompanionVisualState& state,
    int width,
    int height);

} // namespace mousefx::windows
