#include "pch.h"

#include "MouseFx/Core/Overlay/InputIndicatorPositionResolver.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformDisplayTopology.h"

#include <algorithm>
#include <unordered_map>

namespace mousefx {
namespace {

constexpr int kMinSizePx = 40;
constexpr int kMaxSizePx = 200;
constexpr int kMinDurationMs = 120;
constexpr int kMaxDurationMs = 2000;

InputIndicatorAnchor BuildAnchor(const ScreenPoint& topLeft, int sizePx, int durationMs) {
    const int clampedSize = std::clamp(sizePx, kMinSizePx, kMaxSizePx);
    InputIndicatorAnchor anchor{};
    anchor.topLeft = topLeft;
    anchor.sizePx = clampedSize;
    anchor.durationMs = std::clamp(durationMs, kMinDurationMs, kMaxDurationMs);
    anchor.center.x = topLeft.x + clampedSize / 2;
    anchor.center.y = topLeft.y + clampedSize / 2;
    return anchor;
}

bool IsCustomTarget(const std::string& targetMonitor) {
    return ToLowerAscii(TrimAscii(targetMonitor)) == "custom";
}

} // namespace

std::vector<InputIndicatorAnchor> ResolveInputIndicatorAnchors(
    const InputIndicatorConfig& cfg,
    const ScreenPoint& anchorPt) {
    const InputIndicatorConfig sanitized = config_internal::SanitizeInputIndicatorConfig(cfg);
    std::vector<InputIndicatorAnchor> anchors;

    if (sanitized.positionMode == "relative") {
        ScreenPoint topLeft{};
        topLeft.x = anchorPt.x + sanitized.offsetX;
        topLeft.y = anchorPt.y + sanitized.offsetY;
        anchors.emplace_back(BuildAnchor(topLeft, sanitized.sizePx, sanitized.durationMs));
        return anchors;
    }

    if (IsCustomTarget(sanitized.targetMonitor)) {
        const auto monitors = platform::EnumerateDisplayMonitors();
        if (monitors.empty()) {
            ScreenPoint topLeft{ sanitized.absoluteX, sanitized.absoluteY };
            anchors.emplace_back(BuildAnchor(topLeft, sanitized.sizePx, sanitized.durationMs));
            return anchors;
        }

        std::unordered_map<std::string, platform::DisplayRect> lookup;
        lookup.reserve(monitors.size());
        for (const auto& monitor : monitors) {
            lookup.emplace(monitor.id, monitor.bounds);
        }

        for (const auto& [id, overrideCfg] : sanitized.perMonitorOverrides) {
            if (!overrideCfg.enabled) {
                continue;
            }
            const auto it = lookup.find(id);
            if (it == lookup.end()) {
                continue;
            }
            const platform::DisplayRect rect = it->second;
            ScreenPoint topLeft{};
            topLeft.x = rect.left + overrideCfg.absoluteX;
            topLeft.y = rect.top + overrideCfg.absoluteY;
            anchors.emplace_back(BuildAnchor(topLeft, sanitized.sizePx, sanitized.durationMs));
        }
        return anchors;
    }

    const platform::DisplayPoint anchorPoint{ anchorPt.x, anchorPt.y };
    const auto [resolvedId, rect] = platform::ResolveTargetDisplayMonitor(
        sanitized.targetMonitor, anchorPoint);

    int absX = sanitized.absoluteX;
    int absY = sanitized.absoluteY;
    if (!resolvedId.empty()) {
        const auto it = sanitized.perMonitorOverrides.find(resolvedId);
        if (it != sanitized.perMonitorOverrides.end() && it->second.enabled) {
            absX = it->second.absoluteX;
            absY = it->second.absoluteY;
        }
    }

    ScreenPoint topLeft{};
    topLeft.x = rect.left + absX;
    topLeft.y = rect.top + absY;
    anchors.emplace_back(BuildAnchor(topLeft, sanitized.sizePx, sanitized.durationMs));
    return anchors;
}

} // namespace mousefx
