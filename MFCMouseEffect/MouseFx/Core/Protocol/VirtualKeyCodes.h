#pragma once

#include <cstdint>

namespace mousefx::vk {

constexpr uint32_t kBackspace = 0x08;
constexpr uint32_t kTab = 0x09;
constexpr uint32_t kReturn = 0x0D;
constexpr uint32_t kShift = 0x10;
constexpr uint32_t kControl = 0x11;
constexpr uint32_t kMenu = 0x12; // Alt
constexpr uint32_t kPause = 0x13;
constexpr uint32_t kCapital = 0x14; // CapsLock
constexpr uint32_t kEscape = 0x1B;
constexpr uint32_t kSpace = 0x20;
constexpr uint32_t kPrior = 0x21; // PageUp
constexpr uint32_t kNext = 0x22;  // PageDown
constexpr uint32_t kEnd = 0x23;
constexpr uint32_t kHome = 0x24;
constexpr uint32_t kLeft = 0x25;
constexpr uint32_t kUp = 0x26;
constexpr uint32_t kRight = 0x27;
constexpr uint32_t kDown = 0x28;
constexpr uint32_t kSnapshot = 0x2C; // PrintScreen
constexpr uint32_t kInsert = 0x2D;
constexpr uint32_t kDelete = 0x2E;

constexpr uint32_t kNumpad0 = 0x60;
constexpr uint32_t kNumpad9 = 0x69;

constexpr uint32_t kF1 = 0x70;
constexpr uint32_t kF24 = 0x87;

constexpr uint32_t kLShift = 0xA0;
constexpr uint32_t kRShift = 0xA1;
constexpr uint32_t kLControl = 0xA2;
constexpr uint32_t kRControl = 0xA3;
constexpr uint32_t kLMenu = 0xA4; // Left Alt
constexpr uint32_t kRMenu = 0xA5; // Right Alt

constexpr uint32_t kLWin = 0x5B;
constexpr uint32_t kRWin = 0x5C;
constexpr uint32_t kApps = 0x5D;

} // namespace mousefx::vk
