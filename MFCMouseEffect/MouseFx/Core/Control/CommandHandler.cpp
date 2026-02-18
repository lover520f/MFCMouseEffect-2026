// CommandHandler.cpp - JSON command routing extracted from AppController

#include "pch.h"
#include "CommandHandler.h"
#include "AppController.h"
#include "MouseFx/Core/Protocol/JsonLite.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

#include <array>

namespace mousefx {

CommandHandler::CommandHandler(AppController* controller)
    : controller_(controller) {}

void CommandHandler::Handle(const std::string& jsonCmd) {
    using CommandHandlerMethod = void (CommandHandler::*)(const std::string&);
    struct CommandRoute {
        const char* command;
        CommandHandlerMethod handler;
    };
    static const std::array<CommandRoute, 12> kCommandRoutes{{
        {"set_effect", &CommandHandler::HandleSetEffectCommand},
        {"clear_effect", &CommandHandler::HandleClearEffectCommand},
        {"set_theme", &CommandHandler::HandleSetThemeCommand},
        {"set_ui_language", &CommandHandler::HandleSetUiLanguageCommand},
        {"effect_cmd", &CommandHandler::HandleEffectCommand},
        {"reload_config", &CommandHandler::HandleReloadConfigCommand},
        {"reset_config", &CommandHandler::HandleResetConfigCommand},
        {"apply_settings", &CommandHandler::HandleApplySettings},
        {"wasm_enable", &CommandHandler::HandleWasmEnableCommand},
        {"wasm_disable", &CommandHandler::HandleWasmDisableCommand},
        {"wasm_reload", &CommandHandler::HandleWasmReloadCommand},
        {"wasm_load_manifest", &CommandHandler::HandleWasmLoadManifestCommand},
    }};

    const std::string cmd = ExtractJsonStringValue(jsonCmd, "cmd");
    for (const auto& route : kCommandRoutes) {
        if (cmd != route.command) {
            continue;
        }
        (this->*route.handler)(jsonCmd);
        return;
    }
}

void CommandHandler::HandleSetEffectCommand(const std::string& jsonCmd) {
    std::string category = ExtractJsonStringValue(jsonCmd, "category");
    std::string type = ExtractJsonStringValue(jsonCmd, "type");

    if (category.empty()) {
        // Legacy format: {"cmd": "set_effect", "type": "ripple"}
        std::string reason;
        const std::string effectiveType = controller_->ResolveRuntimeEffectType(EffectCategory::Click, type, &reason);
        controller_->SetEffect(EffectCategory::Click, type);
        controller_->SetActiveEffectType(EffectCategory::Click, effectiveType);
    } else {
        const auto cat = CategoryFromString(category);
        std::string reason;
        const std::string effectiveType = controller_->ResolveRuntimeEffectType(cat, type, &reason);
        controller_->SetEffect(cat, type);
        controller_->SetActiveEffectType(cat, effectiveType);
    }
    controller_->PersistConfig();
}

void CommandHandler::HandleClearEffectCommand(const std::string& jsonCmd) {
    std::string category = ExtractJsonStringValue(jsonCmd, "category");
    const auto cat = CategoryFromString(category);
    controller_->ClearEffect(cat);
    controller_->SetActiveEffectType(cat, "none");
    controller_->PersistConfig();
}

void CommandHandler::HandleSetThemeCommand(const std::string& jsonCmd) {
    const std::string theme = ExtractJsonStringValue(jsonCmd, "theme");
    controller_->SetTheme(theme);
}

void CommandHandler::HandleSetUiLanguageCommand(const std::string& jsonCmd) {
    const std::string lang = ExtractJsonStringValue(jsonCmd, "lang");
    controller_->SetUiLanguage(lang);
}

void CommandHandler::HandleEffectCommand(const std::string& jsonCmd) {
    std::string category = ExtractJsonStringValue(jsonCmd, "category");
    std::string command = ExtractJsonStringValue(jsonCmd, "command");
    std::string args = ExtractJsonStringValue(jsonCmd, "args");

    if (category.empty()) {
        return;
    }

    const auto cat = CategoryFromString(category);
    if (auto* effect = controller_->GetEffect(cat)) {
        effect->OnCommand(command, args);
    }
}

void CommandHandler::HandleReloadConfigCommand(const std::string&) {
    controller_->ReloadConfigFromDisk();
}

void CommandHandler::HandleResetConfigCommand(const std::string&) {
    controller_->ResetConfig();
}

void CommandHandler::HandleWasmEnableCommand(const std::string&) {
    if (auto* host = controller_->WasmHost()) {
        host->SetEnabled(true);
    }
}

void CommandHandler::HandleWasmDisableCommand(const std::string&) {
    if (auto* host = controller_->WasmHost()) {
        host->SetEnabled(false);
    }
}

void CommandHandler::HandleWasmReloadCommand(const std::string&) {
    if (auto* host = controller_->WasmHost()) {
        host->ReloadPlugin();
    }
}

void CommandHandler::HandleWasmLoadManifestCommand(const std::string& jsonCmd) {
    if (auto* host = controller_->WasmHost()) {
        const std::string pathUtf8 = ExtractJsonStringValue(jsonCmd, "manifest_path");
        if (pathUtf8.empty()) {
            return;
        }
        host->LoadPluginFromManifest(Utf8ToWString(pathUtf8));
    }
}

} // namespace mousefx
