#include "pch.h"

#include "Platform/windows/Effects/Win32TextEffectFallback.h"

namespace mousefx {

bool Win32TextEffectFallback::EnsureInitialized(size_t count) {
    return pool_.Initialize(count);
}

void Win32TextEffectFallback::Shutdown() {
    pool_.Shutdown();
}

void Win32TextEffectFallback::ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    pool_.ShowText(pt, text, color, config);
}

} // namespace mousefx
