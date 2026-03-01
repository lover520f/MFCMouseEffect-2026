#include "pch.h"

#include "Platform/PlatformStartupOptionsFactory.h"

#include <cstdlib>
#include <string>
#include <string_view>

#include "Platform/PlatformTarget.h"

#if MFX_PLATFORM_WINDOWS
#include <shellapi.h>
#endif

namespace mousefx::platform {

namespace {

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

bool StartsWithIgnoreCaseAscii(std::string_view value, std::string_view prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (ToLowerAscii(value[i]) != ToLowerAscii(prefix[i])) {
            return false;
        }
    }
    return true;
}

bool TryParseInlineMode(std::string_view arg, std::string_view* outMode) {
    if (!outMode) {
        return false;
    }
    constexpr std::string_view kModePrefixShort = "-mode=";
    constexpr std::string_view kModePrefixLong = "--mode=";
    if (StartsWithIgnoreCaseAscii(arg, kModePrefixShort)) {
        *outMode = arg.substr(kModePrefixShort.size());
        return true;
    }
    if (StartsWithIgnoreCaseAscii(arg, kModePrefixLong)) {
        *outMode = arg.substr(kModePrefixLong.size());
        return true;
    }
    return false;
}

bool TryParseInlineSingleInstanceKey(std::string_view arg, std::string_view* outKey) {
    if (!outKey) {
        return false;
    }
    constexpr std::string_view kPrefixes[] = {
        "-single-instance-key=",
        "--single-instance-key=",
        "-single-instance=",
        "--single-instance=",
    };
    for (const auto prefix : kPrefixes) {
        if (!StartsWithIgnoreCaseAscii(arg, prefix)) {
            continue;
        }
        *outKey = arg.substr(prefix.size());
        return true;
    }
    return false;
}

void ApplyModeArg(std::string_view mode, AppShellStartOptions* options) {
    if (!options) {
        return;
    }
    if (EqualsIgnoreCaseAscii(mode, "background")) {
        options->showTrayIcon = false;
        return;
    }
    if (EqualsIgnoreCaseAscii(mode, "tray") || EqualsIgnoreCaseAscii(mode, "normal")) {
        options->showTrayIcon = true;
    }
}

void ApplySingleInstanceKeyArg(std::string_view key, AppShellStartOptions* options) {
    if (!options || key.empty()) {
        return;
    }
    options->singleInstanceKey.assign(key.begin(), key.end());
}

void ApplySingleInstanceKeyEnv(AppShellStartOptions* options) {
    if (!options) {
        return;
    }
    const char* envKey = std::getenv("MFX_SINGLE_INSTANCE_KEY");
    if (!envKey || *envKey == '\0') {
        return;
    }
    ApplySingleInstanceKeyArg(envKey, options);
}

#if MFX_PLATFORM_WINDOWS
std::string WideToUtf8(const wchar_t* wideText) {
    if (!wideText || *wideText == L'\0') {
        return {};
    }

    const int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 1) {
        return {};
    }

    std::string utf8(static_cast<size_t>(utf8Size - 1), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wideText,
        -1,
        utf8.data(),
        utf8Size - 1,
        nullptr,
        nullptr);
    return utf8;
}
#endif

AppShellStartOptions ParseStartupOptionsFromArgs(const PlatformEntryArgs& entryArgs) {
    AppShellStartOptions options{};
    ApplySingleInstanceKeyEnv(&options);
    const auto& args = entryArgs.argvUtf8;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        std::string_view inlineKey{};
        if (TryParseInlineSingleInstanceKey(arg, &inlineKey)) {
            ApplySingleInstanceKeyArg(inlineKey, &options);
            continue;
        }
        std::string_view inlineMode{};
        if (TryParseInlineMode(arg, &inlineMode)) {
            ApplyModeArg(inlineMode, &options);
            continue;
        }

        if (EqualsIgnoreCaseAscii(arg, "-mode") || EqualsIgnoreCaseAscii(arg, "--mode")) {
            if (i + 1 >= args.size()) {
                continue;
            }
            ApplyModeArg(args[i + 1], &options);
            continue;
        }
        if (EqualsIgnoreCaseAscii(arg, "-single-instance-key") ||
            EqualsIgnoreCaseAscii(arg, "--single-instance-key") ||
            EqualsIgnoreCaseAscii(arg, "-single-instance") ||
            EqualsIgnoreCaseAscii(arg, "--single-instance")) {
            if (i + 1 >= args.size()) {
                continue;
            }
            ApplySingleInstanceKeyArg(args[i + 1], &options);
            continue;
        }
    }
    return options;
}

} // namespace

AppShellStartOptions CreatePlatformStartupOptions() {
#if MFX_PLATFORM_WINDOWS
    PlatformEntryArgs entryArgs{};
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return AppShellStartOptions{};
    }
    entryArgs.argvUtf8.reserve(static_cast<size_t>(argc));

    for (int i = 0; i < argc; ++i) {
        entryArgs.argvUtf8.push_back(WideToUtf8(argv[i]));
    }

    LocalFree(argv);
    return ParseStartupOptionsFromArgs(entryArgs);
#else
    return AppShellStartOptions{};
#endif
}

AppShellStartOptions CreatePlatformStartupOptions(const PlatformEntryArgs& entryArgs) {
    return ParseStartupOptionsFromArgs(entryArgs);
}

} // namespace mousefx::platform
