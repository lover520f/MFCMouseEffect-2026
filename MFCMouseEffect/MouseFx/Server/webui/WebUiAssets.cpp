#include "pch.h"
#include "WebUiAssets.h"

#include <cstring>
#include <filesystem>
#include <fstream>

#include "Resource.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformBinaryResourceLoader.h"

namespace mousefx {

WebUiAssets::WebUiAssets(std::wstring baseDir) : baseDir_(std::move(baseDir)) {}

static bool IsSafeWebPath(const std::string& path) {
    if (path.empty()) return false;
    if (path[0] != '/') return false;
    if (path.find("..") != std::string::npos) return false;
    if (path.find('\\') != std::string::npos) return false;
    return true;
}

std::string WebUiAssets::ContentTypeForPath(const std::string& path) {
    const std::string lowered = ToLowerAscii(path);
    auto ends = [&](const char* suf) {
        size_t n = std::strlen(suf);
        return lowered.size() >= n && lowered.compare(lowered.size() - n, n, suf) == 0;
    };
    if (ends(".html")) return "text/html; charset=utf-8";
    if (ends(".js")) return "application/javascript; charset=utf-8";
    if (ends(".css")) return "text/css; charset=utf-8";
    if (ends(".json")) return "application/json; charset=utf-8";
    if (ends(".png")) return "image/png";
    if (ends(".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

bool WebUiAssets::TryGet(const std::string& path, WebUiAsset& out) const {
    if (!IsSafeWebPath(path)) return false;

    std::string p = path;
    size_t q = p.find('?');
    if (q != std::string::npos) p = p.substr(0, q);
    if (p == "/") p = "/index.html";

    out.contentType = ContentTypeForPath(p);

    // Disk override: $(OutDir)\webui\*
    if (!baseDir_.empty()) {
        std::wstring rel = Utf8ToWString(p.substr(1)); // strip leading '/'
        if (!rel.empty()) {
            const std::filesystem::path disk = std::filesystem::path(baseDir_) / std::filesystem::path(rel);
            if (TryGetFromDisk(disk, out)) return true;
        }
    }

    // Embedded fallback (RCDATA) for core files.
    if (p == "/index.html") return TryGetFromResource(IDR_WEBUI_INDEX, out);
    if (p == "/app.js") return TryGetFromResource(IDR_WEBUI_APPJS, out);
    if (p == "/styles.css") return TryGetFromResource(IDR_WEBUI_STYLES, out);

    return false;
}

bool WebUiAssets::TryGetFromDisk(const std::filesystem::path& filePath, WebUiAsset& out) const {
    std::ifstream f(filePath, std::ios::binary);
    if (!f.is_open()) return false;
    f.seekg(0, std::ios::end);
    std::streamoff n = f.tellg();
    if (n <= 0 || n > (std::streamoff)(4 * 1024 * 1024)) return false;
    f.seekg(0, std::ios::beg);
    out.bytes.resize((size_t)n);
    f.read((char*)out.bytes.data(), n);
    return f.good();
}

bool WebUiAssets::TryGetFromResource(int resourceId, WebUiAsset& out) const {
    std::vector<uint8_t> bytes;
    if (!platform::TryLoadEmbeddedBinaryResource(resourceId, &bytes)) {
        return false;
    }
    out.bytes = std::move(bytes);
    return true;
}

} // namespace mousefx
