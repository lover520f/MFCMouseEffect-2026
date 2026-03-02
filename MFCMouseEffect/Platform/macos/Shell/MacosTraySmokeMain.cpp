#include "Platform/macos/Shell/MacosEventLoopService.h"
#include "Platform/macos/Shell/MacosSettingsLauncher.h"
#include "Platform/macos/Shell/MacosTrayService.h"
#include "Platform/posix/Shell/PosixKeyValueCaptureFile.h"

#include "MouseFx/Core/Shell/IAppShellHost.h"

#import <dispatch/dispatch.h>

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

bool ReadBoolEnv(const char* key) {
    if (key == nullptr || key[0] == '\0') {
        return false;
    }
    const char* raw = std::getenv(key);
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

std::string ReadStringEnvOrDefault(const char* key, const char* defaultValue) {
    const char* raw = std::getenv(key);
    if (raw == nullptr || raw[0] == '\0') {
        return (defaultValue == nullptr) ? std::string() : std::string(defaultValue);
    }
    return std::string(raw);
}

class TraySmokeHost final : public mousefx::IAppShellHost {
public:
    TraySmokeHost(
        mousefx::MacosEventLoopService* loop,
        bool expectSettingsAction,
        bool expectThemeAction,
        std::string expectedThemeValue,
        std::string settingsUrl,
        std::string launchCaptureFilePath)
        : loop_(loop),
          expectSettingsAction_(expectSettingsAction),
          expectThemeAction_(expectThemeAction),
          expectedThemeValue_(std::move(expectedThemeValue)),
          settingsUrl_(std::move(settingsUrl)),
          launchCaptureFilePath_(std::move(launchCaptureFilePath)) {}

    mousefx::AppController* AppControllerForShell() noexcept override {
        return nullptr;
    }

    void GetThemeMenuSnapshotFromShell(
        bool preferZhLabels,
        std::vector<mousefx::ShellThemeMenuItem>* outItems,
        std::string* outSelectedTheme) override {
        (void)preferZhLabels;
        if (outItems != nullptr) {
            outItems->clear();
            if (expectThemeAction_) {
                outItems->push_back({expectedThemeValue_, expectedThemeValue_});
                if (expectedThemeValue_ != "default") {
                    outItems->push_back({"default", "default"});
                }
            }
        }
        if (outSelectedTheme != nullptr) {
            *outSelectedTheme = expectThemeAction_ ? expectedThemeValue_ : std::string();
        }
    }

    void OpenSettingsFromShell() override {
        ++settingsActionCount_;
        settingsLaunchOk_ = settingsLauncher_.OpenUrlUtf8(settingsUrl_);
        MaybeRequestExit();
    }

    void RequestExitFromShell() override {
        if (loop_ != nullptr) {
            loop_->RequestExit();
        }
    }

    void SetThemeFromShell(const std::string& theme) override {
        selectedThemeValue_ = theme;
        ++themeActionCount_;
        MaybeRequestExit();
    }

    bool HasSettingsAction() const {
        return settingsActionCount_ > 0;
    }

    bool SettingsLaunchOk() const {
        return settingsLaunchOk_;
    }

    const std::string& SettingsUrl() const {
        return settingsUrl_;
    }
    const std::string& LaunchCaptureFilePath() const {
        return launchCaptureFilePath_;
    }
    bool HasThemeAction() const {
        return themeActionCount_ > 0;
    }
    const std::string& SelectedThemeValue() const {
        return selectedThemeValue_;
    }
    const std::string& ExpectedThemeValue() const {
        return expectedThemeValue_;
    }

private:
    void MaybeRequestExit() {
        if (loop_ == nullptr) {
            return;
        }
        const bool settingsSatisfied = !expectSettingsAction_ || settingsActionCount_ > 0;
        const bool themeSatisfied = !expectThemeAction_ || themeActionCount_ > 0;
        if (settingsSatisfied && themeSatisfied) {
            loop_->RequestExit();
        }
    }

    mousefx::MacosEventLoopService* loop_ = nullptr;
    bool expectSettingsAction_ = false;
    bool expectThemeAction_ = false;
    std::string expectedThemeValue_{};
    std::string settingsUrl_{};
    std::string launchCaptureFilePath_{};
    mousefx::MacosSettingsLauncher settingsLauncher_{};
    bool settingsLaunchOk_ = false;
    size_t settingsActionCount_ = 0;
    size_t themeActionCount_ = 0;
    std::string selectedThemeValue_{};
};

} // namespace

int main(int argc, char* argv[]) {
    const bool expectSettingsAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_SETTINGS_ACTION");
    const bool expectThemeAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_THEME_ACTION");
    std::string settingsUrl = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_SETTINGS_URL",
        "http://127.0.0.1:9527/?token=tray-smoke");
    std::string themeValue = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_THEME_VALUE",
        "neon");
    std::string launchCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE",
        "");
    std::string themeCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_THEME_CAPTURE_FILE",
        "");

    bool forceExpectSettingsAction = false;
    bool forceExpectThemeAction = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--expect-settings-action") {
            forceExpectSettingsAction = true;
            continue;
        }
        if (arg == "--expect-theme-action") {
            forceExpectThemeAction = true;
            continue;
        }
        if (arg == "--settings-url") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --settings-url\n");
                return 64;
            }
            settingsUrl = argv[++i];
            continue;
        }
        if (arg == "--theme-value") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --theme-value\n");
                return 64;
            }
            themeValue = argv[++i];
            continue;
        }
        if (arg == "--launch-capture-file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --launch-capture-file\n");
                return 64;
            }
            launchCaptureFilePath = argv[++i];
            continue;
        }
        if (arg == "--theme-capture-file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --theme-capture-file\n");
                return 64;
            }
            themeCaptureFilePath = argv[++i];
            continue;
        }
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: unknown argument: %.*s\n", static_cast<int>(arg.size()), arg.data());
        return 64;
    }

    const bool expectSettingsActionEffective = expectSettingsAction || forceExpectSettingsAction;
    const bool expectThemeActionEffective = expectThemeAction || forceExpectThemeAction;
    if (!launchCaptureFilePath.empty()) {
        setenv("MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE", launchCaptureFilePath.c_str(), 1);
    }
    if (expectSettingsActionEffective) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_SETTINGS_ACTION", "1", 1);
    } else {
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_SETTINGS_ACTION");
    }
    if (expectThemeActionEffective) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_THEME_VALUE", themeValue.c_str(), 1);
    } else {
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_THEME_VALUE");
    }

    mousefx::MacosEventLoopService loop;
    mousefx::MacosTrayService tray;
    TraySmokeHost host(
        &loop,
        expectSettingsActionEffective,
        expectThemeActionEffective,
        themeValue,
        settingsUrl,
        launchCaptureFilePath);
    TraySmokeHost* hostPtr = &host;

    if (!tray.Start(&host, true)) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: failed to start tray service\n");
        return 2;
    }

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(800) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (hostPtr != nullptr) {
              hostPtr->RequestExitFromShell();
          }
        });

    const int code = loop.Run();
    tray.Stop();
    if (code != 0) {
        return code;
    }

    if (expectSettingsActionEffective && !host.HasSettingsAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: settings action was not triggered\n");
        return 3;
    }
    if (expectSettingsActionEffective && !host.SettingsLaunchOk()) {
        std::fprintf(
            stderr,
            "mfx_shell_macos_tray_smoke: settings launch failed for url: %s\n",
            host.SettingsUrl().c_str());
        return 4;
    }
    if (expectThemeActionEffective && !host.HasThemeAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: theme action was not triggered\n");
        return 6;
    }
    if (expectThemeActionEffective && host.SelectedThemeValue() != host.ExpectedThemeValue()) {
        std::fprintf(
            stderr,
            "mfx_shell_macos_tray_smoke: selected theme mismatch (expected=%s actual=%s)\n",
            host.ExpectedThemeValue().c_str(),
            host.SelectedThemeValue().c_str());
        return 7;
    }
    if (expectSettingsActionEffective && !host.LaunchCaptureFilePath().empty()) {
        const bool wrote = mousefx::WritePosixKeyValueCaptureFile(
            host.LaunchCaptureFilePath(),
            {
                {"command", "open"},
                {"url", host.SettingsUrl()},
                {"captured", "1"},
            });
        if (!wrote) {
            std::fprintf(
                stderr,
                "mfx_shell_macos_tray_smoke: failed to write launch capture file: %s\n",
                host.LaunchCaptureFilePath().c_str());
            return 5;
        }
    }
    if (expectThemeActionEffective && !themeCaptureFilePath.empty()) {
        const bool wroteThemeCapture = mousefx::WritePosixKeyValueCaptureFile(
            themeCaptureFilePath,
            {
                {"command", "theme_select"},
                {"expected_theme", host.ExpectedThemeValue()},
                {"selected_theme", host.SelectedThemeValue()},
                {"captured", "1"},
            });
        if (!wroteThemeCapture) {
            std::fprintf(
                stderr,
                "mfx_shell_macos_tray_smoke: failed to write theme capture file: %s\n",
                themeCaptureFilePath.c_str());
            return 8;
        }
    }
    return 0;
}
