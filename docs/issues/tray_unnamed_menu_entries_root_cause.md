# Tray Menu `(Unnamed)` Entries Root Cause

Date: 2026-02-13

## Symptom
- Tray effect submenu shows valid items mixed with multiple `(Unnamed)` entries.
- Sometimes expected `None/无` item is missing or menu selection status is inconsistent.

## Root Cause
- In `TrayMenuBuilder::BuildTrayMenu`, calls were written like:
  - `AppendEffectSubMenu(..., ScrollMetadata(n), n);`
- This uses `n` both as an output parameter (inside `ScrollMetadata(n)`) and as an input argument (`n`) in the same function call expression.
- Function argument evaluation order is not guaranteed here, causing `n` to be read before being updated in some cases.
- Result: wrong `count` is passed to submenu builder, which iterates beyond metadata array bounds and produces unknown command IDs, shown as `(Unnamed)`.

## Fix
- Split metadata fetch and submenu append into two sequenced statements:
  - `opts = ScrollMetadata(n);`
  - `AppendEffectSubMenu(..., opts, n);`
- Applied to all categories (click/trail/scroll/hold/hover).

## Affected History
- Initial introduction came with metadata centralization refactor:
  - `329fd6d` (`refactor: 将特效进行集中管理`)
- Later defensive hardening (`ffb681c`) reduced crashes but did not remove this root cause.

## Validation
- Build `Release|x64` succeeds.
- Runtime tray submenu no longer shows random `(Unnamed)` items under normal metadata.
