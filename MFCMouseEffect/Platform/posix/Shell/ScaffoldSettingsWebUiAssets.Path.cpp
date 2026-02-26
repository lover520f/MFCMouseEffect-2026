#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.Internal.h"

namespace mousefx::platform::scaffold {

bool IsSafeWebPath(std::string_view path) {
    if (path.empty()) {
        return false;
    }
    if (path.front() != '/') {
        return false;
    }
    if (path.find("..") != std::string_view::npos) {
        return false;
    }
    if (path.find('\\') != std::string_view::npos) {
        return false;
    }
    return true;
}

std::string NormalizeWebAssetRequestPath(std::string_view requestPath) {
    if (!IsSafeWebPath(requestPath)) {
        return {};
    }

    std::string path(requestPath);
    const size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }
    if (path == "/") {
        path = "/index.html";
    }
    if (!IsSafeWebPath(path)) {
        return {};
    }
    return path;
}

} // namespace mousefx::platform::scaffold
