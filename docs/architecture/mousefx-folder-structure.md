# MouseFx Folder Structure (2026-01-30)

## Goal
`MFCMouseEffect/MouseFx` had too many files at one level. This change refolders MouseFx into responsibility-based subfolders to make navigation easier, without changing behavior.

## Layout
- `MFCMouseEffect/MouseFx/Core/`: lifecycle/config/IPC/hook glue
- `MFCMouseEffect/MouseFx/Effects/`: effect entrypoints per category
- `MFCMouseEffect/MouseFx/Windows/`: layered windows and pools
- `MFCMouseEffect/MouseFx/Styles/`: theme + style parameters
- `MFCMouseEffect/MouseFx/Interfaces/`: shared interfaces/strategies
- `MFCMouseEffect/MouseFx/ThirdParty/`: vendored single-header libs (e.g. `json.hpp`)
`MFCMouseEffect/MouseFx/Legacy/`: kept for reference only (old `RenderStrategies/StandardRenderers` removed; use git history).
- `MFCMouseEffect/MouseFx/Renderers/`: render implementations (unchanged)

## Notes
- Cross-folder includes prefer project-root paths like `MouseFx/...` to avoid fragile relative paths.
- This is a refolder + include/vcxproj sync only.
