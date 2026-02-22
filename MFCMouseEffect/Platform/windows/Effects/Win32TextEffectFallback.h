#pragma once

#include "MouseFx/Interfaces/ITextEffectFallback.h"
#include "Platform/windows/Effects/TextWindowPool.h"

namespace mousefx {

class Win32TextEffectFallback final : public ITextEffectFallback {
public:
    bool EnsureInitialized(size_t count) override;
    void Shutdown() override;
    void ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) override;

private:
    TextWindowPool pool_{};
};

} // namespace mousefx
