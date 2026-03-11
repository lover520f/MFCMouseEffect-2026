#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mousefx::wasm {

inline constexpr uint32_t kManifestInputKindClickBit = 1u << 0;
inline constexpr uint32_t kManifestInputKindMoveBit = 1u << 1;
inline constexpr uint32_t kManifestInputKindScrollBit = 1u << 2;
inline constexpr uint32_t kManifestInputKindHoldStartBit = 1u << 3;
inline constexpr uint32_t kManifestInputKindHoldUpdateBit = 1u << 4;
inline constexpr uint32_t kManifestInputKindHoldEndBit = 1u << 5;
inline constexpr uint32_t kManifestInputKindHoverStartBit = 1u << 6;
inline constexpr uint32_t kManifestInputKindHoverEndBit = 1u << 7;
inline constexpr uint32_t kManifestInputKindIndicatorClickBit = 1u << 8;
inline constexpr uint32_t kManifestInputKindIndicatorScrollBit = 1u << 9;
inline constexpr uint32_t kManifestInputKindIndicatorKeyBit = 1u << 10;
inline constexpr uint32_t kManifestInputKindAllBits =
    kManifestInputKindClickBit |
    kManifestInputKindMoveBit |
    kManifestInputKindScrollBit |
    kManifestInputKindHoldStartBit |
    kManifestInputKindHoldUpdateBit |
    kManifestInputKindHoldEndBit |
    kManifestInputKindHoverStartBit |
    kManifestInputKindHoverEndBit |
    kManifestInputKindIndicatorClickBit |
    kManifestInputKindIndicatorScrollBit |
    kManifestInputKindIndicatorKeyBit;

inline constexpr uint32_t kManifestSurfaceEffectsBit = 1u << 0;
inline constexpr uint32_t kManifestSurfaceIndicatorBit = 1u << 1;
inline constexpr uint32_t kManifestSurfaceAllBits =
    kManifestSurfaceEffectsBit |
    kManifestSurfaceIndicatorBit;

struct PluginManifest final {
    std::string id{};
    std::string name{};
    std::string version{};
    uint32_t apiVersion = 0;
    std::wstring entryWasm{};
    std::vector<std::wstring> imageAssets{};
    uint32_t inputKindsMask = kManifestInputKindAllBits;
    uint32_t surfaceKindsMask = kManifestSurfaceAllBits;
    bool hasExplicitSurfaceKinds = false;
    bool enableFrameTick = true;
};

struct PluginManifestLoadResult final {
    bool ok = false;
    PluginManifest manifest{};
    std::string error{};
};

class WasmPluginManifest final {
public:
    static PluginManifestLoadResult LoadFromFile(const std::wstring& manifestPath);
    static bool Validate(const PluginManifest& manifest, std::string* outError);
};

} // namespace mousefx::wasm
