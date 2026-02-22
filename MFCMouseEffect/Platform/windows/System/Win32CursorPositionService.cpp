#include "pch.h"

#include "Platform/windows/System/Win32CursorPositionService.h"

#include <windows.h>

namespace mousefx {

bool Win32CursorPositionService::TryGetCursorScreenPoint(ScreenPoint* outPt) const {
    if (!outPt) {
        return false;
    }

    POINT nativePt{};
    if (!GetCursorPos(&nativePt)) {
        return false;
    }

    outPt->x = nativePt.x;
    outPt->y = nativePt.y;
    return true;
}

} // namespace mousefx
