#pragma once

#include <d2d1.h>
#include <dwrite.h>

namespace mousefx {

// Process-level shared D2D1 and DWrite factories.
// Uses C++11 magic statics for lazy, thread-safe initialization.
// The factories are created as single-threaded (D2D1_FACTORY_TYPE_SINGLE_THREADED
// / DWRITE_FACTORY_TYPE_SHARED) matching the existing usage pattern where all
// rendering happens on the main thread.
//
// Thread-safety note: the factories themselves are obtained thread-safely,
// but D2D1_FACTORY_TYPE_SINGLE_THREADED means the *in-flight rendering calls*
// must still happen from a single thread at a time. Since all TextWindow
// instances pump on the UI thread via WM_TIMER, this is safe.

ID2D1Factory* SharedD2D1Factory();
IDWriteFactory* SharedDWriteFactory();

} // namespace mousefx
