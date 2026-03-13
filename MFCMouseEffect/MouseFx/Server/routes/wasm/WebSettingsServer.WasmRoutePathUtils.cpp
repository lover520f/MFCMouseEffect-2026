#include "pch.h"
#include "WebSettingsServer.WasmRouteUtils.h"

#include <cwctype>
#include <filesystem>
#include <string>

namespace mousefx::websettings_wasm_routes {
namespace {

std::wstring NormalizeManifestPathForCompare(const std::wstring& path) {
    std::wstring normalized = path;
    for (wchar_t& ch : normalized) {
        if (ch == L'/') {
            ch = L'\\';
        }
        ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return normalized;
}

} // namespace

bool IsSameManifestPath(const std::wstring& expected, const std::wstring& actual) {
    if (expected.empty() || actual.empty()) {
        return false;
    }

    const std::wstring expectedCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(expected).lexically_normal().wstring());
    const std::wstring actualCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(actual).lexically_normal().wstring());
    if (!expectedCanonical.empty() && !actualCanonical.empty()) {
        return expectedCanonical == actualCanonical;
    }

    return NormalizeManifestPathForCompare(expected) == NormalizeManifestPathForCompare(actual);
}

} // namespace mousefx::websettings_wasm_routes
