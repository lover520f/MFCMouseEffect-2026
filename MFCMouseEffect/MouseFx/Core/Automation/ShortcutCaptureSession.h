#pragma once

#include <windows.h>

#include <cstdint>
#include <mutex>
#include <string>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class ShortcutCaptureSession final {
public:
    enum class PollState : uint8_t {
        InvalidSession = 0,
        Pending = 1,
        Captured = 2,
        Expired = 3,
    };

    struct PollResult {
        PollState state{PollState::InvalidSession};
        std::string shortcut{};
    };

    ShortcutCaptureSession() = default;

    std::string Start(uint64_t timeoutMs);
    bool Stop(const std::string& sessionId);
    PollResult Poll(const std::string& sessionId);
    void OnKeyDown(const KeyEvent& ev);
    bool IsActive() const;

private:
    static uint64_t NowMs();
    static std::string CreateSessionId();
    static bool IsModifierKey(UINT vkCode);
    static std::string KeyTokenFromVk(UINT vkCode);
    static std::string BuildShortcutText(const KeyEvent& ev);

    mutable std::mutex mutex_{};
    std::string sessionId_{};
    uint64_t expireAtMs_{0};
    bool active_{false};
    bool captured_{false};
    std::string capturedShortcut_{};
};

} // namespace mousefx
