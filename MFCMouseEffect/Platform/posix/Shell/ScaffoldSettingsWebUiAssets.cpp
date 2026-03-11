#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.h"
#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.Internal.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <utility>

namespace mousefx::platform::scaffold {
namespace {

void AddWebUiDirIfExists(
    const std::filesystem::path& dir,
    std::vector<std::filesystem::path>* outDirs) {
    if (!outDirs || dir.empty()) {
        return;
    }

    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || ec) {
        return;
    }
    if (!std::filesystem::is_directory(dir, ec) || ec) {
        return;
    }
    outDirs->push_back(dir);
}

bool TryReadWebUiFile(const std::filesystem::path& filePath, std::vector<uint8_t>* outBytes) {
    if (!outBytes) {
        return false;
    }

    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size <= 0 || size > static_cast<std::streamoff>(4 * 1024 * 1024)) {
        return false;
    }

    input.seekg(0, std::ios::beg);
    outBytes->resize(static_cast<size_t>(size));
    input.read(reinterpret_cast<char*>(outBytes->data()), size);
    return input.good();
}

void AddSourceTreeWebUiDir(std::vector<std::filesystem::path>* outDirs) {
    const std::filesystem::path sourcePath(__FILE__);
    if (!sourcePath.is_absolute()) {
        return;
    }

    const std::filesystem::path projectDir =
        sourcePath.parent_path().parent_path().parent_path().parent_path();
    AddWebUiDirIfExists(projectDir / "WebUI", outDirs);
}

} // namespace

std::vector<std::filesystem::path> BuildWebUiBaseDirs() {
    std::vector<std::filesystem::path> dirs;

    const char* overrideDir = std::getenv("MFX_SCAFFOLD_WEBUI_DIR");
    if (overrideDir && *overrideDir != '\0') {
        AddWebUiDirIfExists(std::filesystem::path(overrideDir), &dirs);
        return dirs;
    }

    // Keep dev runtime stable: prefer source-tree WebUI over accidental stale cwd copies.
    AddSourceTreeWebUiDir(&dirs);

    std::error_code ec;
    const std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (!ec && !cwd.empty()) {
        AddWebUiDirIfExists(cwd / "MFCMouseEffect" / "WebUI", &dirs);
        AddWebUiDirIfExists(cwd / "WebUI", &dirs);
        AddWebUiDirIfExists(cwd.parent_path() / "MFCMouseEffect" / "WebUI", &dirs);
    }
    return dirs;
}

bool TryLoadWebUiAsset(
    const std::vector<std::filesystem::path>& baseDirs,
    const std::string& requestPath,
    WebUiAsset* outAsset) {
    if (!outAsset || baseDirs.empty()) {
        return false;
    }

    const std::string path = NormalizeWebAssetRequestPath(requestPath);
    if (path.empty()) {
        return false;
    }

    for (const auto& baseDir : baseDirs) {
        const std::filesystem::path diskPath = baseDir / path.substr(1);
        WebUiAsset candidate;
        if (!TryReadWebUiFile(diskPath, &candidate.bytes)) {
            continue;
        }
        candidate.contentType = ContentTypeForWebPath(path);
        *outAsset = std::move(candidate);
        return true;
    }
    return false;
}

std::string BuildMissingWebUiMessage() {
    return "Scaffold WebUI assets not found. Build WebUIWorkspace to populate WebUI/*.svelte.js.";
}

} // namespace mousefx::platform::scaffold
