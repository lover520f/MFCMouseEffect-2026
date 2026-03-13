#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>
#include <vector>

namespace mousefx {

struct GestureSimilarityMetrics final {
    size_t strokeCount = 0;
    size_t pointCount = 0;
    double pathLengthPx = 0.0;
    double startEndDistancePx = 0.0;
};

struct GestureTemplateProfile final {
    size_t strokeCount = 0;
    size_t pointCount = 0;
    size_t segmentCount = 0;
    size_t turnCount = 0;
};

struct GestureMatchWindow final {
    size_t start = 0;
    size_t end = 0;
};

struct GestureMatchOptions final {
    bool enableWindowSearch = true;
    bool enableTimeWindowSearch = true;
    bool strictStrokeCount = true;
    bool strictStrokeOrder = true;
    double minEffectiveStrokeLengthPx = 0.0;
    int windowCoverageMinPercent = 30;
    int windowCoverageMaxPercent = 100;
    int windowCoverageStepPercent = 12;
    int windowSlideDivisor = 4;
    int timeWindowMinMs = 200;
    int timeWindowMaxMs = 1200;
    int timeWindowStepMs = 160;
    int timeWindowAnchorStepMs = 90;
    int timeWindowMaxCandidates = 72;
};

struct GestureMatchResult final {
    double bestScore = -1.0;
    double runnerUpScore = -1.0;
    GestureMatchWindow bestWindow{};
    size_t candidateCount = 0;
};

bool IsCustomGesturePatternMode(const std::string& mode);

std::vector<std::vector<AutomationKeyBinding::GesturePoint>> GestureTemplateStrokesFromPattern(
    const AutomationKeyBinding::GesturePattern& pattern);

GestureSimilarityMetrics MeasureCapturedGesture(
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes);

GestureTemplateProfile MeasureGestureTemplateProfile(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes);

GestureTemplateProfile MeasurePresetGestureProfile(
    const std::string& normalizedActionId);

double ScoreGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes);

GestureMatchResult MatchGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes,
    const GestureMatchOptions& options = {});

GestureMatchResult MatchGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes,
    const std::vector<std::vector<uint32_t>>& capturedStrokeTimesMs,
    const GestureMatchOptions& options = {});

double ScorePresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke);

GestureMatchResult MatchPresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke,
    const GestureMatchOptions& options = {});

GestureMatchResult MatchPresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke,
    const std::vector<uint32_t>& capturedStrokeTimesMs,
    const GestureMatchOptions& options = {});

} // namespace mousefx
