#include "pch.h"
#include "ShortcutCaptureSession.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
#include <windows.h>

namespace mousefx {
namespace {

constexpr uint64_t kDefaultTimeoutMs = 10000;
constexpr uint64_t kMinTimeoutMs = 1000;
constexpr uint64_t kMaxTimeoutMs = 30000;

std::string JoinTokens(const std::vector<std::string>& tokens) {
    std::string out;
    for (const std::string& token : tokens) {
        if (token.empty()) {
            continue;
        }
        if (!out.empty()) {
            out.push_back('+');
        }
        out += token;
    }
    return out;
}

} // namespace

uint64_t ShortcutCaptureSession::NowMs() const {
    if (clockService_) {
        return clockService_->NowMs();
    }
    using namespace std::chrono;
    const auto now = steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(duration_cast<milliseconds>(now).count());
}

std::string ShortcutCaptureSession::CreateSessionId() {
    std::array<unsigned long long, 2> parts{};
    std::random_device rd;
    std::mt19937_64 rng(rd());
    parts[0] = rng();
    parts[1] = rng();

    char buf[64]{};
    std::snprintf(buf, sizeof(buf), "%016llx%016llx", parts[0], parts[1]);
    return std::string(buf);
}

bool ShortcutCaptureSession::IsModifierKey(uint32_t vkCode) {
    switch (vkCode) {
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
        return true;
    default:
        return false;
    }
}

std::string ShortcutCaptureSession::KeyTokenFromVk(uint32_t vkCode) {
    if (vkCode >= 'A' && vkCode <= 'Z') {
        return std::string(1, static_cast<char>(vkCode));
    }
    if (vkCode >= '0' && vkCode <= '9') {
        return std::string(1, static_cast<char>(vkCode));
    }
    if (vkCode >= VK_F1 && vkCode <= VK_F24) {
        const int index = static_cast<int>(vkCode - VK_F1 + 1);
        return "F" + std::to_string(index);
    }
    if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) {
        const int index = static_cast<int>(vkCode - VK_NUMPAD0);
        return std::to_string(index);
    }

    switch (vkCode) {
    case VK_TAB: return "Tab";
    case VK_RETURN: return "Enter";
    case VK_ESCAPE: return "Esc";
    case VK_SPACE: return "Space";
    case VK_BACK: return "Backspace";
    case VK_DELETE: return "Delete";
    case VK_INSERT: return "Insert";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "PageUp";
    case VK_NEXT: return "PageDown";
    case VK_UP: return "Up";
    case VK_DOWN: return "Down";
    case VK_LEFT: return "Left";
    case VK_RIGHT: return "Right";
    case VK_CAPITAL: return "CapsLock";
    case VK_SNAPSHOT: return "PrintScreen";
    case VK_PAUSE: return "Pause";
    case VK_APPS: return "Apps";
    default:
        break;
    }

    return {};
}

std::string ShortcutCaptureSession::BuildShortcutText(const KeyEvent& ev) {
    if (IsModifierKey(ev.vkCode)) {
        return {};
    }

    const std::string keyToken = KeyTokenFromVk(ev.vkCode);
    if (keyToken.empty()) {
        return {};
    }

    std::vector<std::string> tokens;
    if (ev.ctrl) tokens.emplace_back("Ctrl");
    if (ev.shift) tokens.emplace_back("Shift");
    if (ev.alt) tokens.emplace_back("Alt");
    if (ev.win) tokens.emplace_back("Win");
    tokens.push_back(keyToken);
    return JoinTokens(tokens);
}

std::string ShortcutCaptureSession::Start(uint64_t timeoutMs) {
    const uint64_t boundedTimeout = std::clamp(
        timeoutMs == 0 ? kDefaultTimeoutMs : timeoutMs,
        kMinTimeoutMs,
        kMaxTimeoutMs);

    std::lock_guard<std::mutex> lock(mutex_);
    sessionId_ = CreateSessionId();
    active_ = true;
    captured_ = false;
    capturedShortcut_.clear();
    expireAtMs_ = NowMs() + boundedTimeout;
    return sessionId_;
}

bool ShortcutCaptureSession::Stop(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionId.empty() || sessionId != sessionId_) {
        return false;
    }

    sessionId_.clear();
    expireAtMs_ = 0;
    active_ = false;
    captured_ = false;
    capturedShortcut_.clear();
    return true;
}

ShortcutCaptureSession::PollResult ShortcutCaptureSession::Poll(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionId.empty() || sessionId != sessionId_) {
        return {};
    }

    const uint64_t now = NowMs();
    if (captured_) {
        PollResult out{};
        out.state = PollState::Captured;
        out.shortcut = capturedShortcut_;
        sessionId_.clear();
        expireAtMs_ = 0;
        active_ = false;
        captured_ = false;
        capturedShortcut_.clear();
        return out;
    }

    if (now >= expireAtMs_) {
        sessionId_.clear();
        expireAtMs_ = 0;
        active_ = false;
        captured_ = false;
        capturedShortcut_.clear();
        return {PollState::Expired, {}};
    }

    return {PollState::Pending, {}};
}

void ShortcutCaptureSession::OnKeyDown(const KeyEvent& ev) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!active_ || captured_) {
        return;
    }

    if (NowMs() >= expireAtMs_) {
        active_ = false;
        captured_ = false;
        return;
    }

    const std::string shortcut = BuildShortcutText(ev);
    if (shortcut.empty()) {
        return;
    }

    capturedShortcut_ = shortcut;
    captured_ = true;
}

bool ShortcutCaptureSession::IsActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

void ShortcutCaptureSession::SetClockService(const IMonotonicClockService* clockService) {
    std::lock_guard<std::mutex> lock(mutex_);
    clockService_ = clockService;
}

} // namespace mousefx
