#include "pch.h"

#include "WasmDynamicTextLabelScope.h"

#include <utility>

namespace mousefx::wasm {

namespace {

thread_local std::wstring gDynamicTextLabel{};
thread_local bool gHasDynamicTextLabel = false;
thread_local bool gDynamicTextLabelFromIndicatorEvent = false;

} // namespace

ScopedWasmDynamicTextLabel::ScopedWasmDynamicTextLabel(std::wstring label, bool fromIndicatorEvent) {
    if (gHasDynamicTextLabel) {
        previous_ = gDynamicTextLabel;
        hadPrevious_ = true;
        previousFromIndicatorEvent_ = gDynamicTextLabelFromIndicatorEvent;
        hadPreviousFromIndicatorEvent_ = true;
    }
    gDynamicTextLabel = std::move(label);
    gHasDynamicTextLabel = !gDynamicTextLabel.empty();
    gDynamicTextLabelFromIndicatorEvent = gHasDynamicTextLabel && fromIndicatorEvent;
}

ScopedWasmDynamicTextLabel::~ScopedWasmDynamicTextLabel() {
    if (hadPrevious_) {
        gDynamicTextLabel = std::move(previous_);
        gHasDynamicTextLabel = !gDynamicTextLabel.empty();
        if (hadPreviousFromIndicatorEvent_) {
            gDynamicTextLabelFromIndicatorEvent = previousFromIndicatorEvent_;
        } else {
            gDynamicTextLabelFromIndicatorEvent = false;
        }
        return;
    }
    gDynamicTextLabel.clear();
    gHasDynamicTextLabel = false;
    gDynamicTextLabelFromIndicatorEvent = false;
}

std::wstring ResolveWasmDynamicTextLabel() {
    if (!gHasDynamicTextLabel) {
        return {};
    }
    return gDynamicTextLabel;
}

bool IsWasmDynamicTextLabelFromIndicatorEvent() {
    return gHasDynamicTextLabel && gDynamicTextLabelFromIndicatorEvent;
}

} // namespace mousefx::wasm
