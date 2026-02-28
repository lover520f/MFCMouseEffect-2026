#pragma once

#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

#include <string>

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_input_indicator {

int ClampInt(int value, int lo, int hi);
std::string MouseButtonLabel(MouseButton button);
std::string ScrollLabel(int delta);
std::string KeyLabel(const KeyEvent& ev);

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block);
void RunOnMainThreadAsync(dispatch_block_t block);
void FlushMainThreadQueueSync();
#endif

} // namespace mousefx::macos_input_indicator
