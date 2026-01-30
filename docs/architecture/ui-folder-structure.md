# UI Folder Structure (2026-01-30)

## Goal
Reduce top-level clutter under `MFCMouseEffect/` by grouping UI files into feature folders, without changing behavior or adding new frameworks.

## Layout
- `MFCMouseEffect/UI/Tray/`: tray host window + menu builder
- `MFCMouseEffect/UI/Settings/`: settings window UI + emoji preview/formatting
- `MFCMouseEffect/UI/Frame/`: MFC frame windows
- `MFCMouseEffect/UI/Panes/`: docking panes / auxiliary views
- `MFCMouseEffect/UI/DocView/`: SDI Doc/View layer

## Notes
- Includes use project-root-relative paths (e.g. `UI/Frame/MainFrm.h`) to avoid fragile relative paths.
- Behavior unchanged; this is a structural refolder + vcxproj sync.

