// CommandHandler.cpp - JSON command routing extracted from AppController

#include "pch.h"
#include "CommandHandler.h"
#include "AppController.h"
#include "MouseFx/Core/Protocol/JsonLite.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/StringUtils.h"

#include <array>
#include <limits>

namespace mousefx {

CommandHandler::CommandHandler(AppController* controller)
    : controller_(controller) {}

void CommandHandler::Handle(const std::string& jsonCmd) {
    using CommandHandlerMethod = void (CommandHandler::*)(const std::string&);
    struct CommandRoute {
        const char* command;
        CommandHandlerMethod handler;
    };
    static const std::array<CommandRoute, 13> kCommandRoutes{{
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
        {"wasm_set_policy", &CommandHandler::HandleWasmSetPolicyCommand},
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
    controller_->SetWasmEnabled(true);
}

void CommandHandler::HandleWasmDisableCommand(const std::string&) {
    controller_->SetWasmEnabled(false);
}

void CommandHandler::HandleWasmReloadCommand(const std::string&) {
    if (auto* host = controller_->WasmHost()) {
        host->ReloadPlugin();
    }
}

void CommandHandler::HandleWasmLoadManifestCommand(const std::string& jsonCmd) {
    if (!controller_->WasmHost()) {
        return;
    }
    std::string pathUtf8;
    try {
        const nlohmann::json root = nlohmann::json::parse(jsonCmd);
        if (root.contains("manifest_path") && root["manifest_path"].is_string()) {
            pathUtf8 = root["manifest_path"].get<std::string>();
        }
    } catch (...) {
        pathUtf8 = ExtractJsonStringValue(jsonCmd, "manifest_path");
    }
    if (pathUtf8.empty()) {
        return;
    }
    controller_->LoadWasmPluginFromManifestPath(pathUtf8);
}

void CommandHandler::HandleWasmSetPolicyCommand(const std::string& jsonCmd) {
    nlohmann::json root;
    try {
        root = nlohmann::json::parse(jsonCmd);
    } catch (...) {
        return;
    }

    if (root.contains("enabled") && root["enabled"].is_boolean()) {
        controller_->SetWasmEnabled(root["enabled"].get<bool>());
    }
    if (root.contains("manifest_path") && root["manifest_path"].is_string()) {
        controller_->SetWasmManifestPath(root["manifest_path"].get<std::string>());
    }
    if (root.contains("catalog_root_path") && root["catalog_root_path"].is_string()) {
        controller_->SetWasmCatalogRootPath(root["catalog_root_path"].get<std::string>());
    }
    if (root.contains("fallback_to_builtin_click") && root["fallback_to_builtin_click"].is_boolean()) {
        controller_->SetWasmFallbackToBuiltinClick(root["fallback_to_builtin_click"].get<bool>());
    }

    bool hasOutputBudget = false;
    bool hasCommandBudget = false;
    bool hasExecutionBudget = false;
    uint32_t outputBudgetBytes = controller_->Config().wasm.outputBufferBytes;
    uint32_t maxCommands = controller_->Config().wasm.maxCommands;
    double maxExecutionMs = controller_->Config().wasm.maxEventExecutionMs;

    if (root.contains("output_buffer_bytes") && root["output_buffer_bytes"].is_number_integer()) {
        const int64_t raw = root["output_buffer_bytes"].get<int64_t>();
        outputBudgetBytes = (raw <= 0)
            ? 0u
            : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
        hasOutputBudget = true;
    }
    if (root.contains("max_commands") && root["max_commands"].is_number_integer()) {
        const int64_t raw = root["max_commands"].get<int64_t>();
        maxCommands = (raw <= 0)
            ? 0u
            : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
        hasCommandBudget = true;
    }
    if (root.contains("max_execution_ms") && root["max_execution_ms"].is_number()) {
        maxExecutionMs = root["max_execution_ms"].get<double>();
        hasExecutionBudget = true;
    }
    if (hasOutputBudget || hasCommandBudget || hasExecutionBudget) {
        controller_->SetWasmExecutionBudget(outputBudgetBytes, maxCommands, maxExecutionMs);
    }
}

} // namespace mousefx

