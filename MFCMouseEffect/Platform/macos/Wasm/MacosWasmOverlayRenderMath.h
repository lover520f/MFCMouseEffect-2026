#pragma once

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

#include <cstdint>

namespace mousefx::platform::macos::wasm_overlay_render_math {

CGFloat ClampFloat(CGFloat value, CGFloat lo, CGFloat hi);
CGFloat ClampScale(float scale);
uint32_t ClampLifeMs(uint32_t lifeMs);
NSColor* ColorFromArgb(uint32_t argb, CGFloat alphaScale);

} // namespace mousefx::platform::macos::wasm_overlay_render_math
