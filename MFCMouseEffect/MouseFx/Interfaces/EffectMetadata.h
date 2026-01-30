#pragma once

#include <windows.h>
#include <string>

namespace mousefx {

// Master metadata for an effect option to ensure consistency across UI components.
struct EffectOption {
    const char* value;       // Internal type name (e.g. "ripple")
    UINT trayCmd;            // Corresponding Tray menu command ID
    const wchar_t* displayZh; // Chinese display name
    const wchar_t* displayEn; // English display name
    const char* secondType = nullptr; // Optional alias for matching

    // Helper for Settings UI
    const wchar_t* GetDisplay(bool zh) const {
        return zh ? displayZh : displayEn;
    }

    bool IsMatch(const std::string& current) const {
        if (current == value) return true;
        if (secondType && current == secondType) return true;
        return false;
    }
};

} // namespace mousefx
