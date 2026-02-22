#pragma once

#include <cstdint>
#include <mutex>
#include <string>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/System/IMonotonicClockService.h"

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
    void SetClockService(const IMonotonicClockService* clockService);

private:
    uint64_t NowMs() const;
    static std::string CreateSessionId();
    static bool IsModifierKey(uint32_t vkCode);
    static std::string KeyTokenFromVk(uint32_t vkCode);
    static std::string BuildShortcutText(const KeyEvent& ev);

    mutable std::mutex mutex_{};
    std::string sessionId_{};
    uint64_t expireAtMs_{0};
    bool active_{false};
    bool captured_{false};
    std::string capturedShortcut_{};
    const IMonotonicClockService* clockService_{nullptr};
};

} // namespace mousefx
