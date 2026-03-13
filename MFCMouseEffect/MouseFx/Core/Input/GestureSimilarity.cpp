#include "pch.h"
#include "GestureSimilarity.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace mousefx {
namespace {

struct NormalizedPoint final {
    double x = 0.0;
    double y = 0.0;
};

struct UnitVector final {
    double x = 0.0;
    double y = 0.0;
};

struct StrokeTurnFeature final {
    NormalizedPoint point{};
    double progress = 0.0;
};

struct StrokeStructureFeature final {
    std::vector<NormalizedPoint> anchors;
    std::vector<UnitVector> segmentDirections;
    std::vector<char> segmentCodes;
    std::vector<double> segmentFractions;
    std::vector<StrokeTurnFeature> turns;
    std::vector<int> turnSigns;
    std::vector<double> turnAngles;
    std::vector<double> turnRhythmFractions;
    double startSegmentFraction = 0.0;
    double endSegmentFraction = 0.0;
    double chordToPathRatio = 0.0;
};

double PointDistance(const NormalizedPoint& lhs, const NormalizedPoint& rhs) {
    const double dx = lhs.x - rhs.x;
    const double dy = lhs.y - rhs.y;
    return std::sqrt(dx * dx + dy * dy);
}

double PointDistance(const ScreenPoint& lhs, const ScreenPoint& rhs) {
    const double dx = static_cast<double>(lhs.x) - static_cast<double>(rhs.x);
    const double dy = static_cast<double>(lhs.y) - static_cast<double>(rhs.y);
    return std::sqrt(dx * dx + dy * dy);
}

char QuantizeVectorDirection(double dx, double dy) {
    if (std::abs(dx) < 1e-6 && std::abs(dy) < 1e-6) {
        return '\0';
    }
    const double absDx = std::abs(dx);
    const double absDy = std::abs(dy);
    const double major = std::max(absDx, absDy);
    const double minor = std::min(absDx, absDy);
    if (minor * 5.0 >= major * 2.0) {
        if (dx >= 0.0 && dy >= 0.0) return 'E';
        if (dx >= 0.0) return 'A';
        if (dy >= 0.0) return 'C';
        return 'Q';
    }
    if (absDx >= absDy) {
        return (dx >= 0.0) ? 'R' : 'L';
    }
    return (dy >= 0.0) ? 'D' : 'U';
}

std::vector<std::vector<NormalizedPoint>> NormalizeTemplateStrokes(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& strokes) {
    std::vector<std::vector<NormalizedPoint>> out;
    if (strokes.empty()) {
        return out;
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    bool hasPoint = false;
    for (const auto& stroke : strokes) {
        for (const auto& point : stroke) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
            hasPoint = true;
        }
    }
    if (!hasPoint) {
        return out;
    }

    const double width = std::max(0.0, static_cast<double>(maxX - minX));
    const double height = std::max(0.0, static_cast<double>(maxY - minY));
    const double scale = std::max(1.0, std::max(width, height));
    const double normalizedWidth = (width / scale) * 100.0;
    const double normalizedHeight = (height / scale) * 100.0;
    const double offsetX = (100.0 - normalizedWidth) * 0.5;
    const double offsetY = (100.0 - normalizedHeight) * 0.5;
    for (const auto& stroke : strokes) {
        std::vector<NormalizedPoint> normalizedStroke;
        normalizedStroke.reserve(stroke.size());
        for (const auto& point : stroke) {
            const NormalizedPoint normalized{
                ((static_cast<double>(point.x) - static_cast<double>(minX)) / scale) * 100.0 + offsetX,
                ((static_cast<double>(point.y) - static_cast<double>(minY)) / scale) * 100.0 + offsetY
            };
            if (!normalizedStroke.empty()) {
                const NormalizedPoint& prev = normalizedStroke.back();
                if (std::abs(prev.x - normalized.x) < 0.05 &&
                    std::abs(prev.y - normalized.y) < 0.05) {
                    continue;
                }
            }
            normalizedStroke.push_back(normalized);
        }
        if (!normalizedStroke.empty()) {
            out.push_back(std::move(normalizedStroke));
        }
    }
    return out;
}

std::vector<std::vector<NormalizedPoint>> NormalizeCapturedStrokes(
    const std::vector<std::vector<ScreenPoint>>& strokes) {
    std::vector<std::vector<NormalizedPoint>> out;
    if (strokes.empty()) {
        return out;
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    bool hasPoint = false;
    for (const auto& stroke : strokes) {
        for (const auto& point : stroke) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
            hasPoint = true;
        }
    }
    if (!hasPoint) {
        return out;
    }

    const double width = std::max(0.0, static_cast<double>(maxX - minX));
    const double height = std::max(0.0, static_cast<double>(maxY - minY));
    const double scale = std::max(1.0, std::max(width, height));
    const double normalizedWidth = (width / scale) * 100.0;
    const double normalizedHeight = (height / scale) * 100.0;
    const double offsetX = (100.0 - normalizedWidth) * 0.5;
    const double offsetY = (100.0 - normalizedHeight) * 0.5;
    for (const auto& stroke : strokes) {
        std::vector<NormalizedPoint> normalizedStroke;
        normalizedStroke.reserve(stroke.size());
        for (const auto& point : stroke) {
            const NormalizedPoint normalized{
                ((static_cast<double>(point.x) - static_cast<double>(minX)) / scale) * 100.0 + offsetX,
                ((static_cast<double>(point.y) - static_cast<double>(minY)) / scale) * 100.0 + offsetY
            };
            if (!normalizedStroke.empty()) {
                const NormalizedPoint& prev = normalizedStroke.back();
                if (std::abs(prev.x - normalized.x) < 0.05 &&
                    std::abs(prev.y - normalized.y) < 0.05) {
                    continue;
                }
            }
            normalizedStroke.push_back(normalized);
        }
        if (!normalizedStroke.empty()) {
            out.push_back(std::move(normalizedStroke));
        }
    }
    return out;
}

double StrokeLength(const std::vector<NormalizedPoint>& stroke) {
    if (stroke.size() < 2) {
        return 0.0;
    }
    double length = 0.0;
    for (size_t i = 1; i < stroke.size(); ++i) {
        length += PointDistance(stroke[i - 1], stroke[i]);
    }
    return length;
}

double PointToSegmentDistance(
    const NormalizedPoint& point,
    const NormalizedPoint& from,
    const NormalizedPoint& to) {
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    const double lengthSquared = dx * dx + dy * dy;
    if (lengthSquared <= 1e-6) {
        return PointDistance(point, from);
    }

    const double t = std::clamp(
        ((point.x - from.x) * dx + (point.y - from.y) * dy) / lengthSquared,
        0.0,
        1.0);
    const NormalizedPoint projection{
        from.x + dx * t,
        from.y + dy * t,
    };
    return PointDistance(point, projection);
}

double MeanDistanceToChordLine(const std::vector<NormalizedPoint>& stroke) {
    if (stroke.size() <= 2) {
        return 0.0;
    }
    double distanceSum = 0.0;
    size_t count = 0;
    for (size_t i = 1; i + 1 < stroke.size(); ++i) {
        distanceSum += PointToSegmentDistance(stroke[i], stroke.front(), stroke.back());
        ++count;
    }
    if (count == 0) {
        return 0.0;
    }
    return distanceSum / static_cast<double>(count);
}

std::vector<NormalizedPoint> ResampleStroke(const std::vector<NormalizedPoint>& stroke, size_t sampleCount) {
    if (stroke.empty() || sampleCount == 0) {
        return {};
    }
    if (stroke.size() == 1 || sampleCount == 1) {
        return std::vector<NormalizedPoint>(sampleCount, stroke.front());
    }

    std::vector<double> cumulativeLengths;
    cumulativeLengths.reserve(stroke.size());
    cumulativeLengths.push_back(0.0);
    for (size_t i = 1; i < stroke.size(); ++i) {
        cumulativeLengths.push_back(cumulativeLengths.back() + PointDistance(stroke[i - 1], stroke[i]));
    }

    const double totalLength = cumulativeLengths.back();
    if (!(totalLength > 1e-6)) {
        return std::vector<NormalizedPoint>(sampleCount, stroke.front());
    }

    std::vector<NormalizedPoint> resampled;
    resampled.reserve(sampleCount);
    size_t segmentIndex = 1;
    for (size_t i = 0; i < sampleCount; ++i) {
        const double ratio = (sampleCount <= 1)
            ? 0.0
            : static_cast<double>(i) / static_cast<double>(sampleCount - 1);
        const double targetLength = ratio * totalLength;
        while (segmentIndex < cumulativeLengths.size() &&
               cumulativeLengths[segmentIndex] < targetLength) {
            ++segmentIndex;
        }
        if (segmentIndex >= cumulativeLengths.size()) {
            resampled.push_back(stroke.back());
            continue;
        }
        const size_t from = (segmentIndex == 0) ? 0 : segmentIndex - 1;
        const double segmentStart = cumulativeLengths[from];
        const double segmentEnd = cumulativeLengths[segmentIndex];
        const double segmentLength = std::max(1e-6, segmentEnd - segmentStart);
        const double localRatio = (targetLength - segmentStart) / segmentLength;
        resampled.push_back(NormalizedPoint{
            stroke[from].x + (stroke[segmentIndex].x - stroke[from].x) * localRatio,
            stroke[from].y + (stroke[segmentIndex].y - stroke[from].y) * localRatio
        });
    }
    return resampled;
}

void DouglasPeuckerCollect(
    const std::vector<NormalizedPoint>& stroke,
    size_t start,
    size_t end,
    double epsilon,
    std::vector<size_t>* outIndices) {
    if (!outIndices || end <= start + 1 || end >= stroke.size()) {
        return;
    }

    double maxDistance = -1.0;
    size_t farthestIndex = start;
    for (size_t i = start + 1; i < end; ++i) {
        const double distance = PointToSegmentDistance(stroke[i], stroke[start], stroke[end]);
        if (distance > maxDistance) {
            maxDistance = distance;
            farthestIndex = i;
        }
    }

    if (maxDistance < epsilon || farthestIndex <= start || farthestIndex >= end) {
        return;
    }

    DouglasPeuckerCollect(stroke, start, farthestIndex, epsilon, outIndices);
    outIndices->push_back(farthestIndex);
    DouglasPeuckerCollect(stroke, farthestIndex, end, epsilon, outIndices);
}

double TurnAngleDegrees(
    const NormalizedPoint& a,
    const NormalizedPoint& b,
    const NormalizedPoint& c) {
    const double abx = b.x - a.x;
    const double aby = b.y - a.y;
    const double bcx = c.x - b.x;
    const double bcy = c.y - b.y;
    const double abLen = std::sqrt(abx * abx + aby * aby);
    const double bcLen = std::sqrt(bcx * bcx + bcy * bcy);
    if (abLen <= 1e-6 || bcLen <= 1e-6) {
        return 0.0;
    }
    const double dot = std::clamp((abx * bcx + aby * bcy) / (abLen * bcLen), -1.0, 1.0);
    return std::acos(dot) * (180.0 / 3.14159265358979323846);
}

std::vector<NormalizedPoint> SimplifyStrokeAnchors(const std::vector<NormalizedPoint>& stroke) {
    if (stroke.size() <= 2) {
        return stroke;
    }

    const size_t sampleCount = std::clamp<size_t>(stroke.size(), 16, 56);
    const std::vector<NormalizedPoint> sampled = ResampleStroke(stroke, sampleCount);
    if (sampled.size() <= 2) {
        return sampled;
    }

    std::vector<bool> protectedTurn(sampled.size(), false);
    for (size_t i = 1; i + 1 < sampled.size(); ++i) {
        const double prevLen = PointDistance(sampled[i - 1], sampled[i]);
        const double nextLen = PointDistance(sampled[i], sampled[i + 1]);
        if (prevLen <= 1.8 || nextLen <= 1.8) {
            continue;
        }
        const double turnAngle = TurnAngleDegrees(sampled[i - 1], sampled[i], sampled[i + 1]);
        const double middleDistance = PointToSegmentDistance(sampled[i], sampled[i - 1], sampled[i + 1]);
        // Preserve meaningful direction changes so zig-zag families (W/M/complex) are not flattened.
        if (turnAngle >= 34.0 && middleDistance >= 1.4) {
            protectedTurn[i] = true;
        }
    }

    std::vector<size_t> anchorIndices{0};
    DouglasPeuckerCollect(sampled, 0, sampled.size() - 1, 3.8, &anchorIndices);
    anchorIndices.push_back(sampled.size() - 1);
    for (size_t i = 1; i + 1 < sampled.size(); ++i) {
        if (protectedTurn[i]) {
            anchorIndices.push_back(i);
        }
    }
    std::sort(anchorIndices.begin(), anchorIndices.end());
    anchorIndices.erase(std::unique(anchorIndices.begin(), anchorIndices.end()), anchorIndices.end());

    bool changed = true;
    while (changed && anchorIndices.size() >= 3) {
        changed = false;
        for (size_t i = 1; i + 1 < anchorIndices.size(); ++i) {
            const size_t currentIndex = anchorIndices[i];
            if (protectedTurn[currentIndex]) {
                continue;
            }
            const size_t prevIndex = anchorIndices[i - 1];
            const size_t nextIndex = anchorIndices[i + 1];
            const double middleDistance =
                PointToSegmentDistance(sampled[currentIndex], sampled[prevIndex], sampled[nextIndex]);
            const double prevLength = PointDistance(sampled[prevIndex], sampled[currentIndex]);
            const double nextLength = PointDistance(sampled[currentIndex], sampled[nextIndex]);
            const double turnAngle =
                TurnAngleDegrees(sampled[prevIndex], sampled[currentIndex], sampled[nextIndex]);
            if (middleDistance <= 3.2 &&
                (prevLength <= 10.5 || nextLength <= 10.5 || turnAngle <= 24.0)) {
                anchorIndices.erase(anchorIndices.begin() + static_cast<std::ptrdiff_t>(i));
                changed = true;
                break;
            }
        }
    }

    std::vector<NormalizedPoint> anchors;
    anchors.reserve(anchorIndices.size());
    for (const size_t index : anchorIndices) {
        anchors.push_back(sampled[index]);
    }
    return anchors;
}

StrokeStructureFeature BuildStrokeStructureFeature(const std::vector<NormalizedPoint>& stroke) {
    StrokeStructureFeature feature;
    feature.anchors = SimplifyStrokeAnchors(stroke);
    if (feature.anchors.size() < 2) {
        return feature;
    }

    double totalLength = 0.0;
    std::vector<double> segmentLengths;
    segmentLengths.reserve(feature.anchors.size() - 1);
    for (size_t i = 1; i < feature.anchors.size(); ++i) {
        const double length = PointDistance(feature.anchors[i - 1], feature.anchors[i]);
        if (length <= 1e-6) {
            continue;
        }
        segmentLengths.push_back(length);
        totalLength += length;
    }
    if (!(totalLength > 1e-6) || segmentLengths.empty()) {
        return feature;
    }

    double cumulativeLength = 0.0;
    for (size_t i = 1; i < feature.anchors.size(); ++i) {
        const double dx = feature.anchors[i].x - feature.anchors[i - 1].x;
        const double dy = feature.anchors[i].y - feature.anchors[i - 1].y;
        const double length = std::sqrt(dx * dx + dy * dy);
        if (length <= 1e-6) {
            continue;
        }
        feature.segmentDirections.push_back(UnitVector{dx / length, dy / length});
        feature.segmentCodes.push_back(QuantizeVectorDirection(dx, dy));
        feature.segmentFractions.push_back(length / totalLength);
        cumulativeLength += length;
        if (i + 1 < feature.anchors.size()) {
            feature.turns.push_back(StrokeTurnFeature{
                feature.anchors[i],
                cumulativeLength / totalLength,
            });
        }
    }

    feature.startSegmentFraction = feature.segmentFractions.front();
    feature.endSegmentFraction = feature.segmentFractions.back();
    feature.chordToPathRatio =
        PointDistance(feature.anchors.front(), feature.anchors.back()) / totalLength;
    for (size_t i = 1; i < feature.segmentDirections.size(); ++i) {
        const UnitVector& prev = feature.segmentDirections[i - 1];
        const UnitVector& next = feature.segmentDirections[i];
        const double cross = prev.x * next.y - prev.y * next.x;
        feature.turnSigns.push_back((cross > 0.08) ? 1 : ((cross < -0.08) ? -1 : 0));
        const double dot = std::clamp(prev.x * next.x + prev.y * next.y, -1.0, 1.0);
        feature.turnAngles.push_back(
            std::acos(dot) * (180.0 / 3.14159265358979323846));
    }
    if (!feature.turns.empty()) {
        feature.turnRhythmFractions.reserve(feature.turns.size() + 1);
        double prevProgress = 0.0;
        for (const StrokeTurnFeature& turn : feature.turns) {
            const double clampedProgress = std::clamp(turn.progress, 0.0, 1.0);
            feature.turnRhythmFractions.push_back(
                std::max(0.0, clampedProgress - prevProgress));
            prevProgress = clampedProgress;
        }
        feature.turnRhythmFractions.push_back(std::max(0.0, 1.0 - prevProgress));
    }
    return feature;
}

double DtwShapeScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return -1.0;
    }
    const size_t sampleCount = std::clamp<size_t>(
        std::max(lhs.size(), rhs.size()) * 2,
        12,
        96);
    const std::vector<NormalizedPoint> sampledLhs = ResampleStroke(lhs, sampleCount);
    const std::vector<NormalizedPoint> sampledRhs = ResampleStroke(rhs, sampleCount);
    if (sampledLhs.empty() || sampledRhs.empty()) {
        return -1.0;
    }

    const size_t m = sampledRhs.size();
    std::vector<double> prev(m, std::numeric_limits<double>::infinity());
    std::vector<double> curr(m, std::numeric_limits<double>::infinity());
    for (size_t i = 0; i < sampledLhs.size(); ++i) {
        for (size_t j = 0; j < m; ++j) {
            const double cost = PointDistance(sampledLhs[i], sampledRhs[j]);
            if (i == 0 && j == 0) {
                curr[j] = cost;
                continue;
            }
            const double fromUp = (i > 0) ? prev[j] : std::numeric_limits<double>::infinity();
            const double fromLeft = (j > 0) ? curr[j - 1] : std::numeric_limits<double>::infinity();
            const double fromDiag = (i > 0 && j > 0) ? prev[j - 1] : std::numeric_limits<double>::infinity();
            curr[j] = cost + std::min({fromUp, fromLeft, fromDiag});
        }
        std::swap(prev, curr);
        std::fill(curr.begin(), curr.end(), std::numeric_limits<double>::infinity());
    }

    const double pathCost = prev.back();
    const double averageDistance = pathCost / static_cast<double>(sampledLhs.size() + sampledRhs.size());
    const double maxDistance = std::sqrt(100.0 * 100.0 + 100.0 * 100.0);
    return std::clamp(1.0 - (averageDistance / maxDistance), 0.0, 1.0) * 100.0;
}

char QuantizeDirection(const NormalizedPoint& from, const NormalizedPoint& to) {
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    if (std::abs(dx) < 1e-6 && std::abs(dy) < 1e-6) {
        return '\0';
    }
    const double absDx = std::abs(dx);
    const double absDy = std::abs(dy);
    const double major = std::max(absDx, absDy);
    const double minor = std::min(absDx, absDy);
    if (minor * 5.0 >= major * 2.0) {
        if (dx >= 0.0 && dy >= 0.0) return 'E';
        if (dx >= 0.0) return 'A';
        if (dy >= 0.0) return 'C';
        return 'Q';
    }
    if (absDx >= absDy) {
        return (dx >= 0.0) ? 'R' : 'L';
    }
    return (dy >= 0.0) ? 'D' : 'U';
}

bool IsCardinalDirection(char dir) {
    return dir == 'L' || dir == 'R' || dir == 'U' || dir == 'D';
}

void SimplifyDirectionSequence(std::vector<char>* dirs) {
    if (!dirs) {
        return;
    }
    bool changed = true;
    while (changed && dirs->size() >= 3) {
        changed = false;
        for (size_t i = 1; i + 1 < dirs->size(); ++i) {
            const char prev = (*dirs)[i - 1];
            const char mid = (*dirs)[i];
            const char next = (*dirs)[i + 1];
            if (prev == next && IsCardinalDirection(mid)) {
                dirs->erase(
                    dirs->begin() + static_cast<std::ptrdiff_t>(i),
                    dirs->begin() + static_cast<std::ptrdiff_t>(i + 2));
                changed = true;
                break;
            }
        }
    }
}

std::vector<char> DirectionSequence(const std::vector<NormalizedPoint>& stroke) {
    std::vector<char> dirs;
    if (stroke.size() < 2) {
        return dirs;
    }
    char prev = '\0';
    for (size_t i = 1; i < stroke.size(); ++i) {
        const char dir = QuantizeDirection(stroke[i - 1], stroke[i]);
        if (dir == '\0' || dir == prev) {
            continue;
        }
        dirs.push_back(dir);
        prev = dir;
    }
    SimplifyDirectionSequence(&dirs);
    return dirs;
}

size_t EditDistance(const std::vector<char>& lhs, const std::vector<char>& rhs) {
    if (lhs.empty()) {
        return rhs.size();
    }
    if (rhs.empty()) {
        return lhs.size();
    }
    std::vector<size_t> prev(rhs.size() + 1);
    std::vector<size_t> curr(rhs.size() + 1);
    for (size_t j = 0; j <= rhs.size(); ++j) {
        prev[j] = j;
    }
    for (size_t i = 1; i <= lhs.size(); ++i) {
        curr[0] = i;
        for (size_t j = 1; j <= rhs.size(); ++j) {
            const size_t substitution = prev[j - 1] + (lhs[i - 1] == rhs[j - 1] ? 0 : 1);
            const size_t insertion = curr[j - 1] + 1;
            const size_t deletion = prev[j] + 1;
            curr[j] = std::min({substitution, insertion, deletion});
        }
        std::swap(prev, curr);
    }
    return prev.back();
}

double DirectionSequenceScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs) {
    const std::vector<char> lhsDirs = DirectionSequence(lhs);
    const std::vector<char> rhsDirs = DirectionSequence(rhs);
    if (lhsDirs.empty() || rhsDirs.empty()) {
        return 50.0;
    }
    const size_t maxLen = std::max(lhsDirs.size(), rhsDirs.size());
    if (maxLen == 0) {
        return 100.0;
    }
    const size_t edits = EditDistance(lhsDirs, rhsDirs);
    return std::clamp(1.0 - (static_cast<double>(edits) / static_cast<double>(maxLen)), 0.0, 1.0) * 100.0;
}

bool TryDirectionVector(
    const std::vector<NormalizedPoint>& stroke,
    bool useEnd,
    UnitVector* outVector) {
    if (!outVector || stroke.size() < 2) {
        return false;
    }
    if (!useEnd) {
        for (size_t i = 1; i < stroke.size(); ++i) {
            const double dx = stroke[i].x - stroke[i - 1].x;
            const double dy = stroke[i].y - stroke[i - 1].y;
            const double length = std::sqrt(dx * dx + dy * dy);
            if (length > 1e-6) {
                *outVector = UnitVector{dx / length, dy / length};
                return true;
            }
        }
        return false;
    }
    for (size_t i = stroke.size() - 1; i > 0; --i) {
        const double dx = stroke[i].x - stroke[i - 1].x;
        const double dy = stroke[i].y - stroke[i - 1].y;
        const double length = std::sqrt(dx * dx + dy * dy);
        if (length > 1e-6) {
            *outVector = UnitVector{dx / length, dy / length};
            return true;
        }
    }
    return false;
}

double TangentScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs) {
    UnitVector lhsStart{};
    UnitVector rhsStart{};
    UnitVector lhsEnd{};
    UnitVector rhsEnd{};
    const bool hasStart = TryDirectionVector(lhs, false, &lhsStart) &&
        TryDirectionVector(rhs, false, &rhsStart);
    const bool hasEnd = TryDirectionVector(lhs, true, &lhsEnd) &&
        TryDirectionVector(rhs, true, &rhsEnd);
    if (!hasStart && !hasEnd) {
        return 50.0;
    }

    auto dotScore = [](const UnitVector& a, const UnitVector& b) {
        const double dot = std::clamp(a.x * b.x + a.y * b.y, -1.0, 1.0);
        return ((dot + 1.0) * 0.5) * 100.0;
    };

    if (hasStart && hasEnd) {
        return (dotScore(lhsStart, rhsStart) + dotScore(lhsEnd, rhsEnd)) * 0.5;
    }
    return hasStart ? dotScore(lhsStart, rhsStart) : dotScore(lhsEnd, rhsEnd);
}

double SegmentDirectionScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.segmentDirections.empty() || rhs.segmentDirections.empty()) {
        return 50.0;
    }

    const size_t pairCount = std::min(lhs.segmentDirections.size(), rhs.segmentDirections.size());
    double weightedScore = 0.0;
    double totalWeight = 0.0;
    for (size_t i = 0; i < pairCount; ++i) {
        const UnitVector& a = lhs.segmentDirections[i];
        const UnitVector& b = rhs.segmentDirections[i];
        const double dot = std::clamp(a.x * b.x + a.y * b.y, -1.0, 1.0);
        const double score = ((dot + 1.0) * 0.5) * 100.0;
        const double weight = std::max(
            0.12,
            (lhs.segmentFractions[i] + rhs.segmentFractions[i]) * 0.5);
        weightedScore += score * weight;
        totalWeight += weight;
    }
    if (!(totalWeight > 1e-6)) {
        return 50.0;
    }

    const double countRatio =
        static_cast<double>(pairCount) /
        static_cast<double>(std::max(lhs.segmentDirections.size(), rhs.segmentDirections.size()));
    return std::clamp((weightedScore / totalWeight) * countRatio, 0.0, 100.0);
}

double SegmentCodeSequenceScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.segmentCodes.empty() || rhs.segmentCodes.empty()) {
        return 50.0;
    }
    const size_t maxLen = std::max(lhs.segmentCodes.size(), rhs.segmentCodes.size());
    if (maxLen == 0) {
        return 100.0;
    }
    const size_t edits = EditDistance(lhs.segmentCodes, rhs.segmentCodes);
    return std::clamp(1.0 - (static_cast<double>(edits) / static_cast<double>(maxLen)), 0.0, 1.0) * 100.0;
}

double TurnSignScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.turnSigns.empty() && rhs.turnSigns.empty()) {
        return 100.0;
    }
    if (lhs.turnSigns.empty() || rhs.turnSigns.empty()) {
        return 20.0;
    }
    const size_t maxLen = std::max(lhs.turnSigns.size(), rhs.turnSigns.size());
    const size_t pairCount = std::min(lhs.turnSigns.size(), rhs.turnSigns.size());
    size_t mismatches = 0;
    for (size_t i = 0; i < pairCount; ++i) {
        if (lhs.turnSigns[i] != rhs.turnSigns[i]) {
            ++mismatches;
        }
    }
    mismatches += (maxLen - pairCount);
    return std::clamp(1.0 - (static_cast<double>(mismatches) / static_cast<double>(maxLen)), 0.0, 1.0) * 100.0;
}

double TurnAngleProfileScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.turnAngles.empty() && rhs.turnAngles.empty()) {
        return 100.0;
    }
    if (lhs.turnAngles.empty() || rhs.turnAngles.empty()) {
        return 20.0;
    }

    const size_t pairCount = std::min(lhs.turnAngles.size(), rhs.turnAngles.size());
    double scoreSum = 0.0;
    for (size_t i = 0; i < pairCount; ++i) {
        const double angleDiff = std::abs(lhs.turnAngles[i] - rhs.turnAngles[i]);
        double turnScore = std::clamp(100.0 - angleDiff * 0.85, 0.0, 100.0);
        if (i < lhs.turnSigns.size() && i < rhs.turnSigns.size() &&
            lhs.turnSigns[i] != 0 && rhs.turnSigns[i] != 0 &&
            lhs.turnSigns[i] != rhs.turnSigns[i]) {
            turnScore *= 0.72;
        }
        scoreSum += turnScore;
    }

    const double meanScore = scoreSum / static_cast<double>(pairCount);
    const double countRatio =
        static_cast<double>(pairCount) /
        static_cast<double>(std::max(lhs.turnAngles.size(), rhs.turnAngles.size()));
    return std::clamp(meanScore * countRatio, 0.0, 100.0);
}

double TurnRhythmScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.turnRhythmFractions.empty() && rhs.turnRhythmFractions.empty()) {
        return 100.0;
    }
    if (lhs.turnRhythmFractions.empty() || rhs.turnRhythmFractions.empty()) {
        return 20.0;
    }

    const size_t pairCount =
        std::min(lhs.turnRhythmFractions.size(), rhs.turnRhythmFractions.size());
    double diffSum = 0.0;
    for (size_t i = 0; i < pairCount; ++i) {
        diffSum += std::abs(lhs.turnRhythmFractions[i] - rhs.turnRhythmFractions[i]);
    }
    diffSum /= static_cast<double>(pairCount);

    const double countRatio =
        static_cast<double>(pairCount) /
        static_cast<double>(std::max(lhs.turnRhythmFractions.size(), rhs.turnRhythmFractions.size()));
    return std::clamp((100.0 - diffSum * 160.0) * countRatio, 0.0, 100.0);
}

double TurnAlternationRatio(const StrokeStructureFeature& feature) {
    if (feature.turnSigns.size() < 2) {
        return 0.0;
    }
    size_t alternations = 0;
    size_t comparable = 0;
    for (size_t i = 1; i < feature.turnSigns.size(); ++i) {
        const int prev = feature.turnSigns[i - 1];
        const int curr = feature.turnSigns[i];
        if (prev == 0 || curr == 0) {
            continue;
        }
        ++comparable;
        if ((prev > 0 && curr < 0) || (prev < 0 && curr > 0)) {
            ++alternations;
        }
    }
    if (comparable == 0) {
        return 0.0;
    }
    return static_cast<double>(alternations) / static_cast<double>(comparable);
}

double TurnLayoutScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.turns.empty() && rhs.turns.empty()) {
        return 100.0;
    }
    if (lhs.turns.empty() || rhs.turns.empty()) {
        return 20.0;
    }

    const size_t pairCount = std::min(lhs.turns.size(), rhs.turns.size());
    double scoreSum = 0.0;
    for (size_t i = 0; i < pairCount; ++i) {
        const double spatialDistance = PointDistance(lhs.turns[i].point, rhs.turns[i].point);
        const double normalizedSpatial = std::clamp(
            spatialDistance / std::sqrt(100.0 * 100.0 + 100.0 * 100.0),
            0.0,
            1.0);
        const double progressPenalty = std::clamp(
            std::abs(lhs.turns[i].progress - rhs.turns[i].progress),
            0.0,
            1.0);
        const double turnScore = std::clamp(
            1.0 - normalizedSpatial * 0.45 - progressPenalty * 0.55,
            0.0,
            1.0) * 100.0;
        scoreSum += turnScore;
    }

    const double meanScore = scoreSum / static_cast<double>(pairCount);
    const double countRatio =
        static_cast<double>(pairCount) /
        static_cast<double>(std::max(lhs.turns.size(), rhs.turns.size()));
    return std::clamp(meanScore * countRatio, 0.0, 100.0);
}

double SegmentBalanceScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.segmentFractions.empty() || rhs.segmentFractions.empty()) {
        return 50.0;
    }
    const size_t pairCount = std::min(lhs.segmentFractions.size(), rhs.segmentFractions.size());
    double segmentFractionDelta = 0.0;
    for (size_t i = 0; i < pairCount; ++i) {
        segmentFractionDelta += std::abs(lhs.segmentFractions[i] - rhs.segmentFractions[i]);
    }
    segmentFractionDelta /= static_cast<double>(pairCount);

    const double startDiff = std::abs(lhs.startSegmentFraction - rhs.startSegmentFraction);
    const double endDiff = std::abs(lhs.endSegmentFraction - rhs.endSegmentFraction);
    const double chordDiff = std::abs(lhs.chordToPathRatio - rhs.chordToPathRatio);
    return std::clamp(
        100.0 - (
            segmentFractionDelta * 42.0 +
            startDiff * 22.0 +
            endDiff * 22.0 +
            chordDiff * 14.0) * 100.0,
        0.0,
        100.0);
}

double SingleTurnSymmetryScore(
    const StrokeStructureFeature& lhs,
    const StrokeStructureFeature& rhs) {
    if (lhs.segmentFractions.size() != 2 || rhs.segmentFractions.size() != 2 ||
        lhs.turns.size() != 1 || rhs.turns.size() != 1) {
        return 50.0;
    }

    const double lhsAsymmetry = std::abs(lhs.startSegmentFraction - lhs.endSegmentFraction);
    const double rhsAsymmetry = std::abs(rhs.startSegmentFraction - rhs.endSegmentFraction);
    const double asymmetryGap = std::abs(lhsAsymmetry - rhsAsymmetry);
    return std::clamp(100.0 - asymmetryGap * 180.0, 0.0, 100.0);
}

double StructureScore(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature) {
    if (lhsFeature.segmentDirections.empty() || rhsFeature.segmentDirections.empty()) {
        return 50.0;
    }

    const size_t lhsSegments = lhsFeature.segmentDirections.size();
    const size_t rhsSegments = rhsFeature.segmentDirections.size();
    const double segmentCountScore = std::clamp(
        100.0 - static_cast<double>(std::max(lhsSegments, rhsSegments) - std::min(lhsSegments, rhsSegments)) * 28.0,
        0.0,
        100.0);
    const double segmentDirectionScore = SegmentDirectionScore(lhsFeature, rhsFeature);
    const double segmentCodeScore = SegmentCodeSequenceScore(lhsFeature, rhsFeature);
    const double turnLayoutScore = TurnLayoutScore(lhsFeature, rhsFeature);
    const double turnSignScore = TurnSignScore(lhsFeature, rhsFeature);
    const double turnAngleProfileScore = TurnAngleProfileScore(lhsFeature, rhsFeature);
    const double turnRhythmScore = TurnRhythmScore(lhsFeature, rhsFeature);
    const double segmentBalanceScore = SegmentBalanceScore(lhsFeature, rhsFeature);
    const double singleTurnSymmetryScore = SingleTurnSymmetryScore(lhsFeature, rhsFeature);
    return std::clamp(
        segmentCountScore * 0.08 +
        segmentDirectionScore * 0.14 +
        segmentCodeScore * 0.16 +
        turnLayoutScore * 0.18 +
        turnSignScore * 0.10 +
        turnAngleProfileScore * 0.10 +
        turnRhythmScore * 0.06 +
        segmentBalanceScore * 0.10 +
        singleTurnSymmetryScore * 0.08 +
        std::max(turnSignScore, turnAngleProfileScore) * 0.10,
        0.0,
        100.0);
}

double StraightStrokeCompatibilityScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs,
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature) {
    const bool lhsTemplateStraight =
        lhsFeature.segmentDirections.size() == 1 && lhsFeature.turns.empty();
    const bool rhsTemplateStraight =
        rhsFeature.segmentDirections.size() == 1 && rhsFeature.turns.empty();
    if (!lhsTemplateStraight && !rhsTemplateStraight) {
        return -1.0;
    }

    UnitVector lhsChord{};
    UnitVector rhsChord{};
    if (!TryDirectionVector(lhs, false, &lhsChord) || !TryDirectionVector(rhs, false, &rhsChord)) {
        return -1.0;
    }
    const double directionDot = std::clamp(lhsChord.x * rhsChord.x + lhsChord.y * rhsChord.y, -1.0, 1.0);
    const double angleDegrees = std::acos(directionDot) * (180.0 / 3.14159265358979323846);
    const double directionScore = std::clamp(100.0 - angleDegrees * 2.6, 0.0, 100.0);
    const double lhsDeviation = MeanDistanceToChordLine(lhs);
    const double rhsDeviation = MeanDistanceToChordLine(rhs);
    const double deviationScore = std::clamp(
        100.0 - ((lhsDeviation + rhsDeviation) * 0.5) * 7.0,
        0.0,
        100.0);
    const double chordScore = std::clamp(
        100.0 - std::abs(lhsFeature.chordToPathRatio - rhsFeature.chordToPathRatio) * 150.0,
        0.0,
        100.0);
    const double candidateLinearityScore = std::clamp(
        100.0 - std::max(lhsDeviation, rhsDeviation) * 7.5,
        0.0,
        100.0);
    return std::clamp(
        directionScore * 0.48 +
        deviationScore * 0.18 +
        chordScore * 0.16 +
        candidateLinearityScore * 0.18,
        0.0,
        100.0);
}

double MultiTurnPatternBonus(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature) {
    if (lhsFeature.turnSigns.size() < 2 ||
        rhsFeature.turnSigns.size() < 2 ||
        lhsFeature.turnSigns.size() != rhsFeature.turnSigns.size()) {
        return 0.0;
    }
    const double turnScore = TurnSignScore(lhsFeature, rhsFeature);
    const double segmentCodeScore = SegmentCodeSequenceScore(lhsFeature, rhsFeature);
    const double turnAngleScore = TurnAngleProfileScore(lhsFeature, rhsFeature);
    const double lhsAlternation = TurnAlternationRatio(lhsFeature);
    const double rhsAlternation = TurnAlternationRatio(rhsFeature);
    const double alternationScore = std::clamp(
        100.0 - std::abs(lhsAlternation - rhsAlternation) * 100.0,
        0.0,
        100.0);
    const double consistency =
        turnScore * 0.36 +
        segmentCodeScore * 0.28 +
        turnAngleScore * 0.24 +
        alternationScore * 0.12;
    if (consistency <= 64.0) {
        return 0.0;
    }
    return std::clamp((consistency - 64.0) * 0.62, 0.0, 20.0);
}

double MultiSegmentIntentBonus(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature,
    double directionScore) {
    const size_t lhsSegments = lhsFeature.segmentDirections.size();
    const size_t rhsSegments = rhsFeature.segmentDirections.size();
    if (std::max(lhsSegments, rhsSegments) < 3) {
        return 0.0;
    }
    const size_t segmentGap = std::max(lhsSegments, rhsSegments) - std::min(lhsSegments, rhsSegments);
    if (segmentGap > 1 || directionScore < 72.0) {
        return 0.0;
    }
    return std::clamp((directionScore - 72.0) * 0.42, 0.0, 9.0);
}

double DiagonalSegmentRatio(const StrokeStructureFeature& feature);

double ZigZagFamilyIntentBonus(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature) {
    if (lhsFeature.turnSigns.size() < 2 || rhsFeature.turnSigns.size() < 2) {
        return 0.0;
    }
    if (lhsFeature.segmentDirections.size() < 3 || rhsFeature.segmentDirections.size() < 3) {
        return 0.0;
    }
    const double lhsAlternation = TurnAlternationRatio(lhsFeature);
    const double rhsAlternation = TurnAlternationRatio(rhsFeature);
    if (lhsAlternation < 0.85 || rhsAlternation < 0.85) {
        return 0.0;
    }

    const double segmentCodeScore = SegmentCodeSequenceScore(lhsFeature, rhsFeature);
    const double turnAngleProfileScore = TurnAngleProfileScore(lhsFeature, rhsFeature);
    const double turnRhythmScore = TurnRhythmScore(lhsFeature, rhsFeature);
    const double consistency =
        segmentCodeScore * 0.42 +
        turnAngleProfileScore * 0.34 +
        turnRhythmScore * 0.24;
    if (consistency <= 62.0) {
        return 0.0;
    }
    return std::clamp((consistency - 62.0) * 0.40, 0.0, 12.0);
}

double ZigZagStrongMultiTurnBonus(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature) {
    if (lhsFeature.turns.size() < 3 || rhsFeature.turns.size() < 3) {
        return 0.0;
    }
    if (lhsFeature.segmentDirections.size() < 4 || rhsFeature.segmentDirections.size() < 4) {
        return 0.0;
    }

    const double lhsAlternation = TurnAlternationRatio(lhsFeature);
    const double rhsAlternation = TurnAlternationRatio(rhsFeature);
    if (lhsAlternation < 0.90 || rhsAlternation < 0.90) {
        return 0.0;
    }
    const double lhsDiagonalRatio = DiagonalSegmentRatio(lhsFeature);
    const double rhsDiagonalRatio = DiagonalSegmentRatio(rhsFeature);
    if (lhsDiagonalRatio < 0.85 || rhsDiagonalRatio < 0.85) {
        return 0.0;
    }

    const double segmentCodeScore = SegmentCodeSequenceScore(lhsFeature, rhsFeature);
    const double turnAngleProfileScore = TurnAngleProfileScore(lhsFeature, rhsFeature);
    const double turnLayoutScore = TurnLayoutScore(lhsFeature, rhsFeature);
    const double turnRhythmScore = TurnRhythmScore(lhsFeature, rhsFeature);
    const double consistency =
        segmentCodeScore * 0.30 +
        turnAngleProfileScore * 0.28 +
        turnLayoutScore * 0.24 +
        turnRhythmScore * 0.18;
    if (consistency <= 48.0) {
        return 0.0;
    }
    return std::clamp((consistency - 48.0) * 0.58, 0.0, 18.0);
}

double DiagonalSegmentRatio(const StrokeStructureFeature& feature);

double SingleTurnDiagonalIntentBonus(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature,
    double segmentCodeScore,
    double turnAngleProfileScore,
    double singleTurnSymmetryScore) {
    if (lhsFeature.segmentDirections.size() != 2 ||
        rhsFeature.segmentDirections.size() != 2 ||
        lhsFeature.turns.size() != 1 ||
        rhsFeature.turns.size() != 1) {
        return 0.0;
    }

    const double lhsDiagonalRatio = DiagonalSegmentRatio(lhsFeature);
    const double rhsDiagonalRatio = DiagonalSegmentRatio(rhsFeature);
    if (lhsDiagonalRatio < 0.95 || rhsDiagonalRatio < 0.95) {
        return 0.0;
    }
    if (segmentCodeScore < 62.0 || turnAngleProfileScore < 58.0) {
        return 0.0;
    }

    const double consistency =
        segmentCodeScore * 0.42 +
        turnAngleProfileScore * 0.34 +
        singleTurnSymmetryScore * 0.24;
    if (consistency <= 64.0) {
        return 0.0;
    }
    return std::clamp((consistency - 64.0) * 0.20, 0.0, 8.5);
}

double DiagonalSegmentRatio(const StrokeStructureFeature& feature) {
    if (feature.segmentCodes.empty()) {
        return 0.0;
    }
    size_t diagonalCount = 0;
    for (const char code : feature.segmentCodes) {
        if (code == 'Q' || code == 'A' || code == 'C' || code == 'E') {
            ++diagonalCount;
        }
    }
    return static_cast<double>(diagonalCount) /
        static_cast<double>(feature.segmentCodes.size());
}

double StructuralMismatchPenalty(
    const StrokeStructureFeature& lhsFeature,
    const StrokeStructureFeature& rhsFeature,
    double segmentCodeScore,
    double turnSignScore,
    double turnAngleProfileScore,
    double singleTurnSymmetryScore) {
    const size_t lhsSegments = lhsFeature.segmentDirections.size();
    const size_t rhsSegments = rhsFeature.segmentDirections.size();
    const size_t lhsTurns = lhsFeature.turns.size();
    const size_t rhsTurns = rhsFeature.turns.size();

    double penalty = 0.0;
    const size_t segmentGap =
        (lhsSegments > rhsSegments) ? (lhsSegments - rhsSegments) : (rhsSegments - lhsSegments);
    const size_t turnGap =
        (lhsTurns > rhsTurns) ? (lhsTurns - rhsTurns) : (rhsTurns - lhsTurns);

    if (segmentGap >= 2) {
        penalty += std::min(28.0, 12.0 + static_cast<double>(segmentGap - 1) * 8.0);
    } else if (segmentGap == 1 && segmentCodeScore < 72.0) {
        penalty += 7.0;
    }

    if (turnGap >= 2) {
        penalty += std::min(26.0, 12.0 + static_cast<double>(turnGap - 1) * 7.0);
    } else if (turnGap == 1 && turnSignScore < 70.0) {
        penalty += 8.0;
    }

    // Separate single-turn families (V/corners/check-like shapes) from multi-turn zig-zag families.
    if ((lhsTurns == 1 && rhsTurns >= 2) || (rhsTurns == 1 && lhsTurns >= 2)) {
        penalty += 16.0;
    }

    // Keep straight lines distinct from corner/zig-zag shapes unless direction evidence is very strong.
    if ((lhsTurns == 0) != (rhsTurns == 0)) {
        penalty += 12.0;
    }

    // For two-segment families, asymmetry mismatch is a strong signal that V-like intent differs.
    if (lhsSegments == 2 && rhsSegments == 2 &&
        lhsTurns == 1 && rhsTurns == 1 &&
        singleTurnSymmetryScore < 78.0) {
        penalty += (78.0 - singleTurnSymmetryScore) * 0.16;
    }

    // If directional codes already disagree heavily, do not let shape DTW pull the score back up.
    if (segmentCodeScore < 58.0) {
        penalty += (58.0 - segmentCodeScore) * 0.34;
    }

    const double diagonalRatioGap =
        std::abs(DiagonalSegmentRatio(lhsFeature) - DiagonalSegmentRatio(rhsFeature));
    if (diagonalRatioGap > 0.34) {
        penalty += std::min(16.0, diagonalRatioGap * 20.0);
    }

    // Distinguish diagonal V-like families from orthogonal corner families.
    if (lhsSegments == 2 && rhsSegments == 2 &&
        lhsTurns == 1 && rhsTurns == 1 &&
        turnAngleProfileScore < 82.0) {
        penalty += (82.0 - turnAngleProfileScore) * 0.12;
    }

    // For zig-zag families (W-like), allow one-segment simplification drift when alternation remains strong.
    if (lhsTurns >= 2 && rhsTurns >= 2 &&
        std::abs(static_cast<int>(lhsSegments) - static_cast<int>(rhsSegments)) == 1) {
        const double lhsAlternation = TurnAlternationRatio(lhsFeature);
        const double rhsAlternation = TurnAlternationRatio(rhsFeature);
        const double alternationMin = std::min(lhsAlternation, rhsAlternation);
        if (alternationMin >= 0.85 &&
            turnAngleProfileScore >= 72.0 &&
            segmentCodeScore >= 58.0) {
            penalty -= 7.0;
        }
    }

    return std::clamp(penalty, 0.0, 42.0);
}

double StrokeSimilarityScore(
    const std::vector<NormalizedPoint>& lhs,
    const std::vector<NormalizedPoint>& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return -1.0;
    }
    const double shapeScore = DtwShapeScore(lhs, rhs);
    const double directionScore = DirectionSequenceScore(lhs, rhs);
    const double tangentScore = TangentScore(lhs, rhs);
    const StrokeStructureFeature lhsStructure = BuildStrokeStructureFeature(lhs);
    const StrokeStructureFeature rhsStructure = BuildStrokeStructureFeature(rhs);
    const double structureScore = StructureScore(lhsStructure, rhsStructure);
    if (shapeScore < 0.0) {
        return -1.0;
    }
    double finalScore = std::clamp(
        shapeScore * 0.30 +
        directionScore * 0.18 +
        tangentScore * 0.10 +
        structureScore * 0.42,
        0.0,
        100.0);
    const double singleTurnSymmetryScore = SingleTurnSymmetryScore(lhsStructure, rhsStructure);
    const double segmentCodeScore = SegmentCodeSequenceScore(lhsStructure, rhsStructure);
    const double turnSignScore = TurnSignScore(lhsStructure, rhsStructure);
    const double turnAngleProfileScore = TurnAngleProfileScore(lhsStructure, rhsStructure);
    if (lhsStructure.segmentDirections.size() == 2 &&
        rhsStructure.segmentDirections.size() == 2 &&
        lhsStructure.turns.size() == 1 &&
        rhsStructure.turns.size() == 1) {
        const double symmetryFactor = 0.72 + (std::clamp(singleTurnSymmetryScore, 0.0, 100.0) / 100.0) * 0.28;
        finalScore *= symmetryFactor;
    }
    const double straightCompatibilityScore =
        StraightStrokeCompatibilityScore(lhs, rhs, lhsStructure, rhsStructure);
    if (straightCompatibilityScore >= 0.0) {
        finalScore = std::max(finalScore, straightCompatibilityScore * 0.82 + directionScore * 0.18);
    }
    finalScore += MultiTurnPatternBonus(lhsStructure, rhsStructure);
    finalScore += MultiSegmentIntentBonus(lhsStructure, rhsStructure, directionScore);
    finalScore += ZigZagFamilyIntentBonus(lhsStructure, rhsStructure);
    finalScore += ZigZagStrongMultiTurnBonus(lhsStructure, rhsStructure);
    finalScore += SingleTurnDiagonalIntentBonus(
        lhsStructure,
        rhsStructure,
        segmentCodeScore,
        turnAngleProfileScore,
        singleTurnSymmetryScore);
    finalScore -= StructuralMismatchPenalty(
        lhsStructure,
        rhsStructure,
        segmentCodeScore,
        turnSignScore,
        turnAngleProfileScore,
        singleTurnSymmetryScore);
    return std::clamp(finalScore, 0.0, 100.0);
}

double GestureSimilarityScore(
    const std::vector<std::vector<NormalizedPoint>>& templateStrokes,
    const std::vector<std::vector<NormalizedPoint>>& capturedStrokes) {
    if (templateStrokes.empty() ||
        capturedStrokes.empty() ||
        templateStrokes.size() != capturedStrokes.size()) {
        return -1.0;
    }

    double weightedScore = 0.0;
    double totalWeight = 0.0;
    for (size_t i = 0; i < templateStrokes.size(); ++i) {
        const double strokeScore = StrokeSimilarityScore(templateStrokes[i], capturedStrokes[i]);
        if (strokeScore < 0.0) {
            return -1.0;
        }
        const double weight = std::max(1.0, std::max(
            StrokeLength(templateStrokes[i]),
            StrokeLength(capturedStrokes[i])));
        weightedScore += strokeScore * weight;
        totalWeight += weight;
    }
    if (!(totalWeight > 0.0)) {
        return -1.0;
    }
    return weightedScore / totalWeight;
}

double StrokeLengthPx(const std::vector<ScreenPoint>& stroke) {
    if (stroke.size() < 2) {
        return 0.0;
    }
    double length = 0.0;
    for (size_t i = 1; i < stroke.size(); ++i) {
        length += PointDistance(stroke[i - 1], stroke[i]);
    }
    return length;
}

struct StrokeWindowMatch final {
    double bestScore = -1.0;
    double runnerUpScore = -1.0;
    size_t bestWindowStart = 0;
    size_t bestWindowEnd = 0;
    size_t candidateCount = 0;
};

std::vector<ScreenPoint> SliceStroke(
    const std::vector<ScreenPoint>& stroke,
    size_t beginIndex,
    size_t endIndex) {
    if (beginIndex >= endIndex || endIndex > stroke.size()) {
        return {};
    }
    return std::vector<ScreenPoint>(stroke.begin() + static_cast<std::ptrdiff_t>(beginIndex),
                                    stroke.begin() + static_cast<std::ptrdiff_t>(endIndex));
}

bool HasAlignedStrokeTimes(
    const std::vector<ScreenPoint>& stroke,
    const std::vector<uint32_t>* strokeTimesMs) {
    return strokeTimesMs != nullptr &&
        strokeTimesMs->size() == stroke.size() &&
        strokeTimesMs->size() >= 2;
}

std::pair<size_t, size_t> TimeWindowToPointWindow(
    const std::vector<uint32_t>& strokeTimesMs,
    uint32_t windowStartMs,
    uint32_t windowEndMs) {
    if (strokeTimesMs.empty() || windowStartMs >= windowEndMs) {
        return {0, 0};
    }
    const auto first = strokeTimesMs.begin();
    const auto last = strokeTimesMs.end();
    const auto beginIt = std::lower_bound(first, last, windowStartMs);
    const auto endIt = std::upper_bound(first, last, windowEndMs);
    if (beginIt == last || endIt == first || beginIt >= endIt) {
        return {0, 0};
    }
    const size_t beginIndex = static_cast<size_t>(std::distance(first, beginIt));
    const size_t endIndex = static_cast<size_t>(std::distance(first, endIt));
    return {beginIndex, endIndex};
}

StrokeWindowMatch MatchSingleStroke(
    const std::vector<AutomationKeyBinding::GesturePoint>& templateStroke,
    const std::vector<ScreenPoint>& capturedStroke,
    const std::vector<uint32_t>* capturedStrokeTimesMs,
    const GestureMatchOptions& options) {
    StrokeWindowMatch match{};
    if (templateStroke.size() < 2 || capturedStroke.size() < 2) {
        return match;
    }
    if (options.minEffectiveStrokeLengthPx > 0.0 &&
        StrokeLengthPx(capturedStroke) + 1e-6 < options.minEffectiveStrokeLengthPx) {
        return match;
    }

    const std::vector<std::vector<NormalizedPoint>> normalizedTemplate =
        NormalizeTemplateStrokes({templateStroke});
    if (normalizedTemplate.size() != 1 || normalizedTemplate[0].size() < 2) {
        return match;
    }
    const StrokeStructureFeature templateStructure = BuildStrokeStructureFeature(normalizedTemplate[0]);
    const std::vector<std::vector<NormalizedPoint>> normalizedCapturedFull =
        NormalizeCapturedStrokes({capturedStroke});
    const StrokeStructureFeature capturedFullStructure =
        (normalizedCapturedFull.size() == 1 && normalizedCapturedFull[0].size() >= 2)
            ? BuildStrokeStructureFeature(normalizedCapturedFull[0])
            : StrokeStructureFeature{};

    std::unordered_set<uint64_t> visitedWindows;
    auto evaluateCandidate = [&](size_t beginIndex, size_t endIndex) {
        if (beginIndex >= endIndex || endIndex > capturedStroke.size()) {
            return;
        }
        const uint64_t windowKey =
            (static_cast<uint64_t>(beginIndex) << 32) |
            static_cast<uint64_t>(endIndex);
        if (!visitedWindows.insert(windowKey).second) {
            return;
        }
        const std::vector<ScreenPoint> sliced = SliceStroke(capturedStroke, beginIndex, endIndex);
        if (sliced.size() < 2) {
            return;
        }
        const std::vector<std::vector<NormalizedPoint>> normalizedCaptured =
            NormalizeCapturedStrokes({sliced});
        if (normalizedCaptured.size() != 1 || normalizedCaptured[0].size() < 2) {
            return;
        }
        double score = GestureSimilarityScore(normalizedTemplate, normalizedCaptured);
        if (score < 0.0) {
            return;
        }
        const double coverageRatio =
            static_cast<double>(sliced.size()) / static_cast<double>(capturedStroke.size());
        const size_t templateTurns = templateStructure.turns.size();
        const size_t capturedFullTurns = capturedFullStructure.turns.size();
        if (capturedFullTurns >= templateTurns + 2 && coverageRatio < 0.85) {
            const size_t extraTurns = capturedFullTurns - templateTurns;
            const double complexityPenalty =
                static_cast<double>(std::max<size_t>(1, extraTurns - 1)) * 6.0 +
                (0.85 - coverageRatio) * 20.0;
            score -= std::clamp(complexityPenalty, 0.0, 24.0);
        }

        ++match.candidateCount;
        if (score > match.bestScore + 1e-6) {
            match.runnerUpScore = match.bestScore;
            match.bestScore = score;
            match.bestWindowStart = beginIndex;
            match.bestWindowEnd = endIndex;
        } else if (score > match.runnerUpScore + 1e-6) {
            match.runnerUpScore = score;
        }
    };

    if (!options.enableWindowSearch || capturedStroke.size() <= 10) {
        evaluateCandidate(0, capturedStroke.size());
        return match;
    }

    if (options.enableTimeWindowSearch && HasAlignedStrokeTimes(capturedStroke, capturedStrokeTimesMs)) {
        const std::vector<uint32_t>& strokeTimes = *capturedStrokeTimesMs;
        const uint32_t totalDurationMs = strokeTimes.back();
        const uint32_t minWindowMs = static_cast<uint32_t>(std::max(60, options.timeWindowMinMs));
        const uint32_t maxWindowMs = static_cast<uint32_t>(std::max(
            static_cast<int>(minWindowMs),
            std::min<int>(options.timeWindowMaxMs, static_cast<int>(totalDurationMs))));
        const uint32_t durationSpanMs = (maxWindowMs > minWindowMs) ? (maxWindowMs - minWindowMs) : 0;
        const uint32_t durationStepMs = static_cast<uint32_t>(std::max(
            options.timeWindowStepMs,
            static_cast<int>(durationSpanMs / 6 + 1)));
        const uint32_t anchorStepMs = static_cast<uint32_t>(std::max(
            options.timeWindowAnchorStepMs,
            static_cast<int>(totalDurationMs / 14 + 1)));
        const size_t maxTimeCandidates = static_cast<size_t>(std::max(8, options.timeWindowMaxCandidates));
        size_t emittedTimeCandidates = 0;
        for (uint32_t windowMs = minWindowMs; windowMs <= maxWindowMs; windowMs += durationStepMs) {
            if (emittedTimeCandidates >= maxTimeCandidates) {
                break;
            }
            uint32_t startMs = 0;
            for (;; startMs += anchorStepMs) {
                if (emittedTimeCandidates >= maxTimeCandidates) {
                    break;
                }
                uint32_t endMs = startMs + windowMs;
                if (endMs > totalDurationMs) {
                    endMs = totalDurationMs;
                    startMs = (endMs > windowMs) ? (endMs - windowMs) : 0;
                }
                const auto [beginIndex, endIndex] = TimeWindowToPointWindow(strokeTimes, startMs, endMs);
                if (endIndex > beginIndex + 1) {
                    evaluateCandidate(beginIndex, endIndex);
                    ++emittedTimeCandidates;
                }
                if (endMs >= totalDurationMs) {
                    break;
                }
            }
        }
    }

    const size_t totalPoints = capturedStroke.size();
    const int minCoverage = std::clamp(options.windowCoverageMinPercent, 10, 100);
    const int maxCoverage = std::clamp(options.windowCoverageMaxPercent, minCoverage, 100);
    const int coverageStep = std::clamp(options.windowCoverageStepPercent, 1, 30);
    const int slideDivisor = std::clamp(options.windowSlideDivisor, 1, 8);
    for (int coverage = minCoverage; coverage <= maxCoverage; coverage += coverageStep) {
        size_t windowSize = static_cast<size_t>(std::round(
            static_cast<double>(totalPoints) * static_cast<double>(coverage) / 100.0));
        windowSize = std::clamp(windowSize, static_cast<size_t>(6), totalPoints);
        const size_t step = std::max<size_t>(1, windowSize / static_cast<size_t>(slideDivisor));
        size_t start = 0;
        for (; start + windowSize <= totalPoints; start += step) {
            evaluateCandidate(start, start + windowSize);
        }
        const size_t tailStart = totalPoints - windowSize;
        if (start == 0 || tailStart + windowSize > start) {
            evaluateCandidate(tailStart, tailStart + windowSize);
        }
    }
    if (maxCoverage < 100) {
        evaluateCandidate(0, totalPoints);
    }
    if (match.candidateCount == 0) {
        evaluateCandidate(0, capturedStroke.size());
    }
    return match;
}

GestureMatchResult MatchMultiStroke(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes,
    const std::vector<std::vector<uint32_t>>* capturedStrokeTimesMs,
    const GestureMatchOptions& options) {
    GestureMatchResult out{};
    if (templateStrokes.empty() || capturedStrokes.empty()) {
        return out;
    }
    if (options.strictStrokeCount && templateStrokes.size() != capturedStrokes.size()) {
        return out;
    }
    if (options.strictStrokeOrder && templateStrokes.size() != capturedStrokes.size()) {
        return out;
    }
    if (templateStrokes.size() != capturedStrokes.size()) {
        return out;
    }

    std::vector<size_t> strokeOffsets;
    strokeOffsets.reserve(capturedStrokes.size());
    size_t offset = 0;
    for (const auto& stroke : capturedStrokes) {
        strokeOffsets.push_back(offset);
        offset += stroke.size();
    }

    double weightedBest = 0.0;
    double totalWeight = 0.0;
    std::vector<double> strokeBestScores;
    std::vector<double> strokeRunnerUpScores;
    std::vector<double> strokeWeights;
    strokeBestScores.reserve(templateStrokes.size());
    strokeRunnerUpScores.reserve(templateStrokes.size());
    strokeWeights.reserve(templateStrokes.size());

    for (size_t i = 0; i < templateStrokes.size(); ++i) {
        const StrokeWindowMatch strokeMatch = MatchSingleStroke(
            templateStrokes[i],
            capturedStrokes[i],
            (capturedStrokeTimesMs && i < capturedStrokeTimesMs->size()) ? &(*capturedStrokeTimesMs)[i] : nullptr,
            options);
        out.candidateCount += strokeMatch.candidateCount;
        if (strokeMatch.bestScore < 0.0) {
            return GestureMatchResult{};
        }
        const double weight = std::max(1.0, StrokeLengthPx(capturedStrokes[i]));
        weightedBest += strokeMatch.bestScore * weight;
        totalWeight += weight;
        strokeBestScores.push_back(strokeMatch.bestScore);
        strokeRunnerUpScores.push_back(strokeMatch.runnerUpScore);
        strokeWeights.push_back(weight);
        if (i == 0) {
            out.bestWindow.start = strokeOffsets[i] + strokeMatch.bestWindowStart;
        }
        if (i + 1 == templateStrokes.size()) {
            out.bestWindow.end = strokeOffsets[i] + strokeMatch.bestWindowEnd;
        }
    }

    if (!(totalWeight > 0.0)) {
        return GestureMatchResult{};
    }
    out.bestScore = weightedBest / totalWeight;

    double bestAlt = -1.0;
    for (size_t i = 0; i < strokeBestScores.size(); ++i) {
        if (strokeRunnerUpScores[i] < 0.0) {
            continue;
        }
        const double altWeighted =
            weightedBest - strokeBestScores[i] * strokeWeights[i] + strokeRunnerUpScores[i] * strokeWeights[i];
        bestAlt = std::max(bestAlt, altWeighted / totalWeight);
    }
    out.runnerUpScore = bestAlt;
    return out;
}

std::vector<std::vector<AutomationKeyBinding::GesturePoint>> PresetTemplateStrokesForActionId(
    const std::string& normalizedActionId) {
    const std::string id = TrimAscii(normalizedActionId);
    if (id == "right") {
        return {{{10, 50}, {90, 50}}};
    }
    if (id == "left") {
        return {{{90, 50}, {10, 50}}};
    }
    if (id == "up") {
        return {{{50, 85}, {50, 15}}};
    }
    if (id == "down") {
        return {{{50, 15}, {50, 85}}};
    }
    if (id == "diag_down_right") {
        return {{{20, 82}, {80, 18}}};
    }
    if (id == "diag_down_left") {
        return {{{20, 18}, {80, 82}}};
    }
    if (id == "diag_down_right_diag_up_right") {
        return {{{16, 18}, {50, 82}, {84, 18}}};
    }
    if (id == "diag_down_right_diag_up_right_diag_down_right") {
        return {{{8, 18}, {28, 82}, {50, 36}, {72, 82}, {92, 18}}};
    }
    if (id == "up_right") {
        return {{{34, 82}, {34, 22}, {80, 22}}};
    }
    if (id == "up_left") {
        return {{{66, 82}, {66, 22}, {20, 22}}};
    }
    if (id == "down_right") {
        return {{{34, 18}, {34, 78}, {80, 78}}};
    }
    if (id == "down_left") {
        return {{{66, 18}, {66, 78}, {20, 78}}};
    }
    return {};
}

} // namespace

bool IsCustomGesturePatternMode(const std::string& mode) {
    return ToLowerAscii(TrimAscii(mode)) == "custom";
}

std::vector<std::vector<AutomationKeyBinding::GesturePoint>> GestureTemplateStrokesFromPattern(
    const AutomationKeyBinding::GesturePattern& pattern) {
    if (!pattern.customStrokes.empty()) {
        return pattern.customStrokes;
    }
    if (!pattern.customPoints.empty()) {
        return {pattern.customPoints};
    }
    return {};
}

GestureSimilarityMetrics MeasureCapturedGesture(
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes) {
    GestureSimilarityMetrics metrics;
    metrics.strokeCount = capturedStrokes.size();
    for (const auto& stroke : capturedStrokes) {
        metrics.pointCount += stroke.size();
        if (stroke.size() >= 2) {
            metrics.startEndDistancePx += PointDistance(stroke.front(), stroke.back());
        }
        for (size_t i = 1; i < stroke.size(); ++i) {
            metrics.pathLengthPx += PointDistance(stroke[i - 1], stroke[i]);
        }
    }
    return metrics;
}

GestureTemplateProfile MeasureGestureTemplateProfile(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes) {
    GestureTemplateProfile profile;
    const std::vector<std::vector<NormalizedPoint>> normalized =
        NormalizeTemplateStrokes(templateStrokes);
    profile.strokeCount = normalized.size();
    for (const auto& stroke : normalized) {
        profile.pointCount += stroke.size();
        const StrokeStructureFeature structure = BuildStrokeStructureFeature(stroke);
        profile.segmentCount += structure.segmentDirections.size();
        profile.turnCount += structure.turns.size();
    }
    return profile;
}

GestureTemplateProfile MeasurePresetGestureProfile(
    const std::string& normalizedActionId) {
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokes =
        PresetTemplateStrokesForActionId(normalizedActionId);
    if (templateStrokes.empty()) {
        return {};
    }
    return MeasureGestureTemplateProfile(templateStrokes);
}

double ScoreGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes) {
    return MatchGestureTemplateSimilarity(templateStrokes, capturedStrokes).bestScore;
}

GestureMatchResult MatchGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes,
    const GestureMatchOptions& options) {
    return MatchGestureTemplateSimilarity(templateStrokes, capturedStrokes, {}, options);
}

GestureMatchResult MatchGestureTemplateSimilarity(
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>>& templateStrokes,
    const std::vector<std::vector<ScreenPoint>>& capturedStrokes,
    const std::vector<std::vector<uint32_t>>& capturedStrokeTimesMs,
    const GestureMatchOptions& options) {
    GestureMatchResult out{};
    if (templateStrokes.empty() || capturedStrokes.empty()) {
        return out;
    }
    const std::vector<std::vector<uint32_t>>* strokeTimes = nullptr;
    if (!capturedStrokeTimesMs.empty() && capturedStrokeTimesMs.size() == capturedStrokes.size()) {
        strokeTimes = &capturedStrokeTimesMs;
    }
    if (templateStrokes.size() == 1 && capturedStrokes.size() == 1) {
        const StrokeWindowMatch match = MatchSingleStroke(
            templateStrokes[0],
            capturedStrokes[0],
            (strokeTimes != nullptr) ? &(*strokeTimes)[0] : nullptr,
            options);
        out.bestScore = match.bestScore;
        out.runnerUpScore = match.runnerUpScore;
        out.bestWindow.start = match.bestWindowStart;
        out.bestWindow.end = match.bestWindowEnd;
        out.candidateCount = match.candidateCount;
        return out;
    }
    return MatchMultiStroke(templateStrokes, capturedStrokes, strokeTimes, options);
}

double ScorePresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke) {
    return MatchPresetGestureSimilarity(normalizedActionId, capturedStroke).bestScore;
}

GestureMatchResult MatchPresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke,
    const GestureMatchOptions& options) {
    return MatchPresetGestureSimilarity(normalizedActionId, capturedStroke, {}, options);
}

GestureMatchResult MatchPresetGestureSimilarity(
    const std::string& normalizedActionId,
    const std::vector<ScreenPoint>& capturedStroke,
    const std::vector<uint32_t>& capturedStrokeTimesMs,
    const GestureMatchOptions& options) {
    const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokes =
        PresetTemplateStrokesForActionId(normalizedActionId);
    if (templateStrokes.empty() || capturedStroke.size() < 2) {
        return {};
    }
    if (!capturedStrokeTimesMs.empty() && capturedStrokeTimesMs.size() == capturedStroke.size()) {
        return MatchGestureTemplateSimilarity(
            templateStrokes,
            {capturedStroke},
            {capturedStrokeTimesMs},
            options);
    }
    return MatchGestureTemplateSimilarity(templateStrokes, {capturedStroke}, options);
}

} // namespace mousefx
