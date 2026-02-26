#pragma once

#include <string>
#include <utility>
#include <vector>

namespace mousefx {

bool WritePosixKeyValueCaptureFile(
    const std::string& filePath,
    const std::vector<std::pair<std::string, std::string>>& lines);

} // namespace mousefx
