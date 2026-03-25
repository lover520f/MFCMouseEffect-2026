#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

#include <algorithm>
#include <cmath>

namespace mousefx {
namespace {

bool HasCursorDecorationEnabled(const InputIndicatorConfig& config) {
    return config.cursorDecoration.enabled;
}

struct CursorDecorationFrame final {
    int widthPx = 72;
    int heightPx = 72;
    int anchorOffsetXPx = 36;
    int anchorOffsetYPx = 36;
};

CursorDecorationFrame ResolveCursorDecorationFrame(
    const InputIndicatorConfig::CursorDecorationConfig& config) {
    const int sizePx = std::clamp(config.sizePx, 12, 72);
    CursorDecorationFrame frame{};
    frame.widthPx = std::max(36, sizePx * 4);
    frame.heightPx = std::max(36, sizePx * 4);
    frame.anchorOffsetXPx = frame.widthPx / 2;
    frame.anchorOffsetYPx = frame.heightPx / 2;
    return frame;
}

ScreenPoint ResolveCursorDecorationOrigin(
    const ScreenPoint& cursorPt,
    const InputIndicatorConfig::CursorDecorationConfig& config) {
    const CursorDecorationFrame frame = ResolveCursorDecorationFrame(config);
    const ScreenPoint overlayCursor = ScreenToOverlayPoint(cursorPt);
    ScreenPoint origin{};
    origin.x = overlayCursor.x - frame.anchorOffsetXPx;
    origin.y = overlayCursor.y - (frame.heightPx - frame.anchorOffsetYPx);
    return origin;
}

} // namespace

MacosInputIndicatorOverlay::~MacosInputIndicatorOverlay() {
    Shutdown();
}

void MacosInputIndicatorOverlay::UpdateConfig(const InputIndicatorConfig& cfg) {
    bool shouldShowDecoration = false;
    ScreenPoint decorationPoint{};
    InputIndicatorConfig::CursorDecorationConfig decorationConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = cfg;
        shouldShowDecoration = initialized_ && HasCursorDecorationEnabled(config_) && hasCursorPoint_;
        if (shouldShowDecoration) {
            decorationPoint = cursorPoint_;
            decorationConfig = config_.cursorDecoration;
        }
    }
    if (!HasCursorDecorationEnabled(cfg)) {
        macos_input_indicator::RunOnMainThreadAsync(^{
          macos_input_indicator_style::HideDecorationPanel(decorationPanel_);
        });
        return;
    }
    if (!shouldShowDecoration) {
        return;
    }
    const ScreenPoint frameOrigin = ResolveCursorDecorationOrigin(decorationPoint, decorationConfig);
    const std::string pluginId = decorationConfig.pluginId;
    const std::string colorHex = decorationConfig.colorHex;
    const int sizePx = decorationConfig.sizePx;
    const int alphaPercent = decorationConfig.alphaPercent;
    macos_input_indicator::RunOnMainThreadAsync(^{
      macos_input_indicator_style::ApplyDecorationPanelPresentation(
          decorationPanel_,
          frameOrigin.x,
          frameOrigin.y,
          pluginId,
          colorHex,
          sizePx,
          alphaPercent);
    });
}

bool MacosInputIndicatorOverlay::ShouldShowKeyboard() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && config_.enabled && config_.keyboardEnabled;
}

void MacosInputIndicatorOverlay::OnMove(const ScreenPoint& pt) {
    InputIndicatorConfig::CursorDecorationConfig decorationConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cursorPoint_ = pt;
        hasCursorPoint_ = true;
        if (!HasCursorDecorationEnabled(config_)) {
            return;
        }
        decorationConfig = config_.cursorDecoration;
    }

    if (!Initialize()) {
        return;
    }

    const ScreenPoint frameOrigin = ResolveCursorDecorationOrigin(pt, decorationConfig);
    const std::string pluginId = decorationConfig.pluginId;
    const std::string colorHex = decorationConfig.colorHex;
    const int sizePx = decorationConfig.sizePx;
    const int alphaPercent = decorationConfig.alphaPercent;
    macos_input_indicator::RunOnMainThreadAsync(^{
      macos_input_indicator_style::ApplyDecorationPanelPresentation(
          decorationPanel_,
          frameOrigin.x,
          frameOrigin.y,
          pluginId,
          colorHex,
          sizePx,
          alphaPercent);
    });
}

} // namespace mousefx
