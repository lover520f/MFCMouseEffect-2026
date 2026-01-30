#pragma once

#include <string>

namespace mousefx {

// Resolves a directory for persistent config storage.
// In Release builds, prefers %AppData%\\MFCMouseEffect if available.
// Falls back to the EXE directory.
std::wstring ResolveConfigDirectory();

} // namespace mousefx

