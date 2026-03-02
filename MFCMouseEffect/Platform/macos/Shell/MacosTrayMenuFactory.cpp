#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.h"

#include "Platform/macos/Shell/MacosTrayMenuSwiftBridge.h"

#include <cctype>
#include <string>
#include <vector>

namespace mousefx::macos_tray {

#if defined(__APPLE__)
namespace {

struct TrayEffectMenuSection final {
    const char* category = "";
    std::string title;
    std::string selectedValue;
    std::vector<std::string> values;
    std::vector<std::string> labels;
    std::vector<const char*> valuePointers;
    std::vector<const char*> labelPointers;
};

std::string TrimAsciiWhitespace(std::string value) {
    size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

const char* NormalizeOrFallback(const std::string& value, const char* fallback) {
    if (value.empty()) {
        return fallback;
    }
    return value.c_str();
}

void BuildPointerViews(TrayEffectMenuSection* section) {
    if (section == nullptr) {
        return;
    }
    section->valuePointers.clear();
    section->labelPointers.clear();
    section->valuePointers.reserve(section->values.size());
    section->labelPointers.reserve(section->labels.size());
    for (size_t i = 0; i < section->values.size(); ++i) {
        section->valuePointers.push_back(section->values[i].c_str());
        section->labelPointers.push_back(section->labels[i].c_str());
    }
}

TrayEffectMenuSection* FindEffectSectionByCategory(
    const std::string& category,
    TrayEffectMenuSection* click,
    TrayEffectMenuSection* trail,
    TrayEffectMenuSection* scroll,
    TrayEffectMenuSection* hold,
    TrayEffectMenuSection* hover) {
    if (category == "click") {
        return click;
    }
    if (category == "trail") {
        return trail;
    }
    if (category == "scroll") {
        return scroll;
    }
    if (category == "hold") {
        return hold;
    }
    if (category == "hover") {
        return hover;
    }
    return nullptr;
}

void OnOpenSettings(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->OpenSettingsFromShell();
    }
}

void OnReloadConfig(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->ReloadConfigFromShell();
    }
}

void OnExit(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->RequestExitFromShell();
    }
}

void OnOpenProjectRepository(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->OpenProjectRepositoryFromShell();
    }
}

void OnSelectTheme(void* context, const char* themeValueUtf8) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host == nullptr || themeValueUtf8 == nullptr) {
        return;
    }
    const std::string themeValue = TrimAsciiWhitespace(themeValueUtf8);
    if (themeValue.empty()) {
        return;
    }
    host->SetThemeFromShell(themeValue);
}

void OnSelectEffect(void* context, const char* categoryUtf8, const char* effectTypeUtf8) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host == nullptr || categoryUtf8 == nullptr || effectTypeUtf8 == nullptr) {
        return;
    }
    const std::string category = TrimAsciiWhitespace(categoryUtf8);
    const std::string effectType = TrimAsciiWhitespace(effectTypeUtf8);
    if (category.empty() || effectType.empty()) {
        return;
    }
    host->SetEffectFromShell(category, effectType);
}

} // namespace

bool BuildMacosTrayMenu(
    IAppShellHost* host,
    const MacosTrayMenuText& menuText,
    MacosTrayMenuObjects* outObjects) {
    if (host == nullptr || outObjects == nullptr) {
        return false;
    }

    const bool preferZhLabels = host->PreferZhLabelsFromShell(menuText.preferZhLabels);
    const MacosTrayMenuText resolvedMenuText = BuildMacosTrayMenuText(preferZhLabels);

    std::vector<ShellThemeMenuItem> themeOptions;
    std::string selectedTheme;
    host->GetThemeMenuSnapshotFromShell(preferZhLabels, &themeOptions, &selectedTheme);

    std::vector<std::string> themeValues;
    std::vector<std::string> themeLabels;
    std::vector<const char*> themeValuePointers;
    std::vector<const char*> themeLabelPointers;
    themeValues.reserve(themeOptions.size());
    themeLabels.reserve(themeOptions.size());
    themeValuePointers.reserve(themeOptions.size());
    themeLabelPointers.reserve(themeOptions.size());
    for (const auto& option : themeOptions) {
        const std::string value = TrimAsciiWhitespace(option.value);
        if (value.empty()) {
            continue;
        }
        themeValues.push_back(value);
        themeLabels.push_back(option.label.empty() ? value : option.label);
    }
    for (size_t index = 0; index < themeValues.size(); ++index) {
        themeValuePointers.push_back(themeValues[index].c_str());
        themeLabelPointers.push_back(themeLabels[index].c_str());
    }

    TrayEffectMenuSection clickSection{};
    TrayEffectMenuSection trailSection{};
    TrayEffectMenuSection scrollSection{};
    TrayEffectMenuSection holdSection{};
    TrayEffectMenuSection hoverSection{};
    clickSection.category = "click";
    trailSection.category = "trail";
    scrollSection.category = "scroll";
    holdSection.category = "hold";
    hoverSection.category = "hover";
    clickSection.title = resolvedMenuText.clickTitle;
    trailSection.title = resolvedMenuText.trailTitle;
    scrollSection.title = resolvedMenuText.scrollTitle;
    holdSection.title = resolvedMenuText.holdTitle;
    hoverSection.title = resolvedMenuText.hoverTitle;

    std::vector<ShellEffectMenuSection> effectSections;
    host->GetEffectMenuSnapshotFromShell(preferZhLabels, &effectSections);
    for (const auto& section : effectSections) {
        const std::string category = TrimAsciiWhitespace(section.category);
        TrayEffectMenuSection* target = FindEffectSectionByCategory(
            category,
            &clickSection,
            &trailSection,
            &scrollSection,
            &holdSection,
            &hoverSection);
        if (target == nullptr) {
            continue;
        }
        if (!section.title.empty()) {
            target->title = section.title;
        }
        target->values.clear();
        target->labels.clear();
        target->selectedValue.clear();
        target->values.reserve(section.items.size());
        target->labels.reserve(section.items.size());
        for (const auto& item : section.items) {
            const std::string value = TrimAsciiWhitespace(item.value);
            if (value.empty()) {
                continue;
            }
            target->values.push_back(value);
            target->labels.push_back(item.label.empty() ? value : item.label);
            if (item.selected) {
                target->selectedValue = value;
            }
        }
    }

    BuildPointerViews(&clickSection);
    BuildPointerViews(&trailSection);
    BuildPointerViews(&scrollSection);
    BuildPointerViews(&holdSection);
    BuildPointerViews(&hoverSection);

    void* menuHandle = mfx_macos_tray_menu_create_v4(
        NormalizeOrFallback(resolvedMenuText.themeTitle, "Theme"),
        NormalizeOrFallback(resolvedMenuText.starProjectTitle, u8"\u2605 Star Project"),
        NormalizeOrFallback(resolvedMenuText.settingsTitle, "Settings"),
        NormalizeOrFallback(resolvedMenuText.reloadConfigTitle, "Reload config"),
        NormalizeOrFallback(resolvedMenuText.exitTitle, "Exit"),
        NormalizeOrFallback(resolvedMenuText.tooltip, "MFCMouseEffect"),
        themeValuePointers.empty() ? nullptr : themeValuePointers.data(),
        themeLabelPointers.empty() ? nullptr : themeLabelPointers.data(),
        static_cast<uint32_t>(themeValuePointers.size()),
        selectedTheme.empty() ? nullptr : selectedTheme.c_str(),
        NormalizeOrFallback(clickSection.title, "Click Effects"),
        clickSection.valuePointers.empty() ? nullptr : clickSection.valuePointers.data(),
        clickSection.labelPointers.empty() ? nullptr : clickSection.labelPointers.data(),
        static_cast<uint32_t>(clickSection.valuePointers.size()),
        clickSection.selectedValue.empty() ? nullptr : clickSection.selectedValue.c_str(),
        NormalizeOrFallback(trailSection.title, "Trail Effects"),
        trailSection.valuePointers.empty() ? nullptr : trailSection.valuePointers.data(),
        trailSection.labelPointers.empty() ? nullptr : trailSection.labelPointers.data(),
        static_cast<uint32_t>(trailSection.valuePointers.size()),
        trailSection.selectedValue.empty() ? nullptr : trailSection.selectedValue.c_str(),
        NormalizeOrFallback(scrollSection.title, "Scroll Effects"),
        scrollSection.valuePointers.empty() ? nullptr : scrollSection.valuePointers.data(),
        scrollSection.labelPointers.empty() ? nullptr : scrollSection.labelPointers.data(),
        static_cast<uint32_t>(scrollSection.valuePointers.size()),
        scrollSection.selectedValue.empty() ? nullptr : scrollSection.selectedValue.c_str(),
        NormalizeOrFallback(holdSection.title, "Hold Effects"),
        holdSection.valuePointers.empty() ? nullptr : holdSection.valuePointers.data(),
        holdSection.labelPointers.empty() ? nullptr : holdSection.labelPointers.data(),
        static_cast<uint32_t>(holdSection.valuePointers.size()),
        holdSection.selectedValue.empty() ? nullptr : holdSection.selectedValue.c_str(),
        NormalizeOrFallback(hoverSection.title, "Hover Effects"),
        hoverSection.valuePointers.empty() ? nullptr : hoverSection.valuePointers.data(),
        hoverSection.labelPointers.empty() ? nullptr : hoverSection.labelPointers.data(),
        static_cast<uint32_t>(hoverSection.valuePointers.size()),
        hoverSection.selectedValue.empty() ? nullptr : hoverSection.selectedValue.c_str(),
        host,
        &OnOpenSettings,
        &OnReloadConfig,
        &OnExit,
        &OnOpenProjectRepository,
        &OnSelectTheme,
        &OnSelectEffect);
    if (menuHandle == nullptr) {
        return false;
    }

    outObjects->nativeHandle = menuHandle;
    mfx_macos_tray_menu_schedule_auto_open_settings_v1(menuHandle);
    return true;
}

void ReleaseMacosTrayMenu(MacosTrayMenuObjects* objects) {
    if (objects == nullptr || objects->nativeHandle == nullptr) {
        return;
    }
    mfx_macos_tray_menu_release_v1(objects->nativeHandle);
    objects->nativeHandle = nullptr;
}

void TerminateMacosTrayApp() {
    mfx_macos_tray_terminate_app_v1();
}
#endif

} // namespace mousefx::macos_tray
