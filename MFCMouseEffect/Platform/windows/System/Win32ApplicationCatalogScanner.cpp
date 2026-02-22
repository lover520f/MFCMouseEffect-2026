#include "pch.h"
#include "Platform/windows/System/Win32ApplicationCatalogScanner.h"

#include <KnownFolders.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <objbase.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::platform::windows {
namespace {

struct ScanRoot {
    std::filesystem::path path;
    bool recursive = false;
    std::string source;
};

std::wstring TrimWide(std::wstring value) {
    const auto isSpace = [](wchar_t ch) -> bool {
        return ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n';
    };
    size_t start = 0;
    while (start < value.size() && isSpace(value[start])) {
        ++start;
    }
    if (start >= value.size()) {
        return {};
    }

    size_t end = value.size();
    while (end > start && isSpace(value[end - 1])) {
        --end;
    }
    return value.substr(start, end - start);
}

std::wstring StripWrappingQuotes(std::wstring value) {
    value = TrimWide(std::move(value));
    if (value.size() < 2) {
        return value;
    }
    const wchar_t first = value.front();
    const wchar_t last = value.back();
    if ((first == L'"' && last == L'"') || (first == L'\'' && last == L'\'')) {
        value = value.substr(1, value.size() - 2);
    }
    return TrimWide(std::move(value));
}

std::wstring ToLowerWide(std::wstring text) {
    std::transform(text.begin(), text.end(), text.begin(), [](wchar_t ch) -> wchar_t {
        return static_cast<wchar_t>(::towlower(ch));
    });
    return text;
}

bool EndsWithCaseInsensitive(const std::wstring& text, const std::wstring& suffix) {
    if (suffix.empty() || text.size() < suffix.size()) {
        return false;
    }
    const std::wstring textTail = ToLowerWide(text.substr(text.size() - suffix.size()));
    const std::wstring loweredSuffix = ToLowerWide(suffix);
    return textTail == loweredSuffix;
}

std::wstring StripKnownShortcutSuffixes(std::wstring label) {
    label = TrimWide(std::move(label));
    static constexpr std::array<const wchar_t*, 3> kSuffixes = {
        L" - 快捷方式",
        L" - shortcut",
        L".url",
    };
    bool changed = true;
    while (changed) {
        changed = false;
        for (const wchar_t* suffix : kSuffixes) {
            const std::wstring suffixText(suffix ? suffix : L"");
            if (suffixText.empty()) {
                continue;
            }
            if (!EndsWithCaseInsensitive(label, suffixText)) {
                continue;
            }
            label = TrimWide(label.substr(0, label.size() - suffixText.size()));
            changed = true;
            break;
        }
    }
    return label;
}

bool IsAsciiAlphaNum(char ch) {
    return (ch >= '0' && ch <= '9') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z');
}

bool ContainsAsciiWordBoundary(const std::string& text, const std::string& keyword) {
    if (text.empty() || keyword.empty()) {
        return false;
    }

    size_t start = 0;
    while (true) {
        const size_t pos = text.find(keyword, start);
        if (pos == std::string::npos) {
            return false;
        }
        const size_t before = (pos == 0) ? pos : pos - 1;
        const size_t after = pos + keyword.size();
        const bool beforeOk = (pos == 0) || !IsAsciiAlphaNum(text[before]);
        const bool afterOk = (after >= text.size()) || !IsAsciiAlphaNum(text[after]);
        if (beforeOk && afterOk) {
            return true;
        }
        start = pos + 1;
    }
}

bool IsJunkName(const std::wstring& sourceName) {
    std::wstring lowered = ToLowerWide(sourceName);
    lowered = TrimWide(std::move(lowered));
    if (lowered.empty()) {
        return true;
    }

    const std::string utf8 = ToLowerAscii(TrimAscii(Utf16ToUtf8(lowered.c_str())));
    static constexpr std::array<const char*, 10> kAsciiKeywords = {
        "uninstall",
        "uninst",
        "install",
        "setup",
        "config",
        "update",
        "readme",
        "help",
        "visit",
        "website",
    };
    for (const char* keyword : kAsciiKeywords) {
        if (ContainsAsciiWordBoundary(utf8, keyword)) {
            return true;
        }
    }

    static constexpr std::array<const wchar_t*, 6> kCjkKeywords = {
        L"卸载",
        L"安装",
        L"设置",
        L"帮助",
        L"说明",
        L"关于",
    };
    for (const wchar_t* keyword : kCjkKeywords) {
        if (lowered.find(keyword) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

std::filesystem::path ReadKnownFolderPath(REFKNOWNFOLDERID folderId) {
    PWSTR rawPath = nullptr;
    const HRESULT hr = SHGetKnownFolderPath(folderId, 0, nullptr, &rawPath);
    if (FAILED(hr) || !rawPath) {
        if (rawPath) {
            CoTaskMemFree(rawPath);
        }
        return {};
    }
    std::filesystem::path out(rawPath);
    CoTaskMemFree(rawPath);
    return out;
}

void AddKnownFolderRoot(
    REFKNOWNFOLDERID folderId,
    bool recursive,
    const char* source,
    std::vector<ScanRoot>* roots,
    std::unordered_set<std::wstring>* dedupRoots) {
    if (!roots || !dedupRoots) {
        return;
    }
    const std::filesystem::path path = ReadKnownFolderPath(folderId);
    if (path.empty()) {
        return;
    }
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return;
    }
    const std::wstring lowered = ToLowerWide(path.wstring());
    if (!dedupRoots->insert(lowered).second) {
        return;
    }
    roots->push_back(ScanRoot{path, recursive, source ? source : ""});
}

std::string NormalizeProcessNameFromText(std::wstring text) {
    text = StripWrappingQuotes(std::move(text));
    if (text.empty()) {
        return {};
    }

    std::replace(text.begin(), text.end(), L'\\', L'/');
    const size_t slashPos = text.find_last_of(L'/');
    if (slashPos != std::wstring::npos) {
        text = text.substr(slashPos + 1);
    }
    text = TrimWide(std::move(text));
    if (text.empty()) {
        return {};
    }

    const std::filesystem::path normalizedPath(text);
    std::wstring ext = ToLowerWide(normalizedPath.extension().wstring());
    if (ext.empty()) {
        text += L".exe";
    } else if (ext != L".exe") {
        return {};
    }
    return ToLowerAscii(TrimAscii(Utf16ToUtf8(text.c_str())));
}

std::string BuildDisplayName(const std::filesystem::path& filePath, const std::string& processName) {
    std::wstring rawLabel = StripKnownShortcutSuffixes(filePath.stem().wstring());
    if (rawLabel.empty() || IsJunkName(rawLabel)) {
        return processName;
    }
    std::string label = TrimAscii(Utf16ToUtf8(rawLabel.c_str()));
    if (label.empty() || IsJunkName(Utf8ToWString(label))) {
        label = processName;
    }
    return label;
}

class ScopedComApartment final {
public:
    ScopedComApartment() {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        initialized_ = (hr == S_OK || hr == S_FALSE);
    }

    ~ScopedComApartment() {
        if (initialized_) {
            CoUninitialize();
        }
    }

private:
    bool initialized_ = false;
};

std::string ResolveShortcutTargetProcessName(const std::filesystem::path& shortcutPath) {
    IShellLinkW* shellLink = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellLink,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&shellLink));
    if (FAILED(hr) || !shellLink) {
        return {};
    }

    IPersistFile* persistFile = nullptr;
    hr = shellLink->QueryInterface(IID_PPV_ARGS(&persistFile));
    if (FAILED(hr) || !persistFile) {
        shellLink->Release();
        return {};
    }

    hr = persistFile->Load(shortcutPath.c_str(), STGM_READ);
    if (FAILED(hr)) {
        persistFile->Release();
        shellLink->Release();
        return {};
    }

    shellLink->Resolve(nullptr, SLR_NO_UI);

    std::array<wchar_t, 4096> pathBuffer{};
    WIN32_FIND_DATAW findData{};
    hr = shellLink->GetPath(
        pathBuffer.data(),
        static_cast<int>(pathBuffer.size()),
        &findData,
        SLGP_RAWPATH);

    persistFile->Release();
    shellLink->Release();

    if (FAILED(hr) || pathBuffer[0] == L'\0') {
        return {};
    }

    const std::filesystem::path targetPath(pathBuffer.data());
    std::error_code ec;
    if (std::filesystem::is_directory(targetPath, ec) && !ec) {
        return {};
    }
    if (ToLowerWide(targetPath.extension().wstring()) != L".exe") {
        return {};
    }
    return NormalizeProcessNameFromText(targetPath.filename().wstring());
}

std::string ResolveProcessNameForEntry(const std::filesystem::path& filePath) {
    std::wstring ext = ToLowerWide(filePath.extension().wstring());
    if (ext == L".exe") {
        return NormalizeProcessNameFromText(filePath.filename().wstring());
    }
    if (ext == L".lnk") {
        const std::string resolved = ResolveShortcutTargetProcessName(filePath);
        return resolved;
    }
    return {};
}

bool IsAllowedApplicationEntry(const std::filesystem::path& filePath) {
    const std::wstring ext = ToLowerWide(filePath.extension().wstring());
    return ext == L".exe" || ext == L".lnk";
}

void UpsertEntry(
    const std::filesystem::path& filePath,
    const std::string& processName,
    const std::string& source,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess) {
    if (!entries || !indexByProcess || processName.empty()) {
        return;
    }

    auto found = indexByProcess->find(processName);
    if (found != indexByProcess->end()) {
        ApplicationCatalogEntry& existing = entries->at(found->second);
        if (existing.displayName == existing.processName) {
            existing.displayName = BuildDisplayName(filePath, processName);
        }
        return;
    }

    ApplicationCatalogEntry entry;
    entry.processName = processName;
    entry.displayName = BuildDisplayName(filePath, processName);
    entry.source = source;
    (*indexByProcess)[entry.processName] = entries->size();
    entries->push_back(std::move(entry));
}

void ScanSingleRoot(
    const ScanRoot& root,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess) {
    std::error_code ec;
    if (!std::filesystem::exists(root.path, ec) || ec) {
        return;
    }

    const auto handleEntry = [&](const std::filesystem::directory_entry& item) {
        std::error_code localEc;
        if (!item.is_regular_file(localEc) || localEc) {
            return;
        }
        const std::filesystem::path filePath = item.path();
        if (!IsAllowedApplicationEntry(filePath)) {
            return;
        }

        const std::wstring baseName = filePath.stem().wstring();
        if (IsJunkName(baseName)) {
            return;
        }

        const std::string processName = ResolveProcessNameForEntry(filePath);
        if (processName.empty()) {
            return;
        }
        if (IsJunkName(Utf8ToWString(processName))) {
            return;
        }

        UpsertEntry(filePath, processName, root.source, entries, indexByProcess);
    };

    if (root.recursive) {
        std::filesystem::recursive_directory_iterator it(
            root.path,
            std::filesystem::directory_options::skip_permission_denied,
            ec);
        const std::filesystem::recursive_directory_iterator end;
        while (!ec && it != end) {
            handleEntry(*it);
            it.increment(ec);
            if (ec) {
                ec.clear();
            }
        }
        return;
    }

    std::filesystem::directory_iterator it(
        root.path,
        std::filesystem::directory_options::skip_permission_denied,
        ec);
    const std::filesystem::directory_iterator end;
    while (!ec && it != end) {
        handleEntry(*it);
        it.increment(ec);
        if (ec) {
            ec.clear();
        }
    }
}

} // namespace

std::vector<ApplicationCatalogEntry> Win32ApplicationCatalogScanner::Scan() const {
    ScopedComApartment scopedCom;

    std::vector<ScanRoot> roots;
    std::unordered_set<std::wstring> dedupRoots;

    AddKnownFolderRoot(FOLDERID_Programs, true, "start_menu", &roots, &dedupRoots);
    AddKnownFolderRoot(FOLDERID_CommonPrograms, true, "start_menu", &roots, &dedupRoots);
    AddKnownFolderRoot(FOLDERID_Desktop, false, "desktop", &roots, &dedupRoots);
    AddKnownFolderRoot(FOLDERID_PublicDesktop, false, "desktop", &roots, &dedupRoots);

    std::vector<ApplicationCatalogEntry> entries;
    std::unordered_map<std::string, size_t> indexByProcess;
    for (const ScanRoot& root : roots) {
        ScanSingleRoot(root, &entries, &indexByProcess);
    }

    std::sort(entries.begin(), entries.end(), [](const ApplicationCatalogEntry& lhs, const ApplicationCatalogEntry& rhs) {
        const std::string leftLabel = ToLowerAscii(lhs.displayName);
        const std::string rightLabel = ToLowerAscii(rhs.displayName);
        if (leftLabel == rightLabel) {
            return lhs.processName < rhs.processName;
        }
        return leftLabel < rightLabel;
    });
    return entries;
}

} // namespace mousefx::platform::windows
