#include "pch.h"
#include "GestureMatchSelection.h"

#include <algorithm>
#include <cmath>

namespace mousefx::automation_gesture_selection {
namespace {

double CoverageRatio(const Candidate& candidate) {
    if (candidate.capturedPointCount == 0) {
        return 1.0;
    }
    const size_t windowStart = std::min(candidate.bestWindow.start, candidate.capturedPointCount);
    const size_t windowEnd = std::min(candidate.bestWindow.end, candidate.capturedPointCount);
    if (windowEnd <= windowStart) {
        return 1.0;
    }
    const double span = static_cast<double>(windowEnd - windowStart);
    const double total = static_cast<double>(candidate.capturedPointCount);
    return std::clamp(span / total, 0.0, 1.0);
}

double SpecificityBonus(const Candidate& candidate) {
    const double strokeBonus =
        static_cast<double>(std::min<size_t>(candidate.profile.strokeCount, 4)) * 1.15;
    const double turnBonus =
        static_cast<double>(std::min<size_t>(candidate.profile.turnCount, 5)) * 1.55;
    const double segmentBonus =
        static_cast<double>(std::min<size_t>(candidate.profile.segmentCount, 8)) * 0.42;
    double bonus = strokeBonus + turnBonus + segmentBonus;
    const double coverageRatio = CoverageRatio(candidate);
    if (candidate.profile.turnCount >= 2 && coverageRatio >= 0.68) {
        bonus += 1.6;
    }
    if (candidate.profile.segmentCount >= 4 && coverageRatio >= 0.82) {
        bonus += 1.2;
    }
    return bonus;
}

bool IsMeaningfullyMoreComplex(const Candidate& lhs, const Candidate& rhs) {
    return lhs.profile.turnCount > rhs.profile.turnCount ||
        lhs.profile.strokeCount > rhs.profile.strokeCount ||
        lhs.profile.segmentCount >= rhs.profile.segmentCount + 2;
}

} // namespace

CandidateMetrics MeasureCandidate(const Candidate& candidate) {
    CandidateMetrics metrics;
    metrics.coverageRatio = CoverageRatio(candidate);
    metrics.specificityBonus = SpecificityBonus(candidate);
    metrics.selectionScore =
        candidate.bestScore +
        metrics.specificityBonus * 0.78 +
        metrics.coverageRatio * 4.0;
    return metrics;
}

bool PreferLeftOverRight(const Candidate& lhs, const Candidate& rhs) {
    if (lhs.bestScore < 0.0) {
        return false;
    }
    if (rhs.bestScore < 0.0) {
        return true;
    }

    const CandidateMetrics lhsMetrics = MeasureCandidate(lhs);
    const CandidateMetrics rhsMetrics = MeasureCandidate(rhs);
    if (lhs.bestScore > rhs.bestScore + 5.0) {
        return true;
    }
    if (rhs.bestScore > lhs.bestScore + 5.0) {
        return false;
    }
    if (lhsMetrics.selectionScore > rhsMetrics.selectionScore + 1e-6) {
        return true;
    }
    if (rhsMetrics.selectionScore > lhsMetrics.selectionScore + 1e-6) {
        return false;
    }
    if (lhs.scopeSpecificity != rhs.scopeSpecificity) {
        return lhs.scopeSpecificity > rhs.scopeSpecificity;
    }
    if (lhs.bestScore != rhs.bestScore) {
        return lhs.bestScore > rhs.bestScore;
    }
    if (lhs.profile.turnCount != rhs.profile.turnCount) {
        return lhs.profile.turnCount > rhs.profile.turnCount;
    }
    if (lhs.profile.segmentCount != rhs.profile.segmentCount) {
        return lhs.profile.segmentCount > rhs.profile.segmentCount;
    }
    return lhs.profile.strokeCount > rhs.profile.strokeCount;
}

bool IsDominatedRunnerUp(const Candidate& winner, const Candidate& other) {
    if (other.bestScore < 0.0) {
        return false;
    }
    if (PreferLeftOverRight(other, winner)) {
        return false;
    }
    if (winner.bestScore + 6.0 < other.bestScore) {
        return false;
    }

    const CandidateMetrics winnerMetrics = MeasureCandidate(winner);
    const CandidateMetrics otherMetrics = MeasureCandidate(other);
    if (!IsMeaningfullyMoreComplex(winner, other)) {
        return false;
    }
    if (winnerMetrics.coverageRatio + 0.10 < otherMetrics.coverageRatio) {
        return false;
    }
    if (winnerMetrics.selectionScore < otherMetrics.selectionScore + 1.0) {
        return false;
    }
    return true;
}

double EffectiveRunnerUpScore(
    const Candidate& winner,
    const std::vector<Candidate>& candidates) {
    double runnerUpScore = -1.0;
    for (const Candidate& candidate : candidates) {
        if (candidate.bestScore < 0.0) {
            continue;
        }
        if (IsDominatedRunnerUp(winner, candidate)) {
            continue;
        }
        runnerUpScore = std::max(runnerUpScore, candidate.bestScore);
    }
    return runnerUpScore;
}

} // namespace mousefx::automation_gesture_selection
