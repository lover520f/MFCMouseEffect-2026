#pragma once

#include <cstdint>
#include <vector>

namespace mousefx::platform::windows {

class Win32BinaryResourceLoader final {
public:
    static bool TryLoadEmbeddedBinaryResource(int resourceId, std::vector<uint8_t>* outBytes);
};

} // namespace mousefx::platform::windows

