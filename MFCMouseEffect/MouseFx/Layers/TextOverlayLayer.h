#pragma once

#include "MouseFx/Interfaces/IOverlayLayer.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>
#include <vector>

namespace mousefx {

class TextOverlayLayer final : public IOverlayLayer {
public:
    TextOverlayLayer() = default;
    ~TextOverlayLayer() override = default;

    void ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config);

    void Update(uint64_t nowMs) override;
    void Render(Gdiplus::Graphics& graphics) override;
    bool IsAlive() const override { return true; }

private:
    struct TextInstance {
        ScreenPoint startPt{};
        std::wstring text{};
        Argb color{};
        TextConfig config{};
        uint64_t startTick = 0;
        float driftX = 0.0f;
        float swayFreq = 1.0f;
        float swayAmp = 5.0f;
        bool active = false;
    };

    static float EaseOutCubic(float t);
    static std::wstring ResolveFontFamilyName(const TextConfig& config, const std::wstring& text);
    static std::wstring EnsureFontFamily(const std::wstring& name);
    static Gdiplus::Color ToGdiPlus(Argb color, BYTE alpha);

    std::vector<TextInstance> instances_{};
};

} // namespace mousefx
