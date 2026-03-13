#include "pch.h"
#include "WebSettingsServer.WebUiPathResolver.h"

#include <cstdlib>
#include <filesystem>
#include <vector>

#include "Platform/PlatformRuntimeEnvironment.h"

namespace mousefx {
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

void AddEnvWebUiDir(std::vector<std::filesystem::path>* outDirs) {
    const char* envDir = std::getenv("MFX_WEBUI_DIR");
    if (envDir == nullptr || envDir[0] == '\0') {
        return;
    }
    AddWebUiDirIfExists(std::filesystem::path(envDir), outDirs);
}

void AddExecutableWebUiDir(std::vector<std::filesystem::path>* outDirs) {
    const std::wstring exeDir = platform::GetExecutableDirectoryW();
    if (exeDir.empty()) {
        return;
    }
    AddWebUiDirIfExists(std::filesystem::path(exeDir) / L"webui", outDirs);
}

void AddWorkingDirectoryWebUiDirs(std::vector<std::filesystem::path>* outDirs) {
    std::error_code ec;
    const std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (ec || cwd.empty()) {
        return;
    }

    AddWebUiDirIfExists(cwd / "MFCMouseEffect" / "WebUI", outDirs);
    AddWebUiDirIfExists(cwd / "WebUI", outDirs);
    AddWebUiDirIfExists(cwd.parent_path() / "MFCMouseEffect" / "WebUI", outDirs);
}

void AddSourceTreeWebUiDir(std::vector<std::filesystem::path>* outDirs) {
    const std::filesystem::path sourcePath(__FILE__);
    if (!sourcePath.is_absolute()) {
        return;
    }
    const std::filesystem::path projectDir =
        sourcePath.parent_path().parent_path().parent_path();
    AddWebUiDirIfExists(projectDir / "WebUI", outDirs);
}

} // namespace

std::wstring ResolveWebSettingsWebUiBaseDir() {
    std::vector<std::filesystem::path> candidates;
    candidates.reserve(8);
    AddEnvWebUiDir(&candidates);
    AddExecutableWebUiDir(&candidates);
    AddWorkingDirectoryWebUiDirs(&candidates);
    AddSourceTreeWebUiDir(&candidates);
    if (candidates.empty()) {
        return {};
    }
    return candidates.front().wstring();
}

} // namespace mousefx
