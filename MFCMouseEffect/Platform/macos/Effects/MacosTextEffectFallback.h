#pragma once

#include "MouseFx/Interfaces/ITextEffectFallback.h"

#include <cstddef>

namespace mousefx {

class MacosTextEffectFallback final : public ITextEffectFallback {
public:
    bool EnsureInitialized(size_t count) override;
    void Shutdown() override;
    void ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) override;
    void ShowTextComputed(const ScreenPoint& anchorPoint, const TextEffectRenderCommand& command) override;

private:
    size_t maxConcurrentWindows_ = 8;
};

} // namespace mousefx
