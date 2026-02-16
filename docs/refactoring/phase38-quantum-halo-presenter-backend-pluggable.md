# Refactoring Phase 38: Quantum Halo GPU Presentation Layer Pluggability

## Background
- The GPU compute path and GPU presentation path were already separated, but presentation remained hard-wired to `QuantumHaloGpuV2Presenter`.
- Two call sites (`HoldQuantumHaloGpuV2DirectRuntime` and `HoldQuantumHaloGpuV2Renderer`) depended on one concrete presenter class.
- This coupling made backend replacement costly (new API backend would require touching multiple business files).

## Goal
- Make the Quantum Halo GPU presentation layer plug-and-play.
- Keep current behavior unchanged.
- Reduce long-term change cost when adding/replacing presenter backends.

## Design Decision
- Introduce a dedicated `Presentation` subdomain under `MouseFx/Renderers/Hold`.
- Use `Strategy + Registry + Host(Facade)`:
  - `IQuantumHaloPresenterBackend`: stable backend contract.
  - `QuantumHaloPresenterBackendRegistry`: backend registration and priority-based discovery.
  - `QuantumHaloPresenterHost`: lifecycle + backend selection + failover orchestrator.
  - `QuantumHaloDCompPresenterBackend`: adapter for current `QuantumHaloGpuV2Presenter`.

## Implementation

### 1) New backend contract
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/IQuantumHaloPresenterBackend.h`
- Defines frame input (`QuantumHaloPresenterFrameArgs`) and unified lifecycle/render APIs.

### 2) New pluggable backend registry
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.cpp`
- Supports:
  - backend registration with priority
  - backend creation by name
  - ordered backend enumeration for host selection

### 3) New host orchestration layer
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterHost.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterHost.cpp`
- Responsibilities:
  - lazy backend startup
  - ordered backend probing
  - render-time backend failure switch
  - unified error reporting to callers

### 4) Current presenter adapted as default backend
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.cpp`
- Registers current D3D11 + DComp presenter as backend id `dcomp_d3d11` with highest priority.

### 5) Caller decoupling
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.cpp`
  - Replace direct dependency on `QuantumHaloGpuV2Presenter` with `QuantumHaloPresenterHost`.
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
  - Replace presenter member type with `QuantumHaloPresenterHost`.
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.h`
  - Remove obsolete forward declarations.

### 6) Build integration
- `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - Add new `Presentation` headers and cpp files.

## Why This Is Better Long-Term
- Presentation backend expansion no longer requires editing runtime/renderer business flow.
- Failure handling is centralized at host layer, not duplicated in each caller.
- Supports future backends (e.g., alternate swapchain path, Vulkan/Dawn route) through registration only.
- Keeps compute/presentation boundaries cleaner and easier to reason about.

## Compatibility
- External effect IDs and runtime behavior remain unchanged.
- Existing D3D11 + DComp path is still the default implementation.

## Validation
- Build target: `Debug|x64`.
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug;Platform=x64 /m`
