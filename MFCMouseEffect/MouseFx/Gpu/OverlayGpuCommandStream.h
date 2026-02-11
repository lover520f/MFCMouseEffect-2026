#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mousefx::gpu {

enum class OverlayGpuCommandType : uint8_t {
    TrailPolyline = 0,
    RipplePulse = 1,
    ParticleSprites = 2,
};

namespace OverlayGpuCommandFlags {
constexpr uint32_t kContinuous = 1u << 0;
constexpr uint32_t kLoop = 1u << 1;
constexpr uint32_t kHoverContinuous = 1u << 2;
constexpr uint32_t kHoldContinuous = 1u << 3;
constexpr uint32_t kTrailChromatic = 1u << 4;
constexpr uint32_t kParticleChromatic = 1u << 5;
} // namespace OverlayGpuCommandFlags

struct OverlayGpuVertex {
    float x = 0.0f;
    float y = 0.0f;
    float size = 0.0f;
    float extra = 0.0f;
    uint32_t colorArgb = 0;
};

struct OverlayGpuCommand {
    OverlayGpuCommandType type = OverlayGpuCommandType::TrailPolyline;
    std::string effectTag{};
    uint64_t effectInstanceId = 0;
    uint32_t flags = 0;
    float param0 = 0.0f;
    float param1 = 0.0f;
    float param2 = 0.0f;
    float param3 = 0.0f;
    std::vector<OverlayGpuVertex> vertices{};
};

class OverlayGpuCommandStream {
public:
    void Reset(uint64_t frameTickMs) {
        frameTickMs_ = frameTickMs;
        commands_.clear();
    }

    void Reserve(size_t commandCountHint) {
        if (commandCountHint > commands_.capacity()) {
            commands_.reserve(commandCountHint);
        }
    }

    void Add(OverlayGpuCommand cmd) {
        commands_.push_back(std::move(cmd));
    }

    uint64_t FrameTickMs() const { return frameTickMs_; }
    const std::vector<OverlayGpuCommand>& Commands() const { return commands_; }
    bool Empty() const { return commands_.empty(); }

private:
    uint64_t frameTickMs_ = 0;
    std::vector<OverlayGpuCommand> commands_{};
};

} // namespace mousefx::gpu
