#pragma once

#include <cstddef>
#include <string>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class ITextEffectFallback {
public:
    virtual ~ITextEffectFallback() = default;

    virtual bool EnsureInitialized(size_t count) = 0;
    virtual void Shutdown() = 0;
    virtual void ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) = 0;
};

} // namespace mousefx
