#include "pch.h"

// This translation unit exists to force static hold-renderer registration
// for Windows-specific hold routes.

#if defined(MFX_ENABLE_WINDOWS_GPU_EFFECTS) && MFX_ENABLE_WINDOWS_GPU_EFFECTS
#include "Platform/windows/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h"
#include "Platform/windows/Renderers/Hold/FluxFieldHudGpuV2Renderer.h"
#endif
