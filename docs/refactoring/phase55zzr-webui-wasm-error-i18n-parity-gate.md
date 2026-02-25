# Phase 55zzr: WebUI WASM Error i18n Parity Gate

## Capability
- WASM

## Why
- Runtime/transfer error-code mapping already expanded in `action-error-model`.
- A regression risk remained: future additions could update error-model keys but forget to add matching EN/ZH entries in `WebUI/i18n.js`.

## Scope
- Add a script-level contract check that validates all i18n keys referenced by WASM action error-model exist in both `en-US` and `zh-CN` dictionaries.
- Keep runtime behavior unchanged.

## Code Changes
1. Exported model key list:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/src/wasm/action-error-model.js`
   - Added `listWasmActionErrorI18nKeys()` for test-time contract checks.
2. Expanded wasm error-model test:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/scripts/test-wasm-action-error-model.mjs`
   - Added checks:
     - key uniqueness in model mapping
     - each key exists in `WebUI/i18n.js` `en-US`
     - each key exists in `WebUI/i18n.js` `zh-CN`

## Validation
- `node /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/scripts/test-wasm-action-error-model.mjs`
- `pnpm --dir /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace test:wasm-error-model`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime contract change.
- Adds a preventative quality gate to avoid future error-code/i18n drift in WebUI.
