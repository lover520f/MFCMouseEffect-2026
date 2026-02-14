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
    void HandleApplySettings(const std::string& jsonCmd);

    AppController* controller_ = nullptr;
};

} // namespace mousefx
