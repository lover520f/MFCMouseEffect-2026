# Dawn Backend + CPU Fallback (Stage 33 Runtime Folder Fallback Loader)

## Goal
- Support two-level runtime DLL search for Dawn:
  1) exe directory (packaged/runtime default)
  2) project fallback directory for local development
- Make diagnostics reflect both search scopes.

## Changes

### 1) Dawn Loader Search Order
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `LoadFirstAvailableDawnModule(...)` now:
  - tries default `LoadLibraryW(name)` first (includes exe directory)
  - if missing, tries project fallback folder:
    - `<repo>/MFCMouseEffect/Runtime/Dawn`
- The resolved module path is written to probe `module_name` when fallback path is used.

### 2) Runtime File Diagnostics Across Scopes
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `dawn_runtime_files` now reports per-scope checks with `search_scope`:
  - `exe_dir`
  - `project_fallback_dir`
- Each item includes: `name`, `exists`, `path`.

### 3) Web Banner Loader-Missing Rendering
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- For `loader_missing`, UI now distinguishes:
  - truly missing files (not found in either scope)
  - files found only in project fallback dir.

### 4) Project Runtime Folder
- Added:
  - `MFCMouseEffect/Runtime/Dawn/README.md`
- Documents where to place development Dawn DLLs.

## Result
- Packaging flow stays correct (exe-dir first).
- Development flow becomes easier (project fallback auto-load).
- Loader diagnostics are more actionable.
