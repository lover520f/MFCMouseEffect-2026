#pragma once

#include <windows.h>

namespace mousefx {

class AppController;

// Routes Win32 messages from the dispatch HWND to the appropriate
// AppController subsystems (effects, indicator, timers).
// Extracted from AppController::OnDispatchMessage.
class DispatchRouter final {
public:
    explicit DispatchRouter(AppController* controller);

    // Main message routing entry point.  Returns the LRESULT for the message.
    LRESULT Route(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT OnClick(HWND hwnd, LPARAM lParam);
    LRESULT OnMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT OnScroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT OnKey(HWND hwnd, LPARAM lParam);
    LRESULT OnButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT OnButtonUp(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(HWND hwnd, WPARAM wParam);

    AppController* ctrl_ = nullptr;
};

} // namespace mousefx
