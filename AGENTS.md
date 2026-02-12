# AGENTS.md

## Purpose
Provide project-local guidance for agents working in this repo.

## Tech Stack
- C++ (MFC/Win32, GDI+)
- Java
- Python
- Go
- Flutter (Dart)

## General Workflow
- Prefer small, incremental changes; keep effects responsive and lightweight.
- Avoid adding heavy dependencies unless explicitly requested.
- Favor clear separation between UI/tray code and effect rendering.
- Do not break current architecture by default. If a better architecture is identified, propose it proactively with tradeoffs/impact, get review feedback, then execute.
- When changes accumulate across multiple features/files, prioritize a checkpoint commit to avoid long-running stacked diffs.

## C++ (MFC/Win32)
- Use `rg` for searching.
- Keep new code ASCII unless the file already contains Unicode.
- Prefer `apply_patch` for single-file edits.
- Avoid blocking calls on UI threads; use timers or worker threads where needed.
- When adding rendering modes, reuse existing window/pool infrastructure.

## IPC / Background Mode
- Maintain compatibility with existing stdin JSON commands.
- New commands should be optional and backward compatible.
- If adding parent-process monitoring, gate it behind a flag or arg.

## Frontend / UI (Flutter)
- Respect existing visual language if UI exists.
- Keep UI optional for tray/background modes.

## Testing / Validation
- Provide manual test steps if automated tests are absent.
- If you add temporary test code, remove it before final response.

## Docs
- Update `README.md` or `README.zh-CN.md` if behavior changes.
