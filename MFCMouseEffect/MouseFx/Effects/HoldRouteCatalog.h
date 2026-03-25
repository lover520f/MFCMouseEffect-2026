#pragma once

#include <string>

namespace mousefx::hold_route {

inline constexpr const char kTypeQuantumHaloGpuV2[] = "hold_quantum_halo_gpu_v2";
inline constexpr const char kTypeQuantumHaloGpuV2Legacy[] = "hold_neon3d_gpu_v2";
inline constexpr const char kTypeFluxFieldCpu[] = "hold_fluxfield_cpu";
inline constexpr const char kTypeFluxFieldGpuV2[] = "hold_fluxfield_gpu_v2";
inline constexpr const char kTypeHologram[] = "hologram";
inline constexpr const char kTypeHologramLegacy[] = "scifi3d";
inline constexpr const char kTypeNeon3D[] = "hold_neon3d";
inline constexpr const char kTypeNeon3DLegacy[] = "neon3d";

inline constexpr const char kRouteReasonQuantumHaloGpuV2[] = "quantum_halo_gpu_v2_d3d11_dcomp_direct_runtime_route";
inline constexpr const char kRouteReasonFluxFieldGpuV2[] = "flux_gpu_v2_d3d11_compute_route";
inline constexpr const char kRouteReasonGpuDisabledByBuild[] = "gpu_route_disabled_by_build";

std::string NormalizeHoldEffectTypeAlias(const std::string& type);
std::string NormalizeHoldEffectTypeForCurrentBuild(const std::string& type);
bool IsGpuHoldRouteEnabledForCurrentBuild();
bool IsGpuV2RouteType(const std::string& type);
bool IsQuantumHaloGpuV2Type(const std::string& type);
bool IsFluxFieldGpuV2Type(const std::string& type);
bool IsQuantumHaloGpuV2DirectType(const std::string& type);
const char* RouteReasonForType(const std::string& type);
const char* FallbackReasonForCurrentBuild(const std::string& type);

} // namespace mousefx::hold_route
