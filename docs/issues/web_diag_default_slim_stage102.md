# Web Diag Default Slim Snapshot (Stage 102)

## Problem
- Local diagnostic file `web_state_auto.json` could contain full Dawn timeline payload.
- This made routine debugging heavy and wasted context budget.

## Change
- `web_state_auto.json` now stores the slim snapshot by default.
- Full diagnostic payload is redirected to `web_state_full_auto.json`.

## Result
- Default local log read is lightweight and stable for iterative debugging.
- Full deep-dive data is still available when needed.
