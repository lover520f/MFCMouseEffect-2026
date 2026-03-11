#include "pch.h"
#include "ShortcutCaptureSession.h"

#include "MouseFx/Core/Automation/ShortcutTextFormatter.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <random>

namespace mousefx {
namespace {

constexpr uint64_t kDefaultTimeoutMs = 10000;
constexpr uint64_t kMinTimeoutMs = 1000;
constexpr uint64_t kMaxTimeoutMs = 30000;
constexpr uint64_t kModifierOnlyCaptureGraceMs = 240;

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
    pendingModifierAtMs_ = 0;
    pendingModifierShortcut_.clear();
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
    pendingModifierAtMs_ = 0;
    pendingModifierShortcut_.clear();
    return true;
}

ShortcutCaptureSession::PollResult ShortcutCaptureSession::Poll(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionId.empty() || sessionId != sessionId_) {
        return {};
    }

    const uint64_t now = NowMs();
    if (!captured_ &&
        !pendingModifierShortcut_.empty() &&
        pendingModifierAtMs_ > 0 &&
        (now - pendingModifierAtMs_ >= kModifierOnlyCaptureGraceMs)) {
        capturedShortcut_ = pendingModifierShortcut_;
        captured_ = true;
        pendingModifierAtMs_ = 0;
        pendingModifierShortcut_.clear();
    }

    if (captured_) {
        PollResult out{};
        out.state = PollState::Captured;
        out.shortcut = capturedShortcut_;
        sessionId_.clear();
        expireAtMs_ = 0;
        active_ = false;
        captured_ = false;
        capturedShortcut_.clear();
        pendingModifierAtMs_ = 0;
        pendingModifierShortcut_.clear();
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
        pendingModifierAtMs_ = 0;
        pendingModifierShortcut_.clear();
        return;
    }

    const std::string shortcut = shortcut_text::FormatShortcutText(ev);
    if (shortcut.empty()) {
        return;
    }

    if (shortcut_text::IsModifierVirtualKey(ev.vkCode)) {
        pendingModifierAtMs_ = NowMs();
        pendingModifierShortcut_ = shortcut;
        return;
    }

    capturedShortcut_ = shortcut;
    captured_ = true;
    pendingModifierAtMs_ = 0;
    pendingModifierShortcut_.clear();
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
