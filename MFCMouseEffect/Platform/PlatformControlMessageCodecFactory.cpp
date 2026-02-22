#include "pch.h"

#include "Platform/PlatformControlMessageCodecFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Control/Win32DispatchMessageCodec.h"
#else
#include "MouseFx/Core/Control/NullDispatchMessageCodec.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IDispatchMessageCodec> CreateDispatchMessageCodec() {
#if defined(_WIN32)
    return std::make_unique<Win32DispatchMessageCodec>();
#else
    return std::make_unique<NullDispatchMessageCodec>();
#endif
}

} // namespace mousefx::platform
