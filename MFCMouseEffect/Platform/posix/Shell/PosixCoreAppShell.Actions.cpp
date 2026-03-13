#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Server/core/WebSettingsServer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Settings/SettingsOptions.h"

#include <functional>
#include <utility>

namespace mousefx::platform {
namespace {

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

bool PosixCoreAppShell::PreferZhLabelsFromShell(bool fallbackPreferZh) {
    if (!appController_) {
        return fallbackPreferZh;
    }
    const std::string uiLanguage = appController_->Config().uiLanguage;
    if (uiLanguage.empty()) {
        return fallbackPreferZh;
    }
    return IsZhLanguageToken(uiLanguage);
}

void PosixCoreAppShell::GetThemeMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellThemeMenuItem>* outItems,
    std::string* outSelectedTheme) {
    if (outItems == nullptr || outSelectedTheme == nullptr) {
        return;
    }
    outItems->clear();
    outSelectedTheme->clear();
    if (!appController_) {
        return;
    }

    const bool effectivePreferZhLabels = PreferZhLabelsFromShell(preferZhLabels);
    *outSelectedTheme = ResolveRuntimeThemeName(appController_->Config().theme);
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

void PosixCoreAppShell::GetEffectMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellEffectMenuSection>* outSections) {
    if (outSections == nullptr) {
        return;
    }
    outSections->clear();
    if (!appController_) {
        return;
    }

    const bool effectivePreferZhLabels = PreferZhLabelsFromShell(preferZhLabels);
    const EffectConfig config = appController_->GetConfigSnapshot();
    outSections->reserve(5);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Click, ClickMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Trail, TrailMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Scroll, ScrollMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Hold, HoldMetadata, outSections);
    AppendEffectMenuSection(config, effectivePreferZhLabels, EffectCategory::Hover, HoverMetadata, outSections);
}

void PosixCoreAppShell::OpenSettingsFromShell() {
    if (!PostShellTask([this]() {
            ShowWebSettings();
        })) {
        ShowWebSettings();
    }
}

void PosixCoreAppShell::ReloadConfigFromShell() {
    static constexpr const char* kReloadCommandJson = "{\"cmd\":\"reload_config\"}";
    if (!PostShellTask([this]() {
            if (appController_) {
                appController_->HandleCommand(kReloadCommandJson);
            }
        })) {
        if (appController_) {
            appController_->HandleCommand(kReloadCommandJson);
        }
    }
}

void PosixCoreAppShell::OpenProjectRepositoryFromShell() {
    if (!PostShellTask([this]() {
            if (services_.settingsLauncher &&
                !services_.settingsLauncher->OpenUrlUtf8(kProjectRepositoryUrl) &&
                services_.notifier) {
                services_.notifier->ShowWarning("MFCMouseEffect", "Open project repository URL failed.");
            }
        })) {
        if (services_.settingsLauncher &&
            !services_.settingsLauncher->OpenUrlUtf8(kProjectRepositoryUrl) &&
            services_.notifier) {
            services_.notifier->ShowWarning("MFCMouseEffect", "Open project repository URL failed.");
        }
    }
}

void PosixCoreAppShell::RequestExitFromShell() {
    if (!PostShellTask([this]() {
            RequestExitOnLoop();
        })) {
        RequestExitOnLoop();
    }
}

void PosixCoreAppShell::SetThemeFromShell(const std::string& theme) {
    const std::string requestedTheme = theme;
    if (requestedTheme.empty()) {
        return;
    }
    if (!PostShellTask([this, requestedTheme]() {
            if (appController_) {
                appController_->SetTheme(requestedTheme);
            }
        })) {
        if (appController_) {
            appController_->SetTheme(requestedTheme);
        }
    }
}

void PosixCoreAppShell::SetEffectFromShell(const std::string& category, const std::string& effectType) {
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
            if (appController_) {
                appController_->HandleCommand(commandJson);
            }
        })) {
        if (appController_) {
            appController_->HandleCommand(commandJson);
        }
    }
}

bool PosixCoreAppShell::PostShellTask(std::function<void()> task) {
    if (!initialized_ || !services_.eventLoopService || !task) {
        return false;
    }
    return services_.eventLoopService->PostTask(std::move(task));
}

void PosixCoreAppShell::RequestExitOnLoop() {
    if (services_.trayService && !backgroundMode_) {
        services_.trayService->RequestExit();
    }
    if (services_.eventLoopService) {
        services_.eventLoopService->RequestExit();
    }
}

void PosixCoreAppShell::ShowWebSettings() {
    if (backgroundMode_ || !appController_ || !services_.settingsLauncher) {
        return;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(appController_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            if (services_.notifier) {
                services_.notifier->ShowWarning("MFCMouseEffect", "Web settings server start failed.");
            }
            return;
        }
    }

    if (!services_.settingsLauncher->OpenUrlUtf8(webSettings_->Url()) && services_.notifier) {
        services_.notifier->ShowWarning("MFCMouseEffect", "Open core settings URL failed.");
    }
}

} // namespace mousefx::platform

#endif
