#pragma once

#include <string>

namespace mousefx {

class AppController;

// Routes JSON command strings to the appropriate AppController methods.
// Extracted from AppController::HandleCommand to reduce AppController.cpp size.
class CommandHandler final {
public:
    explicit CommandHandler(AppController* controller);

    // Parse and execute a JSON command string.
    void Handle(const std::string& jsonCmd);

private:
    void HandleSetEffectCommand(const std::string& jsonCmd);
    void HandleClearEffectCommand(const std::string& jsonCmd);
    void HandleSetThemeCommand(const std::string& jsonCmd);
    void HandleSetUiLanguageCommand(const std::string& jsonCmd);
    void HandleEffectCommand(const std::string& jsonCmd);
    void HandleReloadConfigCommand(const std::string& jsonCmd);
    void HandleResetConfigCommand(const std::string& jsonCmd);
    void HandleApplySettings(const std::string& jsonCmd);
    void HandleWasmEnableCommand(const std::string& jsonCmd);
    void HandleWasmDisableCommand(const std::string& jsonCmd);
    void HandleWasmReloadCommand(const std::string& jsonCmd);
    void HandleWasmLoadManifestCommand(const std::string& jsonCmd);

    AppController* controller_ = nullptr;
};

} // namespace mousefx
