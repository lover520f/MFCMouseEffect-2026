#include "pch.h"
#include "HoldRouteCatalog.h"

namespace mousefx::hold_route {

std::string NormalizeHoldEffectTypeAlias(const std::string& type) {
    if (type == kTypeQuantumHaloGpuV2Legacy) {
        return kTypeQuantumHaloGpuV2;
    }
    return type;
}

bool IsGpuV2RouteType(const std::string& type) {
    return type.find("_gpu_v2") != std::string::npos;
}

bool IsQuantumHaloGpuV2Type(const std::string& type) {
    const std::string normalized = NormalizeHoldEffectTypeAlias(type);
    return normalized == kTypeQuantumHaloGpuV2;
}

bool IsFluxFieldGpuV2Type(const std::string& type) {
    const std::string normalized = NormalizeHoldEffectTypeAlias(type);
    return normalized == kTypeFluxFieldGpuV2;
}

bool IsQuantumHaloGpuV2DirectType(const std::string& type) {
    return IsQuantumHaloGpuV2Type(type);
}

const char* RouteReasonForType(const std::string& type) {
    const std::string normalized = NormalizeHoldEffectTypeAlias(type);
    if (normalized == kTypeQuantumHaloGpuV2) {
        return kRouteReasonQuantumHaloGpuV2;
    }
    if (normalized == kTypeFluxFieldGpuV2) {
        return kRouteReasonFluxFieldGpuV2;
    }
    return "";
}

} // namespace mousefx::hold_route
