# Refactoring Phase 39: Hold GPU Route Catalog + Presenter Policy + Observability

## Background
- Hold GPU route ids and alias normalization logic were repeated in multiple call sites.
- Quantum Halo presenter backend became pluggable in phase 38, but runtime diagnostics were still coarse.
- There was no explicit settings entry for selecting presenter backend strategy.

## Goals
- Centralize hold GPU route metadata and routing reason mapping.
- Add end-to-end presenter backend strategy configuration entry.
- Improve Quantum Halo presenter observability for runtime troubleshooting.

## Changes

### 1) Hold route catalog centralization
- Added:
  - `MFCMouseEffect/MouseFx/Effects/HoldRouteCatalog.h`
  - `MFCMouseEffect/MouseFx/Effects/HoldRouteCatalog.cpp`
- Centralized:
  - route ids (`hold_quantum_halo_gpu_v2`, legacy alias, `hold_fluxfield_gpu_v2`)
  - alias normalization
  - GPU-v2 and direct-runtime type checks
  - per-route reason mapping
- Replaced scattered string checks in:
  - `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
  - `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
  - `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
  - `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
  - `MFCMouseEffect/Settings/SettingsOptions.h`

### 2) Presenter backend strategy selection
- Added:
  - `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h`
  - `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.cpp`
- Supports two sources:
  - config preference (`hold_presenter_backend`)
  - environment override (`MFX_QUANTUM_HALO_PRESENTER_BACKEND`)
- Host startup now prioritizes preferred backend before normal priority order.

### 3) Config / command / web settings wiring
- Added new root config field:
  - `hold_presenter_backend` (default: `auto`)
- Updated:
  - config model + normalization
  - parse + serialize codec
  - apply-settings command handling
  - app controller setter/reapply path
  - settings schema + settings state output
  - Web UI form (General card) for backend selection

### 4) Presenter observability enhancements
- `QuantumHaloPresenterHost` now exposes:
  - preferred backend name
  - active backend name
  - last backend failure reason
  - backend switch count
- Extended diagnostics in:
  - `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.cpp`
  - `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
- Snapshot now includes presenter policy and failover signals for faster production debugging.

## Build Integration
- Updated:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Validation
- Build target: `Debug|x64`
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug;Platform=x64 /m`
- Result:
  - success, 0 warning, 0 error

