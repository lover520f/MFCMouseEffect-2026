#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteCodec.h"

namespace mousefx::platform::scaffold {

std::string NormalizePath(std::string path) {
    if (path.empty()) {
        return "/";
    }
    if (path.front() != '/') {
        path.insert(path.begin(), '/');
    }
    while (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }
    return path;
}

std::string PathWithoutQuery(const std::string& path) {
    const size_t queryPos = path.find('?');
    if (queryPos == std::string::npos) {
        return path;
    }
    return path.substr(0, queryPos);
}

std::string QueryValue(const std::string& path, const std::string& key) {
    const size_t queryPos = path.find('?');
    if (queryPos == std::string::npos) {
        return {};
    }

    const std::string query = path.substr(queryPos + 1);
    const std::string pattern = key + "=";
    size_t pos = 0;

    while (pos < query.size()) {
        const size_t ampPos = query.find('&', pos);
        const size_t end = (ampPos == std::string::npos) ? query.size() : ampPos;
        const std::string pair = query.substr(pos, end - pos);
        if (pair.rfind(pattern, 0) == 0) {
            return UrlDecodePercentCopy(pair.substr(pattern.size()));
        }
        if (ampPos == std::string::npos) {
            break;
        }
        pos = ampPos + 1;
    }
    return {};
}

std::string BuildTokenQuerySuffix(const std::string& token) {
    if (token.empty()) {
        return {};
    }
    return std::string("?token=") + UrlEncodeQueryValue(token);
}

bool IsHtmlPath(const std::string& pathOnly, const SettingsRoute& route) {
    const std::string normalized = NormalizePath(pathOnly);
    if (normalized == route.path) {
        return true;
    }
    if (normalized == "/" || normalized == "/index.html") {
        return true;
    }
    return false;
}

} // namespace mousefx::platform::scaffold
