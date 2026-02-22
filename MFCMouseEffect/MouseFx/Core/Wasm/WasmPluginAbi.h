#pragma once

#include <cstdint>

namespace mousefx::wasm {

constexpr uint32_t kPluginApiVersionV1 = 1;

enum class CommandKind : uint16_t {
    SpawnText = 1,
    SpawnImage = 2,
    SpawnImageAffine = 3,
};

// Unified input event kinds for mfx_plugin_on_event entry.
enum class InputEventKindV1 : uint8_t {
    Click = 1,
    Move = 2,
    Scroll = 3,
    HoldStart = 4,
    HoldUpdate = 5,
    HoldEnd = 6,
    HoverStart = 7,
    HoverEnd = 8,
};

constexpr uint8_t kEventFlagScrollHorizontal = 0x01u;

#pragma pack(push, 1)

struct CommandHeaderV1 final {
    uint16_t kind = 0;
    uint16_t sizeBytes = 0;
};

struct SpawnTextCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    float alpha = 1.0f;
    uint32_t colorRgba = 0xFFFFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint32_t textId = 0;
};

struct SpawnImageCommandV1 final {
    CommandHeaderV1 header{};
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    float alpha = 1.0f;
    uint32_t tintRgba = 0xFFFFFFFFu;
    uint32_t delayMs = 0;
    uint32_t lifeMs = 0;
    uint32_t imageId = 0;
};

// Generic event input for mfx_plugin_on_event.
// - For Click: button is valid; delta/holdMs/flags are optional.
// - For Move: x/y are valid.
// - For Scroll: delta + flags(horizontal bit) are valid.
// - For Hold*: holdMs + button + x/y are valid.
// - For Hover*: x/y are valid.
struct EventInputV1 final {
    int32_t x = 0;
    int32_t y = 0;
    int32_t delta = 0;
    uint32_t holdMs = 0;
    uint8_t kind = static_cast<uint8_t>(InputEventKindV1::Click);
    uint8_t button = 0;
    uint8_t flags = 0;
    uint8_t reserved0 = 0;
    uint64_t eventTickMs = 0;
};

// Forward-compatible image command variant that keeps SpawnImageCommandV1 as prefix
// and appends affine transform metadata.
struct SpawnImageAffineCommandV1 final {
    SpawnImageCommandV1 base{};
    float affineM11 = 1.0f;
    float affineM12 = 0.0f;
    float affineM21 = 0.0f;
    float affineM22 = 1.0f;
    float affineDx = 0.0f;
    float affineDy = 0.0f;
    uint32_t affineAnchorMode = 0;
    uint32_t affineEnabled = 0;
};

#pragma pack(pop)

static_assert(sizeof(EventInputV1) == 28, "EventInputV1 layout drifted.");
static_assert(sizeof(CommandHeaderV1) == 4, "CommandHeaderV1 layout drifted.");
static_assert(sizeof(SpawnImageAffineCommandV1) == 88, "SpawnImageAffineCommandV1 layout drifted.");

} // namespace mousefx::wasm
