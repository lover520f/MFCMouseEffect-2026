#include "pch.h"
#include "ShortcutCaptureSession.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

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
    case vk::kControl:
    case vk::kLControl:
    case vk::kRControl:
    case vk::kShift:
    case vk::kLShift:
    case vk::kRShift:
    case vk::kMenu:
    case vk::kLMenu:
    case vk::kRMenu:
    case vk::kLWin:
    case vk::kRWin:
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
    if (vkCode >= vk::kF1 && vkCode <= vk::kF24) {
        const int index = static_cast<int>(vkCode - vk::kF1 + 1);
        return "F" + std::to_string(index);
    }
    if (vkCode >= vk::kNumpad0 && vkCode <= vk::kNumpad9) {
        const int index = static_cast<int>(vkCode - vk::kNumpad0);
        return std::to_string(index);
    }

    switch (vkCode) {
    case vk::kTab: return "Tab";
    case vk::kReturn: return "Enter";
    case vk::kEscape: return "Esc";
    case vk::kSpace: return "Space";
    case vk::kBackspace: return "Backspace";
    case vk::kDelete: return "Delete";
    case vk::kInsert: return "Insert";
    case vk::kHome: return "Home";
    case vk::kEnd: return "End";
    case vk::kPrior: return "PageUp";
    case vk::kNext: return "PageDown";
    case vk::kUp: return "Up";
    case vk::kDown: return "Down";
    case vk::kLeft: return "Left";
    case vk::kRight: return "Right";
    case vk::kCapital: return "CapsLock";
    case vk::kSnapshot: return "PrintScreen";
    case vk::kPause: return "Pause";
    case vk::kApps: return "Apps";
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
