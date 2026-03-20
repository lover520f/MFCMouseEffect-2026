# macOS App Packaging

## Purpose
- Build a standard macOS `.app` bundle that includes the core host binary, WebUI assets, pet assets, and bundled WASM plugins.
- Also emit `.zip` and `.dmg` wrappers around the same validated app payload.
- Keep packaging behavior explicit until a signed/notarized pkg flow exists.

## Current Packaging Contract
- Entry binary: `build-macos/mfx_entry_posix_host`
- App bundle: `MFCMouseEffect.app`
- Bundle executable: `Contents/MacOS/MFCMouseEffect` (the packaged app runs the host binary directly)
- Build preparation is shared with the normal macOS manual-run path: packaging now calls the same `mfx_manual_prepare_core_host_binary(...)` helper used by `./mfx run` / `./mfx start`, so core/WebUI full-build vs skip-build behavior stays aligned across run and package flows.
- Bundled runtime assets:
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/WebUI`
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-main.usdz`
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-main.joints.json`
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-actions.json`
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json`
  - `MFCMouseEffect.app/Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-effects.json`
  - `MFCMouseEffect.app/Contents/MacOS/plugins/wasm`
- Packaged app launches do **not** auto-open Web settings anymore; first-launch behavior now matches normal tray startup instead of debug-style browser popups.
- Bundle-relative runtime resolution is active:
  - WebUI resolves from `Contents/Resources/MFCMouseEffect/WebUI`
  - pet runtime assets resolve from `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source`
  - USDZ anonymous skeleton binding uses packaged `pet-main.joints.json` instead of shipping the full `pet-main.glb`
- App-bundle startup stabilization is active:
  - the macOS event loop now explicitly calls `NSApplication.finishLaunching()` before tray creation
  - tray startup happens before degraded input-capture warnings are emitted
  - warning notifications are posted asynchronously so first-run permission prompts no longer block tray initialization
- The app intentionally keeps repo-style relative asset paths under `Contents/Resources/MFCMouseEffect/...` so the runtime can continue to resolve `MFCMouseEffect/Assets/Pet3D/source/...`.
- To keep the macOS ARM64 package small, the packaged pet payload intentionally excludes `pet-main.glb`; only the runtime-used `pet-main.usdz` plus required JSON metadata files are copied.
- To keep the package small, the bundled sample wasm plugin now includes only the runtime-required files (`plugin.json` + `effect.wasm`); developer helpers such as `.wat`, `.d.ts`, `.js`, and `samples/` are excluded from packaged artifacts.
- Known open issue: Launchpad / Applications startup still needs a dedicated stabilization pass; local validation should continue to use `./mfx start` until the app-bundle launch path is unified with the direct host path.

## Build Command
```bash
/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/package/build-macos-portable.sh
```

## Common Options
```bash
# Reuse existing build + WebUI output
/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/package/build-macos-portable.sh \
  --skip-build \
  --skip-webui-build

# Emit folder only, skip zip creation
/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/package/build-macos-portable.sh \
  --no-zip

# Emit folder + zip only, skip dmg creation
/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/package/build-macos-portable.sh \
  --no-dmg
```

## Output Layout
- `Install/macos/MFCMouseEffect-macos-arm64-portable/`
- `Install/macos/MFCMouseEffect-macos-arm64-portable.zip`
- `Install/macos/MFCMouseEffect-macos-arm64-portable.dmg`

Inside the folder:
- `MFCMouseEffect.app`

Inside `MFCMouseEffect.app`:
- `Contents/Info.plist`
- `Contents/MacOS/MFCMouseEffect`
- `Contents/MacOS/plugins/wasm/demo.template.default.v2/*`
- `Contents/Resources/MFCMouseEffect/WebUI/*`
- `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-main.usdz`
- `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-main.joints.json`
- `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-actions.json`
- `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json`
- `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source/pet-effects.json`
- `Contents/Resources/MFCMouseEffect/theme-catalog/`

## Included Plugin Scope
- Current app package bundles the runnable sample plugin from:
  - `examples/wasm-plugin-template/dist`
- It is copied to:
  - `MFCMouseEffect.app/Contents/MacOS/plugins/wasm/demo.template.default.v2`
- Packaged runtime payload is intentionally minimal:
  - `plugin.json`
  - `effect.wasm`
- Discovery works without extra config because the runtime already scans the executable-relative root:
  - `plugins/wasm`

## Release Prep Checklist
1. Build host binary:
   - `cmake --build build-macos --target mfx_entry_posix_host -j8`
2. Build and sync WebUI:
   - `pnpm --dir MFCMouseEffect/WebUIWorkspace run build:mouse-companion`
   - `node MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`
3. Run manual smoke:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --build-dir build-macos --skip-build --skip-webui-build --no-open --auto-stop-seconds 60`
4. Build app package:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/./mfx package --skip-build --skip-webui-build`
5. Verify packaged folder:
   - `MFCMouseEffect.app` launches
   - menu-bar `MFX` item appears without auto-opening settings
   - WebUI opens
   - pet assets load
   - bundled wasm plugin appears in plugin catalog
6. Verify dmg:
   - `.dmg` mounts successfully
   - mounted image contains `MFCMouseEffect.app`
   - mounted image contains a real Finder `Applications` alias for drag-to-install

## Current Non-goal
- This script does not sign, notarize, or produce a `.pkg` installer yet.
- The emitted `.dmg` is an unsigned distribution wrapper around the validated `.app` bundle, not a notarized distribution pipeline.

## Unsigned Package Note
- Current macOS package is unsigned.
- On first launch, Gatekeeper may block the app depending on local policy.
- Internal test guidance:
  - use Finder context menu `Open`, or
  - clear quarantine manually only in trusted internal test environments

## Naming
- Default package name is `MFCMouseEffect-macos-arm64-portable`.
- The `arm64` marker is intentional and indicates the current portable package target is Apple Silicon only.
- Default output root is `Install/macos`, so macOS packaged artifacts now live under the same install-family location as Windows deliverables and avoid macOS case-insensitive `install/Install` ambiguity.
- Finder/DMG icon is now restored with low size impact:
  - packaging generates `Contents/Resources/AppIcon.icns` from a tiny AppKit-rendered `MFX` text iconset at package time
  - `Info.plist` sets `CFBundleIconFile=AppIcon`
  - current icon payload is about `210K`, with packaged artifacts around `2.5M` (`.dmg`) / `2.6M` (`.zip`)
  - this avoids the earlier `CoreSVG` warning noise from the `svg -> sips` path while keeping packaged artifacts close to previous size.
- DMG layout is now staged through Finder customization:
  - packaging creates a writable temporary image, mounts it, and lets Finder create a real `Applications` alias instead of a plain filesystem symlink
  - the alias is then assigned a lightweight system `Applications` icon so Finder shows an `Applications`-style install target without pulling in the larger full-size folder icon payload
  - icon positions are pinned to the standard install flow layout: `MFCMouseEffect.app` on the left, `Applications` on the right
  - the temporary Finder layout window is explicitly closed before detach, so local `./mfx package` runs do not leave the staging DMG window open after packaging finishes
  - the packaged host binary inside `MFCMouseEffect.app` is stripped with `strip -x`, and the final `.dmg` uses `UDBZ` compression to keep distribution size down.
