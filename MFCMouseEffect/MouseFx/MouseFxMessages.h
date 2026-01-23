#pragma once

// Custom app messages used by the mouse effect subsystem.
// WM_APP range is per-process and safe for internal message passing.

#ifndef WM_APP
#define WM_APP 0x8000
#endif

namespace mousefx {

// lParam points to a heap-allocated ClickEvent (freed by the receiver).
constexpr unsigned int WM_MFX_CLICK = WM_APP + 0x210;

// lParam is packed x,y coordinate (LOWORD, HIWORD) or pointer to structure.
// Since mouse moves are frequent, allocating a struct per move is bad perf.
// Let's use standard LPARAM packing: x | (y << 16). 
// But coordinates are screen coords, can go negative? GetCursorPos returns LONG. 
// Standard WM_MOUSEMOVE uses client coords. WH_MOUSE_LL MSLLHOOKSTRUCT has POINT (LONG).
// Let's use a pointer to heap struct but only PostMessage if we throttle?
// Or better: Use global shared memory? 
// Actually, standard PostMessage with (x, y) packed might overflow if multiple monitors large coords?
// 16-bit signed is -32768 to 32767. That covers most 8K setups unless very wide.
// Let's use a custom struct but allocate it efficiently? 
// Or just pass x in wParam and y in lParam? 
// PostMessage(hwnd, msg, wParam, lParam). wParam is generic. 
// Let's use wParam = x, lParam = y. Cast to (WPARAM)x. Note x is LONG (32-bit). WPARAM is 64-bit on x64.
constexpr unsigned int WM_MFX_MOVE = WM_APP + 0x211;

} // namespace mousefx

