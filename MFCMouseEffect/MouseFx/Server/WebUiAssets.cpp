#include "pch.h"
#include "WebUiAssets.h"

#include <windows.h>

#include <fstream>

#include "Resource.h"

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
    auto ends = [&](const char* suf) {
        size_t n = strlen(suf);
        return path.size() >= n && _stricmp(path.c_str() + (path.size() - n), suf) == 0;
    };
    if (ends(".html")) return "text/html; charset=utf-8";
    if (ends(".js")) return "application/javascript; charset=utf-8";
    if (ends(".css")) return "text/css; charset=utf-8";
    if (ends(".json")) return "application/json; charset=utf-8";
    if (ends(".png")) return "image/png";
    if (ends(".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

std::wstring WebUiAssets::Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring out((size_t)len, L'\0');
    int written = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.empty() ? nullptr : &out[0], len);
    if (written <= 0) return {};
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
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
        std::wstring rel = Utf8ToWide(p.substr(1)); // strip leading '/'
        if (!rel.empty()) {
            std::wstring disk = baseDir_ + L"\\" + rel;
            if (TryGetFromDisk(disk, out)) return true;
        }
    }

    // Embedded fallback (RCDATA) for core files.
    if (p == "/index.html") return TryGetFromResource(IDR_WEBUI_INDEX, out);
    if (p == "/app.js") return TryGetFromResource(IDR_WEBUI_APPJS, out);
    if (p == "/styles.css") return TryGetFromResource(IDR_WEBUI_STYLES, out);

    return false;
}

bool WebUiAssets::TryGetFromDisk(const std::wstring& filePath, WebUiAsset& out) const {
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
    HINSTANCE h = GetModuleHandleW(nullptr);
    HRSRC r = FindResourceW(h, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!r) return false;
    HGLOBAL hg = LoadResource(h, r);
    if (!hg) return false;
    DWORD sz = SizeofResource(h, r);
    if (sz == 0) return false;
    void* p = LockResource(hg);
    if (!p) return false;
    out.bytes.assign((const uint8_t*)p, (const uint8_t*)p + sz);
    return true;
}

} // namespace mousefx
