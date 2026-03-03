#pragma once

#include <memory>
#include <vector>

#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "TextWindow.h"

namespace mousefx {

class TextWindowPool final {
public:
    TextWindowPool() = default;
    ~TextWindowPool() = default;

    TextWindowPool(const TextWindowPool&) = delete;
    TextWindowPool& operator=(const TextWindowPool&) = delete;

    bool Initialize(size_t count);
    void Shutdown();

    void ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config);
    void ShowTextComputed(const ScreenPoint& anchorPoint, const TextEffectRenderCommand& command);

private:
    std::vector<std::unique_ptr<TextWindow>> windows_;
};

} // namespace mousefx
