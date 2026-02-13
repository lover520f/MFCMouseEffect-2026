# Stage 42 - Quantum Halo GPU Driver SEH Guard (nvwgf2umx Access Violation)

## Problem
- During `Quantum Halo GPU` runtime, some machines hit an access violation inside NVIDIA user-mode driver:
  - `nvwgf2umx.dll`
  - `0xC0000005` read at low/null-adjacent address
- Crash point appeared around presenter GPU submit path (`pipeline_.Render` / `Present`).

## Root cause
- This is a driver-layer fault surface (outside app-owned memory safety checks).
- Without structured exception handling around GPU submit calls, process-level crash can occur before fallback logic executes.

## Fix
1. Added SEH guard in `QuantumHaloGpuV2Presenter::RenderFrame` around:
   - `pipeline_.Render(...)`
   - `swapChain_->Present(...)`
   - `dcompDevice_->Commit()`
2. On SEH exception:
   - set error reason `gpu_driver_seh_<exception_code>`
   - mark presenter failure
   - return `false` to let upper runtime route fallback/degrade safely

## Files changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2Presenter.h`

## Validation
1. Run `hold_quantum_halo_gpu_v2` in Debug/Release.
2. If driver faults in GPU submit path, app should not hard-crash immediately.
3. Runtime should report non-`ok` reason in diagnostics and enter failure handling path.
