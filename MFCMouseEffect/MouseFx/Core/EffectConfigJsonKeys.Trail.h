#pragma once

namespace mousefx::config_json::keys::profile {

inline constexpr const char kLine[] = "line";
inline constexpr const char kStreamer[] = "streamer";
inline constexpr const char kElectric[] = "electric";
inline constexpr const char kMeteor[] = "meteor";
inline constexpr const char kTubes[] = "tubes";
inline constexpr const char kDurationMs[] = "duration_ms";
inline constexpr const char kMaxPoints[] = "max_points";

} // namespace mousefx::config_json::keys::profile

namespace mousefx::config_json::keys::trail_params {

inline constexpr const char kStreamer[] = "streamer";
inline constexpr const char kElectric[] = "electric";
inline constexpr const char kMeteor[] = "meteor";
inline constexpr const char kIdleFadeStartMs[] = "idle_fade_start_ms";
inline constexpr const char kIdleFadeEndMs[] = "idle_fade_end_ms";

namespace streamer {
inline constexpr const char kGlowWidthScale[] = "glow_width_scale";
inline constexpr const char kCoreWidthScale[] = "core_width_scale";
inline constexpr const char kHeadPower[] = "head_power";
} // namespace streamer

namespace electric {
inline constexpr const char kAmplitudeScale[] = "amplitude_scale";
inline constexpr const char kForkChance[] = "fork_chance";
} // namespace electric

namespace meteor {
inline constexpr const char kSparkRateScale[] = "spark_rate_scale";
inline constexpr const char kSparkSpeedScale[] = "spark_speed_scale";
} // namespace meteor

} // namespace mousefx::config_json::keys::trail_params
