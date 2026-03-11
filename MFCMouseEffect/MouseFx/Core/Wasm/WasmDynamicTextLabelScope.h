#pragma once

#include <string>

namespace mousefx::wasm {

class ScopedWasmDynamicTextLabel final {
public:
    ScopedWasmDynamicTextLabel(std::wstring label, bool fromIndicatorEvent = false);
    ~ScopedWasmDynamicTextLabel();

    ScopedWasmDynamicTextLabel(const ScopedWasmDynamicTextLabel&) = delete;
    ScopedWasmDynamicTextLabel& operator=(const ScopedWasmDynamicTextLabel&) = delete;

private:
    std::wstring previous_{};
    bool hadPrevious_ = false;
    bool previousFromIndicatorEvent_ = false;
    bool hadPreviousFromIndicatorEvent_ = false;
};

std::wstring ResolveWasmDynamicTextLabel();
bool IsWasmDynamicTextLabelFromIndicatorEvent();

} // namespace mousefx::wasm
