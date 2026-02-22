#pragma once

#include <cstdint>
#include <vector>

namespace mousefx::platform {

// Read an embedded binary blob by resource id.
bool TryLoadEmbeddedBinaryResource(int resourceId, std::vector<uint8_t>* outBytes);

} // namespace mousefx::platform

