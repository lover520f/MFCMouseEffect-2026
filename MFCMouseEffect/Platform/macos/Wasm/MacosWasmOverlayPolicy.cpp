#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayPolicy.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <string_view>

namespace mousefx::platform::macos {

namespace {

constexpr const char* kEnvMaxInFlight = "MFX_MACOS_WASM_OVERLAY_MAX_INFLIGHT";
constexpr const char* kEnvMinImageIntervalMs = "MFX_MACOS_WASM_IMAGE_MIN_INTERVAL_MS";
constexpr const char* kEnvMinTextIntervalMs = "MFX_MACOS_WASM_TEXT_MIN_INTERVAL_MS";
constexpr const char* kEnvMinIndicatorTextIntervalMs = "MFX_MACOS_WASM_INDICATOR_TEXT_MIN_INTERVAL_MS";
constexpr const char* kEnvMaxTextWindows = "MFX_MACOS_WASM_TEXT_MAX_WINDOWS";
constexpr const char* kEnvMaxIndicatorTextWindows = "MFX_MACOS_WASM_INDICATOR_TEXT_MAX_WINDOWS";

bool TryParseUintEnv(const char* envName, uint32_t* outValue) {
    if (outValue == nullptr) {
        return false;
    }
    const char* raw = std::getenv(envName);
    if (raw == nullptr || *raw == '\0') {
        return false;
    }

    const std::string_view text(raw);
    uint32_t parsed = 0;
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto parseResult = std::from_chars(begin, end, parsed, 10);
    if (parseResult.ec != std::errc() || parseResult.ptr != end) {
        return false;
    }
    *outValue = parsed;
    return true;
}

size_t ClampSize(uint32_t value, size_t lo, size_t hi) {
    const size_t converted = static_cast<size_t>(value);
    return std::max(lo, std::min(converted, hi));
}

uint32_t ClampMs(uint32_t value, uint32_t lo, uint32_t hi) {
    return std::max(lo, std::min(value, hi));
}

} // namespace

const MacosWasmOverlayPolicy& GetMacosWasmOverlayPolicy() {
    static const MacosWasmOverlayPolicy kPolicy = [] {
      MacosWasmOverlayPolicy policy{};

      uint32_t parsed = 0;
      if (TryParseUintEnv(kEnvMaxInFlight, &parsed)) {
          policy.maxInFlightOverlays = ClampSize(parsed, 8u, 512u);
      }
      if (TryParseUintEnv(kEnvMinImageIntervalMs, &parsed)) {
          policy.minImageIntervalMs = ClampMs(parsed, 0u, 1000u);
      }
      if (TryParseUintEnv(kEnvMinTextIntervalMs, &parsed)) {
          policy.minTextIntervalMs = ClampMs(parsed, 0u, 1000u);
      }
      if (TryParseUintEnv(kEnvMinIndicatorTextIntervalMs, &parsed)) {
          policy.minIndicatorTextIntervalMs = ClampMs(parsed, 0u, 1000u);
      }
      if (TryParseUintEnv(kEnvMaxTextWindows, &parsed)) {
          policy.maxTextWindows = ClampSize(parsed, 1u, 48u);
      }
      if (TryParseUintEnv(kEnvMaxIndicatorTextWindows, &parsed)) {
          policy.maxIndicatorTextWindows = ClampSize(parsed, 1u, 8u);
      }

      return policy;
    }();
    return kPolicy;
}

} // namespace mousefx::platform::macos
