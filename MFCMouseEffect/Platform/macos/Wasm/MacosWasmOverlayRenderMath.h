#pragma once

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

#include <cstdint>

namespace mousefx::platform::macos::wasm_overlay_render_math {

CGFloat ClampFloat(CGFloat value, CGFloat lo, CGFloat hi);
CGFloat ClampScale(float scale);
uint32_t ClampLifeMs(uint32_t lifeMs);

} // namespace mousefx::platform::macos::wasm_overlay_render_math
