#include "pch.h"

#include "Platform/PlatformInputServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32CursorPositionService.h"
#include "Platform/windows/System/Win32GlobalMouseHook.h"
#include "Platform/windows/System/Win32KeyboardInjector.h"
#else
#include "MouseFx/Core/System/NullCursorPositionService.h"
#include "MouseFx/Core/System/NullGlobalMouseHook.h"
#include "MouseFx/Core/System/NullKeyboardInjector.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IGlobalMouseHook> CreateGlobalMouseHook() {
#if defined(_WIN32)
    return std::make_unique<Win32GlobalMouseHook>();
#else
    return std::make_unique<NullGlobalMouseHook>();
#endif
}

std::unique_ptr<ICursorPositionService> CreateCursorPositionService() {
#if defined(_WIN32)
    return std::make_unique<Win32CursorPositionService>();
#else
    return std::make_unique<NullCursorPositionService>();
#endif
}

std::unique_ptr<IKeyboardInjector> CreateKeyboardInjector() {
#if defined(_WIN32)
    return std::make_unique<Win32KeyboardInjector>();
#else
    return std::make_unique<NullKeyboardInjector>();
#endif
}

} // namespace mousefx::platform
