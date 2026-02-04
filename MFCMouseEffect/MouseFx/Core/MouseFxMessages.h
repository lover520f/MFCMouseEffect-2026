#pragma once

// Custom app messages used by the mouse effect subsystem.
// WM_APP range is per-process and safe for internal message passing.

#ifndef WM_APP
#define WM_APP 0x8000
#endif

namespace mousefx {

// lParam points to a heap-allocated ClickEvent (freed by the receiver).
constexpr unsigned int WM_MFX_CLICK = WM_APP + 0x210;

// Mouse move: wParam = x, lParam = y
constexpr unsigned int WM_MFX_MOVE = WM_APP + 0x211;

// Mouse scroll: wParam = delta (positive = up), lParam = MAKELPARAM(x, y)
constexpr unsigned int WM_MFX_SCROLL = WM_APP + 0x212;

// Button down (for Hold detection): wParam = button (1=L, 2=R, 3=M), lParam = MAKELPARAM(x, y)
constexpr unsigned int WM_MFX_BUTTON_DOWN = WM_APP + 0x213;

// Button up (for Hold end): wParam = button
constexpr unsigned int WM_MFX_BUTTON_UP = WM_APP + 0x214;

// Execute command from IPC: lParam = std::string* (pointer to stack string from sender thread)
constexpr unsigned int WM_MFX_EXEC_CMD = WM_APP + 0x215;

// Fetch a config snapshot: lParam = EffectConfig* (filled by dispatch thread)
constexpr unsigned int WM_MFX_GET_CONFIG = WM_APP + 0x216;

} // namespace mousefx
