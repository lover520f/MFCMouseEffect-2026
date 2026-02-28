#include "Platform/macos/Shell/MacosEventLoopService.h"
#include "Platform/macos/Shell/MacosSettingsLauncher.h"
#include "Platform/macos/Shell/MacosTrayService.h"

#include "MouseFx/Core/Shell/IAppShellHost.h"

#import <dispatch/dispatch.h>

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

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
    TraySmokeHost(mousefx::MacosEventLoopService* loop, bool expectSettingsAction, std::string settingsUrl)
        : loop_(loop),
          expectSettingsAction_(expectSettingsAction),
          settingsUrl_(std::move(settingsUrl)) {}

    mousefx::AppController* AppControllerForShell() noexcept override {
        return nullptr;
    }

    void OpenSettingsFromShell() override {
        ++settingsActionCount_;
        settingsLaunchOk_ = settingsLauncher_.OpenUrlUtf8(settingsUrl_);
        if (expectSettingsAction_ && loop_ != nullptr) {
            loop_->RequestExit();
        }
    }

    void RequestExitFromShell() override {
        if (loop_ != nullptr) {
            loop_->RequestExit();
        }
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

private:
    mousefx::MacosEventLoopService* loop_ = nullptr;
    bool expectSettingsAction_ = false;
    std::string settingsUrl_{};
    mousefx::MacosSettingsLauncher settingsLauncher_{};
    bool settingsLaunchOk_ = false;
    size_t settingsActionCount_ = 0;
};

} // namespace

int main() {
    const bool expectSettingsAction = ReadBoolEnv("MFX_TEST_TRAY_SMOKE_EXPECT_SETTINGS_ACTION");
    const std::string settingsUrl = ReadStringEnvOrDefault(
        "MFX_TEST_TRAY_SMOKE_SETTINGS_URL",
        "http://127.0.0.1:9527/?token=tray-smoke");

    if (expectSettingsAction) {
        setenv("MFX_TEST_TRAY_AUTO_TRIGGER_SETTINGS_ACTION", "1", 1);
    }

    mousefx::MacosEventLoopService loop;
    mousefx::MacosTrayService tray;
    TraySmokeHost host(&loop, expectSettingsAction, settingsUrl);
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

    if (expectSettingsAction && !host.HasSettingsAction()) {
        std::fprintf(stderr, "mfx_shell_macos_tray_smoke: settings action was not triggered\n");
        return 3;
    }
    if (expectSettingsAction && !host.SettingsLaunchOk()) {
        std::fprintf(
            stderr,
            "mfx_shell_macos_tray_smoke: settings launch failed for url: %s\n",
            host.SettingsUrl().c_str());
        return 4;
    }
    return 0;
}
