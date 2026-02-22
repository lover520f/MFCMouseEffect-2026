#pragma once

#include "MouseFx/Interfaces/ITextEffectFallback.h"

namespace mousefx {

class NullTextEffectFallback final : public ITextEffectFallback {
public:
    bool EnsureInitialized(size_t) override { return false; }
    void Shutdown() override {}
    void ShowText(const ScreenPoint&, const std::wstring&, Argb, const TextConfig&) override {}
};

} // namespace mousefx
