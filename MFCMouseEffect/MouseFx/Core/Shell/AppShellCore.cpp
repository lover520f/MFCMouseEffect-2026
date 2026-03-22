#include "pch.h"

#include "MouseFx/Core/Shell/AppShellCore.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Control/IpcController.h"
#include "MouseFx/Core/Shell/IDpiAwarenessService.h"
#include "MouseFx/Core/Shell/IEventLoopService.h"
#include "MouseFx/Core/Shell/ISettingsLauncher.h"
#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"
#include "MouseFx/Core/Shell/ITrayService.h"
#include "MouseFx/Core/Shell/IUserNotificationService.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Server/core/WebSettingsServer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Settings/SettingsOptions.h"

namespace mousefx {

namespace {

const char* StartStageToString(AppController::StartStage stage) {
    using S = AppController::StartStage;
    switch (stage) {
    case S::GdiPlusStartup:
        return "GDI+ startup";
    case S::DispatchWindow:
        return "dispatch window";
    case S::EffectInit:
        return "effect initialization";
    case S::GlobalHook:
        return "global mouse hook";
    default:
        return "(unknown)";
    }
}

std::string ExtractJsonValueA(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\"";
    const size_t keyPos = json.find(search);
    if (keyPos == std::string::npos) return "";

    size_t startQuote = json.find('"', keyPos + search.length());
    if (startQuote == std::string::npos) {
        startQuote = json.find('"', keyPos + search.length() + 1);
    }
    if (startQuote == std::string::npos) return "";

    const size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";

    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

std::string ToLowerAsciiCopy(std::string text) {
    for (char& c : text) {
        c = ToLowerAscii(c);
    }
    return text;
}

bool IsZhLanguageToken(const std::string& uiLanguage) {
    const std::string normalized = ToLowerAsciiCopy(uiLanguage);
    return normalized.rfind("zh", 0) == 0;
}

std::wstring ResolveWebSettingsRuntimeInfoPath() {
    std::filesystem::path baseDir(ResolveConfigDirectory());
    return (baseDir / L"websettings_runtime_auto.json").wstring();
}

std::string EscapeJsonString(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (unsigned char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            if (ch < 0x20) {
                std::ostringstream controlEscape;
                controlEscape << "\\u"
                              << std::hex
                              << std::uppercase
                              << std::setw(4)
                              << std::setfill('0')
                              << static_cast<int>(ch);
                escaped += controlEscape.str();
            } else {
                escaped.push_back(static_cast<char>(ch));
            }
            break;
        }
    }
    return escaped;
}

void WriteWebSettingsRuntimeInfo(const WebSettingsServer& server) {
    const std::wstring outPath = ResolveWebSettingsRuntimeInfoPath();
    if (outPath.empty()) {
        return;
    }

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(outPath).parent_path(), ec);
    if (ec) {
        return;
    }

    const std::string url = server.Url();
    const std::string baseUrl = "http://127.0.0.1:" + std::to_string(server.Port());
    const std::string token = server.Token();
    const long long updatedAtUnixMs =
        static_cast<long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());

    std::ofstream out(std::filesystem::path(outPath), std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return;
    }
    out << "{\n"
        << "  \"url\": \"" << EscapeJsonString(url) << "\",\n"
        << "  \"base_url\": \"" << EscapeJsonString(baseUrl) << "\",\n"
        << "  \"token\": \"" << EscapeJsonString(token) << "\",\n"
        << "  \"port\": " << server.Port() << ",\n"
        << "  \"updated_at_unix_ms\": " << updatedAtUnixMs << "\n"
        << "}\n";
}

std::string NormalizeEffectTypeForCategory(EffectCategory category, const std::string& type) {
    switch (category) {
    case EffectCategory::Click:
        return NormalizeClickEffectType(type);
    case EffectCategory::Trail:
        return NormalizeTrailEffectType(type);
    case EffectCategory::Scroll:
        return NormalizeScrollEffectType(type);
    case EffectCategory::Hold:
        return NormalizeHoldEffectType(type);
    case EffectCategory::Hover:
        return NormalizeHoverEffectType(type);
    default:
        return {};
    }
}

const char* CategoryKey(EffectCategory category) {
    switch (category) {
    case EffectCategory::Click:
        return "click";
    case EffectCategory::Trail:
        return "trail";
    case EffectCategory::Scroll:
        return "scroll";
    case EffectCategory::Hold:
        return "hold";
    case EffectCategory::Hover:
        return "hover";
    default:
        return "";
    }
}

std::string CategoryTitle(EffectCategory category, bool preferZhLabels) {
    switch (category) {
    case EffectCategory::Click:
        return preferZhLabels ? u8"点击特效" : "Click Effects";
    case EffectCategory::Trail:
        return preferZhLabels ? u8"拖尾特效" : "Trail Effects";
    case EffectCategory::Scroll:
        return preferZhLabels ? u8"滚轮特效" : "Scroll Effects";
    case EffectCategory::Hold:
        return preferZhLabels ? u8"长按特效" : "Hold Effects";
    case EffectCategory::Hover:
        return preferZhLabels ? u8"悬停特效" : "Hover Effects";
    default:
        return {};
    }
}

bool ParseEffectCategory(const std::string& categoryText, EffectCategory* outCategory) {
    if (outCategory == nullptr) {
        return false;
    }
    if (categoryText == "click") {
        *outCategory = EffectCategory::Click;
        return true;
    }
    if (categoryText == "trail") {
        *outCategory = EffectCategory::Trail;
        return true;
    }
    if (categoryText == "scroll") {
        *outCategory = EffectCategory::Scroll;
        return true;
    }
    if (categoryText == "hold") {
        *outCategory = EffectCategory::Hold;
        return true;
    }
    if (categoryText == "hover") {
        *outCategory = EffectCategory::Hover;
        return true;
    }
    return false;
}

std::string ReadCurrentActiveType(const EffectConfig& config, EffectCategory category) {
    switch (category) {
    case EffectCategory::Click:
        return config.active.click;
    case EffectCategory::Trail:
        return config.active.trail;
    case EffectCategory::Scroll:
        return config.active.scroll;
    case EffectCategory::Hold:
        return config.active.hold;
    case EffectCategory::Hover:
        return config.active.hover;
    default:
        return {};
    }
}

std::string BuildEffectSelectionCommandJson(const std::string& category, const std::string& normalizedType) {
    if (category.empty() || normalizedType.empty()) {
        return {};
    }
    if (normalizedType == "none") {
        return std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
    }
    return std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category + "\",\"type\":\"" +
        normalizedType + "\"}";
}

std::string OptionLabelFromMetadata(const EffectOption& option, bool preferZhLabels) {
    const wchar_t* text = preferZhLabels ? option.displayZh : option.displayEn;
    if (text != nullptr && text[0] != L'\0') {
        return EnsureUtf8(Utf16ToUtf8(text));
    }
    return option.value ? option.value : "";
}

void AppendEffectMenuSection(
    const EffectConfig& config,
    bool preferZhLabels,
    EffectCategory category,
    const EffectOption* (*metadataFn)(size_t&),
    std::vector<ShellEffectMenuSection>* outSections) {
    if (outSections == nullptr || metadataFn == nullptr) {
        return;
    }

    size_t optionCount = 0;
    const EffectOption* options = metadataFn(optionCount);
    if (options == nullptr || optionCount == 0) {
        return;
    }

    const std::string categoryKey = CategoryKey(category);
    if (categoryKey.empty()) {
        return;
    }
    const std::string selectedNormalized = NormalizeEffectTypeForCategory(
        category,
        ReadCurrentActiveType(config, category));

    ShellEffectMenuSection section;
    section.category = categoryKey;
    section.title = CategoryTitle(category, preferZhLabels);
    section.items.reserve(optionCount);
    for (size_t i = 0; i < optionCount; ++i) {
        const EffectOption& option = options[i];
        if (option.value == nullptr || option.value[0] == '\0') {
            continue;
        }
        ShellEffectMenuItem item;
        item.value = NormalizeEffectTypeForCategory(category, option.value);
        item.label = OptionLabelFromMetadata(option, preferZhLabels);
        item.selected = (!item.value.empty() && item.value == selectedNormalized);
        section.items.push_back(std::move(item));
    }
    if (!section.items.empty()) {
        outSections->push_back(std::move(section));
    }
}

constexpr const char* kProjectRepositoryUrl = "https://github.com/sqmw/MFCMouseEffect";

} // namespace

AppShellCore::AppShellCore(ShellPlatformServices services)
    : trayService_(std::move(services.trayService)),
      settingsLauncher_(std::move(services.settingsLauncher)),
      singleInstanceGuard_(std::move(services.singleInstanceGuard)),
      dpiAwarenessService_(std::move(services.dpiAwarenessService)),
      eventLoopService_(std::move(services.eventLoopService)),
      notifier_(std::move(services.notifier)) {}

AppShellCore::~AppShellCore() {
    Shutdown();
}

bool AppShellCore::Initialize(const AppShellStartOptions& options) {
    if (!settingsLauncher_ || !singleInstanceGuard_ || !eventLoopService_) {
        return false;
    }

    if (!singleInstanceGuard_->Acquire(options.singleInstanceKey)) {
        return false;
    }

    if (dpiAwarenessService_) {
        dpiAwarenessService_->EnableForScreenCoords();
    }

    backgroundMode_ = !options.showTrayIcon || !trayService_;

    mouseFx_ = std::make_unique<AppController>();
    mouseFx_->SetRuntimeDiagnosticsEnabled(options.enableRuntimeDiagnostics);
    if (!mouseFx_->Start()) {
#ifdef _DEBUG
        NotifyWarning("MFCMouseEffect", BuildStartupFailureMessage(mouseFx_.get()));
#endif
        mouseFx_.reset();
        singleInstanceGuard_->Release();
        return false;
    }

    if (!backgroundMode_ && trayService_) {
        if (!trayService_->Start(this, options.showTrayIcon)) {
            if (mouseFx_) {
                mouseFx_->Stop();
                mouseFx_.reset();
            }
            singleInstanceGuard_->Release();
            return false;
        }
    }

    ipc_ = std::make_unique<IpcController>();
    ipc_->Start([this](const std::string& cmd) {
        if (IsExitCommand(cmd)) {
            RequestExitFromShell();
            return;
        }
        if (mouseFx_) {
            mouseFx_->HandleCommand(cmd);
        }
    }, [this]() {
        if (backgroundMode_) {
            RequestExitFromShell();
        }
    });

    initialized_ = true;
    return true;
}

int AppShellCore::RunMessageLoop() {
    if (!eventLoopService_) {
        return -1;
    }
    return eventLoopService_->Run();
}

void AppShellCore::Shutdown() {
    if (!initialized_) {
        return;
    }

    if (ipc_) {
        ipc_->Stop();
        ipc_.reset();
    }
    if (webSettings_) {
        webSettings_->Stop();
        webSettings_.reset();
    }
    if (mouseFx_) {
        mouseFx_->Stop();
        mouseFx_.reset();
    }
    if (trayService_) {
        trayService_->Stop();
    }
    if (singleInstanceGuard_) {
        singleInstanceGuard_->Release();
    }

    initialized_ = false;
}

AppController* AppShellCore::AppControllerForShell() noexcept {
    return mouseFx_.get();
}

bool AppShellCore::PreferZhLabelsFromShell(bool fallbackPreferZh) {
    if (!mouseFx_) {
        return fallbackPreferZh;
    }
    const std::string uiLanguage = mouseFx_->Config().uiLanguage;
    if (uiLanguage.empty()) {
        return fallbackPreferZh;
    }
    return IsZhLanguageToken(uiLanguage);
}

void AppShellCore::GetThemeMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellThemeMenuItem>* outItems,
    std::string* outSelectedTheme) {
    if (outItems == nullptr || outSelectedTheme == nullptr) {
        return;
    }
    outItems->clear();
    outSelectedTheme->clear();
    if (!mouseFx_) {
        return;
    }

    const bool effectivePreferZhLabels = PreferZhLabelsFromShell(preferZhLabels);
    *outSelectedTheme = ResolveRuntimeThemeName(mouseFx_->Config().theme);
    const std::vector<ThemeOption> options = GetThemeOptions();
    outItems->reserve(options.size());
    for (const auto& option : options) {
        ShellThemeMenuItem item;
        item.value = option.value;
        const std::wstring& labelWide = effectivePreferZhLabels ? option.labelZh : option.labelEn;
        if (!labelWide.empty()) {
            item.label = EnsureUtf8(Utf16ToUtf8(labelWide.c_str()));
        }
        if (item.label.empty()) {
            item.label = item.value;
        }
        outItems->push_back(std::move(item));
    }
}

void AppShellCore::GetEffectMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellEffectMenuSection>* outSections) {
    if (outSections == nullptr) {
        return;
    }
    outSections->clear();
    if (!mouseFx_) {
        return;
    }

    const bool effectivePreferZhLabels = PreferZhLabelsFromShell(preferZhLabels);
    const EffectConfig config = mouseFx_->GetConfigSnapshot();
    outSections->reserve(5);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Click, ClickMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Trail, TrailMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Scroll, ScrollMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Hold, HoldMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Hover, HoverMetadata, outSections);
}

void AppShellCore::OpenSettingsFromShell() {
    if (!PostShellTask([this]() {
            ShowWebSettings();
        })) {
        ShowWebSettings();
    }
}

void AppShellCore::ReloadConfigFromShell() {
    static constexpr const char* kReloadCommandJson = "{\"cmd\":\"reload_config\"}";
    if (!PostShellTask([this]() {
            if (mouseFx_) {
                mouseFx_->HandleCommand(kReloadCommandJson);
            }
        })) {
        if (mouseFx_) {
            mouseFx_->HandleCommand(kReloadCommandJson);
        }
    }
}

void AppShellCore::OpenProjectRepositoryFromShell() {
    if (!PostShellTask([this]() {
            if (settingsLauncher_ && !settingsLauncher_->OpenUrlUtf8(kProjectRepositoryUrl) && notifier_) {
                notifier_->ShowWarning("MFCMouseEffect", "Open project repository URL failed.");
            }
        })) {
        if (settingsLauncher_ && !settingsLauncher_->OpenUrlUtf8(kProjectRepositoryUrl) && notifier_) {
            notifier_->ShowWarning("MFCMouseEffect", "Open project repository URL failed.");
        }
    }
}

void AppShellCore::RequestExitFromShell() {
    if (!PostShellTask([this]() {
            RequestExitOnLoop();
        })) {
        RequestExitOnLoop();
    }
}

void AppShellCore::SetThemeFromShell(const std::string& theme) {
    const std::string requestedTheme = theme;
    if (requestedTheme.empty()) {
        return;
    }
    if (!PostShellTask([this, requestedTheme]() {
            if (mouseFx_) {
                mouseFx_->SetTheme(requestedTheme);
            }
        })) {
        if (mouseFx_) {
            mouseFx_->SetTheme(requestedTheme);
        }
    }
}

void AppShellCore::SetEffectFromShell(const std::string& category, const std::string& effectType) {
    const std::string categoryTrimmed = TrimAscii(category);
    const std::string effectTrimmed = TrimAscii(effectType);
    EffectCategory effectCategory = EffectCategory::Click;
    if (!ParseEffectCategory(categoryTrimmed, &effectCategory)) {
        return;
    }
    const std::string normalizedType = NormalizeEffectTypeForCategory(effectCategory, effectTrimmed);
    const std::string commandJson = BuildEffectSelectionCommandJson(categoryTrimmed, normalizedType);
    if (commandJson.empty()) {
        return;
    }
    if (!PostShellTask([this, commandJson]() {
            if (mouseFx_) {
                mouseFx_->HandleCommand(commandJson);
            }
        })) {
        if (mouseFx_) {
            mouseFx_->HandleCommand(commandJson);
        }
    }
}

bool AppShellCore::PostShellTask(std::function<void()> task) {
    if (!initialized_ || !eventLoopService_) {
        return false;
    }
    return eventLoopService_->PostTask(std::move(task));
}

void AppShellCore::RequestExitOnLoop() {
    if (trayService_ && !backgroundMode_) {
        trayService_->RequestExit();
    }
    if (eventLoopService_) {
        eventLoopService_->RequestExit();
    }
}

void AppShellCore::ShowWebSettings() {
    if (backgroundMode_ || !mouseFx_) {
        return;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(mouseFx_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            NotifyWarning("MFCMouseEffect", "Web settings server start failed.");
            return;
        }
    }

    WriteWebSettingsRuntimeInfo(*webSettings_);

    if (!settingsLauncher_->OpenUrlUtf8(webSettings_->Url())) {
        NotifyWarning("MFCMouseEffect", "Web settings open failed.");
    }
}

void AppShellCore::NotifyWarning(const char* titleUtf8, const std::string& messageUtf8) {
    if (!notifier_) {
        return;
    }
    notifier_->ShowWarning(titleUtf8 ? titleUtf8 : "MFCMouseEffect", messageUtf8);
}

std::string AppShellCore::BuildStartupFailureMessage(const AppController* controller) {
    if (!controller) {
        return "MouseFx failed to start.";
    }

    const auto diag = controller->Diagnostics();
    std::ostringstream oss;
    oss << "MouseFx failed to start.\n\n"
        << "Stage: " << StartStageToString(diag.stage) << "\n"
        << "Error: " << static_cast<unsigned long>(diag.error) << "\n\n"
        << "Tips:\n"
        << "- Make sure you're running the correct exe (x64\\\\Debug\\\\MFCMouseEffect.exe).\n"
        << "- Try 'Run as administrator' if clicking admin windows.\n"
        << "- Check Visual Studio Output window for 'MouseFx:' logs.";
    return oss.str();
}

bool AppShellCore::IsExitCommand(const std::string& cmd) {
    if (cmd == "exit") return true;
    if (cmd.find("\"cmd\"") != std::string::npos) {
        return ExtractJsonValueA(cmd, "cmd") == "exit";
    }
    return false;
}

} // namespace mousefx
