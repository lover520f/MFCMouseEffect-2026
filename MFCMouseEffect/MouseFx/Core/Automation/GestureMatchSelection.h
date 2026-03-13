#pragma once

#include "MouseFx/Core/Input/GestureSimilarity.h"

#include <vector>

namespace mousefx::automation_gesture_selection {

struct Candidate final {
    double bestScore = -1.0;
    double runnerUpScore = -1.0;
    GestureMatchWindow bestWindow{};
    size_t candidateCount = 0;
    GestureTemplateProfile profile{};
    int scopeSpecificity = -1;
    int thresholdPercent = 75;
    size_t capturedPointCount = 0;
};

struct CandidateMetrics final {
    double coverageRatio = 1.0;
    double specificityBonus = 0.0;
    double selectionScore = -1.0;
};

CandidateMetrics MeasureCandidate(const Candidate& candidate);

bool PreferLeftOverRight(const Candidate& lhs, const Candidate& rhs);

bool IsDominatedRunnerUp(const Candidate& winner, const Candidate& other);

double EffectiveRunnerUpScore(
    const Candidate& winner,
    const std::vector<Candidate>& candidates);

} // namespace mousefx::automation_gesture_selection
