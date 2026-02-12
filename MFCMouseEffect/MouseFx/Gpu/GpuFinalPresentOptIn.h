#pragma once

namespace mousefx::gpu {

// Default-on final-present opt-in with explicit local kill-switch support.
// Disable by creating ".local/diag/gpu_final_present.off" next to the exe.
bool IsGpuFinalPresentOptInEnabled();

} // namespace mousefx::gpu
