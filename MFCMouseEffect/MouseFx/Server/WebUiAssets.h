#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mousefx {

struct WebUiAsset {
    std::string contentType;
    std::vector<uint8_t> bytes;
};

class WebUiAssets final {
public:
    explicit WebUiAssets(std::wstring baseDir);

    WebUiAssets(const WebUiAssets&) = delete;
    WebUiAssets& operator=(const WebUiAssets&) = delete;

    bool TryGet(const std::string& path, WebUiAsset& out) const;

private:
    bool TryGetFromDisk(const std::wstring& filePath, WebUiAsset& out) const;
    bool TryGetFromResource(int resourceId, WebUiAsset& out) const;

    static std::string ContentTypeForPath(const std::string& path);
    static std::wstring Utf8ToWide(const std::string& s);

    std::wstring baseDir_;
};

} // namespace mousefx

