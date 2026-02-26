#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.Internal.h"

#include <cctype>

namespace mousefx::platform::scaffold {
namespace {

std::string ToLowerAsciiCopy(std::string_view text) {
    std::string lowered;
    lowered.reserve(text.size());
    for (char c : text) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return lowered;
}

} // namespace

std::string ContentTypeForWebPath(std::string_view path) {
    const std::string lowered = ToLowerAsciiCopy(path);
    const auto endsWith = [&](const char* suffix) {
        const size_t suffixSize = std::char_traits<char>::length(suffix);
        return lowered.size() >= suffixSize &&
               lowered.compare(lowered.size() - suffixSize, suffixSize, suffix) == 0;
    };

    if (endsWith(".html")) return "text/html; charset=utf-8";
    if (endsWith(".js")) return "application/javascript; charset=utf-8";
    if (endsWith(".css")) return "text/css; charset=utf-8";
    if (endsWith(".json")) return "application/json; charset=utf-8";
    if (endsWith(".png")) return "image/png";
    if (endsWith(".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

} // namespace mousefx::platform::scaffold
