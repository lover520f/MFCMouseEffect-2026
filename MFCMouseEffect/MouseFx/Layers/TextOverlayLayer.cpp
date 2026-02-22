#include "pch.h"

#include "TextOverlayLayer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Settings/EmojiUtils.h"

#include <algorithm>
#include <cmath>

namespace mousefx {


float TextOverlayLayer::EaseOutCubic(float t) {
    float u = 1.0f - t;
    return 1.0f - (u * u * u);
}



std::wstring TextOverlayLayer::ResolveFontFamilyName(const TextConfig& config, const std::wstring& text) {
    if (settings::HasEmojiStarter(text)) {
        return L"Segoe UI Emoji";
    }
    if (!config.fontFamily.empty()) {
        return config.fontFamily;
    }
    return L"Segoe UI";
}

std::wstring TextOverlayLayer::EnsureFontFamily(const std::wstring& name) {
    Gdiplus::FontFamily family(name.c_str());
    if (family.IsAvailable()) return name;
    Gdiplus::FontFamily yahei(L"Microsoft YaHei");
    if (yahei.IsAvailable()) return L"Microsoft YaHei";
    Gdiplus::FontFamily segoe(L"Segoe UI");
    if (segoe.IsAvailable()) return L"Segoe UI";
    return L"Arial";
}

Gdiplus::Color TextOverlayLayer::ToGdiPlus(Argb color, BYTE alpha) {
    return Gdiplus::Color(alpha,
        (BYTE)((color.value >> 16) & 0xFF),
        (BYTE)((color.value >> 8) & 0xFF),
        (BYTE)(color.value & 0xFF));
}

void TextOverlayLayer::ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    if (text.empty()) return;
    TextInstance instance{};
    instance.startPt = pt;
    instance.text = text;
    instance.color = color;
    instance.config = config;
    instance.startTick = NowMs();
    instance.driftX = (float)(rand() % 100 - 50);
    instance.swayFreq = 1.0f + (float)(rand() % 200) / 100.0f;
    instance.swayAmp = 5.0f + (float)(rand() % 100) / 10.0f;
    instance.active = true;
    instances_.push_back(std::move(instance));
}

void TextOverlayLayer::Update(uint64_t nowMs) {
    for (auto& instance : instances_) {
        if (!instance.active) continue;
        const uint64_t elapsed = (nowMs >= instance.startTick) ? (nowMs - instance.startTick) : 0;
        const uint32_t duration = (instance.config.durationMs <= 0) ? 1u : (uint32_t)instance.config.durationMs;
        const float t = (float)elapsed / (float)duration;
        if (t >= 1.0f) {
            instance.active = false;
        }
    }
    instances_.erase(
        std::remove_if(
            instances_.begin(),
            instances_.end(),
            [](const TextInstance& instance) { return !instance.active; }),
        instances_.end());
}

void TextOverlayLayer::Render(Gdiplus::Graphics& graphics) {
    if (instances_.empty()) return;

    for (const auto& instance : instances_) {
        if (!instance.active) continue;

        const uint64_t elapsed = NowMs() - instance.startTick;
        const uint32_t duration = (instance.config.durationMs <= 0) ? 1u : (uint32_t)instance.config.durationMs;
        float t = (float)elapsed / (float)duration;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        const float eased = EaseOutCubic(t);
        const float yOffset = eased * (float)instance.config.floatDistance;
        const float xOffset = (t * instance.driftX) + std::sin(t * 3.1415926f * instance.swayFreq) * instance.swayAmp;

        float scale = 1.0f;
        if (t < 0.3f) {
            scale = 0.8f + (t / 0.3f) * 0.4f;
        } else {
            scale = 1.2f - ((t - 0.3f) / 0.7f) * 0.2f;
        }

        float alphaFactor = 1.0f;
        if (t < 0.15f) {
            alphaFactor = t / 0.15f;
        } else if (t > 0.6f) {
            alphaFactor = 1.0f - (t - 0.6f) / 0.4f;
        }
        if (alphaFactor < 0.0f) alphaFactor = 0.0f;
        if (alphaFactor > 1.0f) alphaFactor = 1.0f;

        const BYTE alpha = (BYTE)(255.0f * alphaFactor);
        const Gdiplus::Color textColor = ToGdiPlus(instance.color, alpha);
        Gdiplus::SolidBrush brush(textColor);

        std::wstring family = EnsureFontFamily(ResolveFontFamilyName(instance.config, instance.text));
        float fontSizePt = instance.config.fontSize * scale;
        if (fontSizePt < 6.0f) fontSizePt = 6.0f;
        const bool hasEmoji = settings::HasEmojiStarter(instance.text);
        const int fontStyle = hasEmoji ? Gdiplus::FontStyleRegular : Gdiplus::FontStyleBold;
        Gdiplus::Font font(family.c_str(), fontSizePt, fontStyle, Gdiplus::UnitPoint);

        const ScreenPoint startPt = ScreenToOverlayPoint(instance.startPt);
        const float x = (float)startPt.x + xOffset;
        const float y = (float)startPt.y - yOffset;
        const Gdiplus::RectF layout(x - 120.0f, y - 60.0f, 240.0f, 120.0f);
        Gdiplus::StringFormat format;
        format.SetAlignment(Gdiplus::StringAlignmentCenter);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        graphics.DrawString(instance.text.c_str(), -1, &font, layout, &format, &brush);
    }
}

} // namespace mousefx
