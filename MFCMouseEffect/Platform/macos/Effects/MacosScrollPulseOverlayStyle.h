#pragma once

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
NSColor* ScrollPulseStrokeColor(bool horizontal, int delta);
NSColor* ScrollPulseFillColor(bool horizontal, int delta);
CGPathRef CreateScrollPulseDirectionArrowPath(CGRect bodyRect, bool horizontal, int delta);
#endif

} // namespace mousefx::macos_scroll_pulse
