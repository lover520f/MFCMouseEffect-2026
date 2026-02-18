#pragma once

#include <cstdint>

namespace mousefx::wasm {

constexpr uint32_t kPluginApiVersionV1 = 1;

enum class CommandKind : uint16_t {
    SpawnText = 1,
    SpawnImage = 2,
};

#pragma pack(push, 1)

struct ClickInputV1 final {
    int32_t x = 0;
    int32_t y = 0;
    uint8_t button = 0;
    uint8_t reserved0 = 0;
    uint8_t reserved1 = 0;
    uint8_t reserved2 = 0;
    uint64_t eventTickMs = 0;
};

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

#pragma pack(pop)

static_assert(sizeof(ClickInputV1) == 20, "ClickInputV1 layout drifted.");
static_assert(sizeof(CommandHeaderV1) == 4, "CommandHeaderV1 layout drifted.");

} // namespace mousefx::wasm
