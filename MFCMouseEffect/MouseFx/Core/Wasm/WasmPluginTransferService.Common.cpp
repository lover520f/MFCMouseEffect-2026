#include "pch.h"

#include "WasmPluginTransferService.Internal.h"

#include "MouseFx/Utils/StringUtils.h"

#include <array>
#include <chrono>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <system_error>

namespace mousefx::wasm::transfer_detail {
namespace {

std::wstring SanitizeDirectoryName(const std::wstring& input) {
    if (input.empty()) {
        return {};
    }

    std::wstring out;
    out.reserve(input.size());
    for (const wchar_t ch : input) {
        if (ch == L'<' ||
            ch == L'>' ||
            ch == L':' ||
            ch == L'"' ||
            ch == L'/' ||
            ch == L'\\' ||
            ch == L'|' ||
            ch == L'?' ||
            ch == L'*' ||
            ch < 32) {
            out.push_back(L'_');
            continue;
        }
        out.push_back(ch);
    }

    while (!out.empty() && (out.back() == L' ' || out.back() == L'.')) {
        out.pop_back();
    }
    return out;
}

} // namespace

std::filesystem::path NormalizePath(const std::filesystem::path& path) {
    std::error_code ec;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
    if (!ec) {
        return canonical.lexically_normal();
    }
    return path.lexically_normal();
}

std::wstring ToPathKey(const std::filesystem::path& path) {
    std::wstring key = NormalizePath(path).wstring();
    for (wchar_t& ch : key) {
        ch = static_cast<wchar_t>(::towlower(ch));
    }
    return key;
}

std::wstring ResolvePluginDirectoryName(const PluginManifest& manifest, const std::filesystem::path& sourceDir) {
    std::wstring name = SanitizeDirectoryName(Utf8ToWString(manifest.id));
    if (!name.empty()) {
        return name;
    }
    name = SanitizeDirectoryName(sourceDir.filename().wstring());
    if (!name.empty()) {
        return name;
    }
    return L"wasm_plugin";
}

bool ValidateManifestEntryFile(
    const PluginManifest& manifest,
    const std::filesystem::path& pluginDir,
    std::string* error) {
    if (pluginDir.empty()) {
        if (error) {
            *error = "plugin directory is empty";
        }
        return false;
    }

    const std::filesystem::path entryRelativePath(manifest.entryWasm);
    const std::filesystem::path entryPath = pluginDir / entryRelativePath;
    std::error_code ec;
    if (!std::filesystem::exists(entryPath, ec) || ec) {
        if (error) {
            *error = "manifest entry wasm file does not exist";
        }
        return false;
    }
    if (!std::filesystem::is_regular_file(entryPath, ec) || ec) {
        if (error) {
            *error = "manifest entry wasm path is not a file";
        }
        return false;
    }
    return true;
}

bool CopyDirectoryTree(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    std::string* error) {
    if (from.empty() || to.empty()) {
        if (error) {
            *error = "source or destination path is empty";
        }
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::exists(from, ec) || ec) {
        if (error) {
            *error = "source directory does not exist";
        }
        return false;
    }
    if (!std::filesystem::is_directory(from, ec) || ec) {
        if (error) {
            *error = "source path is not a directory";
        }
        return false;
    }

    std::filesystem::create_directories(to.parent_path(), ec);
    if (ec) {
        if (error) {
            *error = "failed to create destination parent directory";
        }
        return false;
    }

    std::filesystem::remove_all(to, ec);
    if (ec) {
        if (error) {
            *error = "failed to clear destination directory";
        }
        return false;
    }

    std::filesystem::create_directories(to, ec);
    if (ec) {
        if (error) {
            *error = "failed to create destination directory";
        }
        return false;
    }

    std::filesystem::copy(
        from,
        to,
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing,
        ec);
    if (ec) {
        if (error) {
            *error = "failed to copy plugin directory";
        }
        return false;
    }
    return true;
}

std::wstring BuildTimestampLabel() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeValue = std::chrono::system_clock::to_time_t(now);

    std::tm localTm{};
#if defined(_WIN32)
    if (::localtime_s(&localTm, &timeValue) != 0) {
        return L"unknown-time";
    }
#else
    if (::localtime_r(&timeValue, &localTm) == nullptr) {
        return L"unknown-time";
    }
#endif

    std::array<wchar_t, 48> buffer{};
    const int n = swprintf(
        buffer.data(),
        buffer.size(),
        L"%04u%02u%02u-%02u%02u%02u",
        static_cast<unsigned>(localTm.tm_year + 1900),
        static_cast<unsigned>(localTm.tm_mon + 1),
        static_cast<unsigned>(localTm.tm_mday),
        static_cast<unsigned>(localTm.tm_hour),
        static_cast<unsigned>(localTm.tm_min),
        static_cast<unsigned>(localTm.tm_sec));
    if (n <= 0) {
        return L"unknown-time";
    }
    return std::wstring(buffer.data(), static_cast<size_t>(n));
}

} // namespace mousefx::wasm::transfer_detail
