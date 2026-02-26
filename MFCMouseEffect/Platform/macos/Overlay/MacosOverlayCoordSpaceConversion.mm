#include "pch.h"

#include "Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#import <dispatch/dispatch.h>
#endif

#include <cmath>

namespace mousefx::macos_overlay_coord_conversion {

namespace {

ScreenPoint ToScreenPoint(double x, double y) {
    ScreenPoint out{};
    out.x = static_cast<int32_t>(std::lround(x));
    out.y = static_cast<int32_t>(std::lround(y));
    return out;
}

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    if ([NSThread isMainThread]) {
        block();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), block);
}
#endif

} // namespace

bool TryConvertQuartzToCocoa(const ScreenPoint& input, ScreenPoint* output) {
#if !defined(__APPLE__)
    (void)input;
    (void)output;
    return false;
#else
    if (!output) {
        return false;
    }

    __block bool converted = false;
    __block ScreenPoint convertedPoint = input;

    RunOnMainThreadSync(^{
      NSArray<NSScreen*>* screens = [NSScreen screens];
      if (screens == nil || [screens count] == 0) {
          return;
      }

      const CGPoint quartzPt = CGPointMake(static_cast<CGFloat>(input.x), static_cast<CGFloat>(input.y));

      NSScreen* matchedScreen = nil;
      CGRect matchedBounds = CGRectZero;
      for (NSScreen* screen in screens) {
          NSDictionary* desc = [screen deviceDescription];
          NSNumber* screenNumber = desc[@"NSScreenNumber"];
          if (screenNumber == nil) {
              continue;
          }
          const CGDirectDisplayID displayId = static_cast<CGDirectDisplayID>([screenNumber unsignedIntValue]);
          const CGRect bounds = CGDisplayBounds(displayId);
          if (CGRectContainsPoint(bounds, quartzPt)) {
              matchedScreen = screen;
              matchedBounds = bounds;
              break;
          }
      }

      if (matchedScreen == nil) {
          matchedScreen = [NSScreen mainScreen];
          if (matchedScreen == nil) {
              matchedScreen = [screens objectAtIndex:0];
          }
          NSDictionary* desc = [matchedScreen deviceDescription];
          NSNumber* screenNumber = desc[@"NSScreenNumber"];
          if (screenNumber != nil) {
              const CGDirectDisplayID displayId = static_cast<CGDirectDisplayID>([screenNumber unsignedIntValue]);
              matchedBounds = CGDisplayBounds(displayId);
          }
      }

      if (matchedScreen == nil) {
          return;
      }

      const NSRect frame = [matchedScreen frame];
      if (matchedBounds.size.width <= 0.0 || matchedBounds.size.height <= 0.0 ||
          frame.size.width <= 0.0 || frame.size.height <= 0.0) {
          convertedPoint = input;
          converted = true;
          return;
      }

      const double scaleX = static_cast<double>(frame.size.width) / static_cast<double>(matchedBounds.size.width);
      const double scaleY = static_cast<double>(frame.size.height) / static_cast<double>(matchedBounds.size.height);
      const double localX = (static_cast<double>(input.x) - static_cast<double>(matchedBounds.origin.x)) * scaleX;
      const double localY = (static_cast<double>(input.y) - static_cast<double>(matchedBounds.origin.y)) * scaleY;

      const double cocoaX = static_cast<double>(frame.origin.x) + localX;
      const double cocoaY = static_cast<double>(frame.origin.y) + static_cast<double>(frame.size.height) - localY;
      convertedPoint = ToScreenPoint(cocoaX, cocoaY);
      converted = true;
    });

    if (!converted) {
        return false;
    }
    *output = convertedPoint;
    return true;
#endif
}

} // namespace mousefx::macos_overlay_coord_conversion
