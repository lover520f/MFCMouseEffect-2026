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
        bool expectEffectAction,
        bool expectReloadAction,
        bool expectStarAction,
        std::string expectedThemeValue,
        std::string expectedEffectCategory,
        std::string expectedEffectValue,
        std::string settingsUrl,
        std::string starProjectUrl,
        std::string launchCaptureFilePath)
        : loop_(loop),
          expectSettingsAction_(expectSettingsAction),
          expectThemeAction_(expectThemeAction),
          expectEffectAction_(expectEffectAction),
          expectReloadAction_(expectReloadAction),
          expectStarAction_(expectStarAction),
          expectedThemeValue_(std::move(expectedThemeValue)),
          expectedEffectCategory_(std::move(expectedEffectCategory)),
          expectedEffectValue_(std::move(expectedEffectValue)),
          settingsUrl_(std::move(settingsUrl)),
          starProjectUrl_(std::move(starProjectUrl)),
          launchCaptureFilePath_(std::move(launchCaptureFilePath)) {}

    mousefx::AppController* AppControllerForShell() noexcept override {
        return nullptr;
    }

    bool PreferZhLabelsFromShell(bool fallbackPreferZh) override {
        return fallbackPreferZh;
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

    void GetEffectMenuSnapshotFromShell(
        bool preferZhLabels,
        std::vector<mousefx::ShellEffectMenuSection>* outSections) override {
        if (outSections != nullptr) {
            outSections->clear();
            if (expectEffectAction_) {
                mousefx::ShellEffectMenuSection section;
                section.category = expectedEffectCategory_;
                section.title = preferZhLabels ? u8"点击特效" : "Click Effects";
                section.items.push_back(
                    {expectedEffectValue_, expectedEffectValue_, true});
                section.items.push_back({"none", "none", false});
                outSections->push_back(std::move(section));
            }
        }
    }

    void OpenSettingsFromShell() override {
        ++settingsActionCount_;
        settingsLaunchOk_ = settingsLauncher_.OpenUrlUtf8(settingsUrl_);
        MaybeRequestExit();
    }

    void ReloadConfigFromShell() override {
        ++reloadActionCount_;
        MaybeRequestExit();
    }

    void OpenProjectRepositoryFromShell() override {
        ++starActionCount_;
        starLaunchOk_ = settingsLauncher_.OpenUrlUtf8(starProjectUrl_);
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

    void SetEffectFromShell(const std::string& category, const std::string& effectType) override {
        selectedEffectCategory_ = category;
        selectedEffectValue_ = effectType;
        ++effectActionCount_;
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
    bool HasEffectAction() const {
        return effectActionCount_ > 0;
    }
    const std::string& SelectedEffectCategory() const {
        return selectedEffectCategory_;
    }
    const std::string& SelectedEffectValue() const {
        return selectedEffectValue_;
    }
    const std::string& ExpectedEffectCategory() const {
        return expectedEffectCategory_;
    }
    const std::string& ExpectedEffectValue() const {
        return expectedEffectValue_;
    }
    bool HasReloadAction() const {
        return reloadActionCount_ > 0;
    }
    bool HasStarAction() const {
        return starActionCount_ > 0;
    }
    bool StarLaunchOk() const {
        return starLaunchOk_;
    }
    const std::string& StarProjectUrl() const {
        return starProjectUrl_;
    }

private:
    void MaybeRequestExit() {
        if (loop_ == nullptr) {
            return;
        }
        const bool settingsSatisfied = !expectSettingsAction_ || settingsActionCount_ > 0;
        const bool themeSatisfied = !expectThemeAction_ || themeActionCount_ > 0;
        const bool effectSatisfied = !expectEffectAction_ || effectActionCount_ > 0;
        const bool reloadSatisfied = !expectReloadAction_ || reloadActionCount_ > 0;
        const bool starSatisfied = !expectStarAction_ || starActionCount_ > 0;
        if (settingsSatisfied && themeSatisfied && effectSatisfied && reloadSatisfied && starSatisfied) {
            loop_->RequestExit();
        }
    }

    mousefx::MacosEventLoopService* loop_ = nullptr;
    bool expectSettingsAction_ = false;
    bool expectThemeAction_ = false;
    bool expectEffectAction_ = false;
    bool expectReloadAction_ = false;
    bool expectStarAction_ = false;
    std::string expectedThemeValue_{};
    std::string expectedEffectCategory_{};
    std::string expectedEffectValue_{};
    std::string settingsUrl_{};
    std::string starProjectUrl_{};
    std::string launchCaptureFilePath_{};
    mousefx::MacosSettingsLauncher settingsLauncher_{};
    bool settingsLaunchOk_ = false;
    bool starLaunchOk_ = false;
    size_t settingsActionCount_ = 0;
    size_t themeActionCount_ = 0;
    size_t effectActionCount_ = 0;
    size_t reloadActionCount_ = 0;
    size_t starActionCount_ = 0;
    std::string selectedThemeValue_{};
    std::string selectedEffectCategory_{};
    std::string selectedEffectValue_{};
};

} // namespace

int main(int argc, char* argv[]) {
    const bool expectSettingsAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_SETTINGS_ACTION");
    const bool expectThemeAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_THEME_ACTION");
    const bool expectEffectAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_EFFECT_ACTION");
    const bool expectReloadAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_RELOAD_ACTION");
    const bool expectStarAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_STAR_ACTION");
    std::string settingsUrl = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_SETTINGS_URL",
        "http://127.0.0.1:9527/?token=tray-smoke");
    std::string starProjectUrl = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_STAR_URL",
        "https://github.com/sqmw/MFCMouseEffect");
    std::string themeValue = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_THEME_VALUE",
        "neon");
    std::string effectCategory = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_EFFECT_CATEGORY",
        "click");
    std::string effectValue = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_EFFECT_VALUE",
        "ripple");
    std::string launchCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE",
        "");
    std::string themeCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_THEME_CAPTURE_FILE",
        "");
    std::string effectCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_EFFECT_CAPTURE_FILE",
        "");
    std::string reloadCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_RELOAD_CAPTURE_FILE",
        "");
    std::string starCaptureFilePath = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_STAR_CAPTURE_FILE",
        "");

    bool forceExpectSettingsAction = false;
    bool forceExpectThemeAction = false;
    bool forceExpectEffectAction = false;
    bool forceExpectReloadAction = false;
    bool forceExpectStarAction = false;
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
        if (arg == "--expect-effect-action") {
            forceExpectEffectAction = true;
            continue;
        }
        if (arg == "--expect-reload-action") {
            forceExpectReloadAction = true;
            continue;
        }
        if (arg == "--expect-star-action") {
            forceExpectStarAction = true;
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
        if (arg == "--star-url") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --star-url\n");
                return 64;
            }
            starProjectUrl = argv[++i];
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
        if (arg == "--effect-category") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --effect-category\n");
                return 64;
            }
            effectCategory = argv[++i];
            continue;
        }
        if (arg == "--effect-value") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --effect-value\n");
                return 64;
            }
            effectValue = argv[++i];
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
        if (arg == "--effect-capture-file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --effect-capture-file\n");
                return 64;
            }
            effectCaptureFilePath = argv[++i];
            continue;
        }
        if (arg == "--reload-capture-file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --reload-capture-file\n");
                return 64;
            }
            reloadCaptureFilePath = argv[++i];
            continue;
        }
        if (arg == "--star-capture-file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "mfx_shell_macos_tray_smoke: missing value for --star-capture-file\n");
                return 64;
            }
            starCaptureFilePath = argv[++i];
            continue;
        }
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: unknown argument: %.*s\n", static_cast<int>(arg.size()), arg.data());
        return 64;
    }

    const bool expectSettingsActionEffective = expectSettingsAction || forceExpectSettingsAction;
    const bool expectThemeActionEffective = expectThemeAction || forceExpectThemeAction;
    const bool expectEffectActionEffective = expectEffectAction || forceExpectEffectAction;
    const bool expectReloadActionEffective = expectReloadAction || forceExpectReloadAction;
    const bool expectStarActionEffective = expectStarAction || forceExpectStarAction;
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
    if (expectEffectActionEffective) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_CATEGORY", effectCategory.c_str(), 1);
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_VALUE", effectValue.c_str(), 1);
    } else {
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_CATEGORY");
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_VALUE");
    }
    if (expectReloadActionEffective) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_RELOAD_ACTION", "1", 1);
    } else {
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_RELOAD_ACTION");
    }
    if (expectStarActionEffective) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_STAR_ACTION", "1", 1);
    } else {
        unsetenv("MFX_TEST_TRAY_AUTO_TRIGGER_STAR_ACTION");
    }

    mousefx::MacosEventLoopService loop;
    mousefx::MacosTrayService tray;
    TraySmokeHost host(
        &loop,
        expectSettingsActionEffective,
        expectThemeActionEffective,
        expectEffectActionEffective,
        expectReloadActionEffective,
        expectStarActionEffective,
        themeValue,
        effectCategory,
        effectValue,
        settingsUrl,
        starProjectUrl,
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
    if (expectEffectActionEffective && !host.HasEffectAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: effect action was not triggered\n");
        return 9;
    }
    if (expectEffectActionEffective &&
        (host.SelectedEffectCategory() != host.ExpectedEffectCategory() ||
         host.SelectedEffectValue() != host.ExpectedEffectValue())) {
        std::fprintf(
            stderr,
            "mfx_shell_macos_tray_smoke: selected effect mismatch (expected=%s:%s actual=%s:%s)\n",
            host.ExpectedEffectCategory().c_str(),
            host.ExpectedEffectValue().c_str(),
            host.SelectedEffectCategory().c_str(),
            host.SelectedEffectValue().c_str());
        return 10;
    }
    if (expectReloadActionEffective && !host.HasReloadAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: reload action was not triggered\n");
        return 12;
    }
    if (expectStarActionEffective && !host.HasStarAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: star action was not triggered\n");
        return 13;
    }
    if (expectStarActionEffective && !host.StarLaunchOk()) {
        std::fprintf(
            stderr,
            "mfx_shell_macos_tray_smoke: star launch failed for url: %s\n",
            host.StarProjectUrl().c_str());
        return 14;
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
    if (expectEffectActionEffective && !effectCaptureFilePath.empty()) {
        const bool wroteEffectCapture = mousefx::WritePosixKeyValueCaptureFile(
            effectCaptureFilePath,
            {
                {"command", "effect_select"},
                {"expected_category", host.ExpectedEffectCategory()},
                {"expected_value", host.ExpectedEffectValue()},
                {"selected_category", host.SelectedEffectCategory()},
                {"selected_value", host.SelectedEffectValue()},
                {"captured", "1"},
            });
        if (!wroteEffectCapture) {
            std::fprintf(
                stderr,
                "mfx_shell_macos_tray_smoke: failed to write effect capture file: %s\n",
                effectCaptureFilePath.c_str());
            return 11;
        }
    }
    if (expectReloadActionEffective && !reloadCaptureFilePath.empty()) {
        const bool wroteReloadCapture = mousefx::WritePosixKeyValueCaptureFile(
            reloadCaptureFilePath,
            {
                {"command", "reload_config"},
                {"captured", "1"},
            });
        if (!wroteReloadCapture) {
            std::fprintf(
                stderr,
                "mfx_shell_macos_tray_smoke: failed to write reload capture file: %s\n",
                reloadCaptureFilePath.c_str());
            return 15;
        }
    }
    if (expectStarActionEffective && !starCaptureFilePath.empty()) {
        const bool wroteStarCapture = mousefx::WritePosixKeyValueCaptureFile(
            starCaptureFilePath,
            {
                {"command", "star_project"},
                {"url", host.StarProjectUrl()},
                {"captured", "1"},
            });
        if (!wroteStarCapture) {
            std::fprintf(
                stderr,
                "mfx_shell_macos_tray_smoke: failed to write star capture file: %s\n",
                starCaptureFilePath.c_str());
            return 16;
        }
    }
    return 0;
}
