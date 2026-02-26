#include "Platform/posix/Shell/PosixKeyValueCaptureFile.h"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace mousefx {
namespace {

bool EnsureParentDirectory(const std::filesystem::path& filePath) {
    const std::filesystem::path parentPath = filePath.parent_path();
    if (parentPath.empty()) {
        return true;
    }
    std::error_code ec;
    std::filesystem::create_directories(parentPath, ec);
    return !ec;
}

} // namespace

bool WritePosixKeyValueCaptureFile(
    const std::string& filePath,
    const std::vector<std::pair<std::string, std::string>>& lines) {
    if (filePath.empty() || lines.empty()) {
        return false;
    }

    const std::filesystem::path targetPath(filePath);
    if (!EnsureParentDirectory(targetPath)) {
        return false;
    }

    const std::string tmpPath = filePath + ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }

        for (const auto& [key, value] : lines) {
            out << key << "=" << value << '\n';
        }
        out.flush();
        if (!out.good()) {
            return false;
        }
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, targetPath, ec);
    if (!ec) {
        return true;
    }

    std::filesystem::remove(targetPath, ec);
    ec.clear();
    std::filesystem::rename(tmpPath, targetPath, ec);
    return !ec;
}

} // namespace mousefx
