# Phase56zzp4: WASM Capability Schema-State Contract Parity Gate

## Summary
- Category: `WASM`
- Goal: add explicit contract assertion that schema capability and runtime state capability stay consistent.
- Scope: regression script only.

## Change
1. `tools/platform/regression/lib/core_http_state_checks.sh` adds JSON bool reader helper (`python3`) for deterministic key extraction.
2. Core state checks now assert:
   - `state.wasm.invoke_supported == schema.capabilities.wasm.invoke`
   - `state.wasm.render_supported == schema.capabilities.wasm.render`

## Why
- Even with shared runtime resolver, contract guard is still required to prevent future one-sided edits.
- This turns capability consistency from "implementation assumption" into "test-enforced invariant".

## Validation
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-build`
2. `./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto`

All passed.
