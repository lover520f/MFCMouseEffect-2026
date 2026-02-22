#include "pch.h"

#include "Platform/PlatformBinaryResourceLoader.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32BinaryResourceLoader.h"
#endif

namespace mousefx::platform {

bool TryLoadEmbeddedBinaryResource(int resourceId, std::vector<uint8_t>* outBytes) {
    if (outBytes == nullptr) {
        return false;
    }

#if defined(_WIN32)
    return windows::Win32BinaryResourceLoader::TryLoadEmbeddedBinaryResource(resourceId, outBytes);
#else
    (void)resourceId;
    return false;
#endif
}

} // namespace mousefx::platform

