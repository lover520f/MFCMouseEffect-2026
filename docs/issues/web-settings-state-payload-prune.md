# Web settings state payload pruning

## Background
- The Web settings page always sent a full `/api/state` payload on Apply.
- Even when users changed only one field, the request still carried all sections (`active`, `trail_profiles`, `trail_params`, etc.), making payloads noisy and harder to debug.

## Root cause
- `buildState()` in `MFCMouseEffect/WebUI/app.js` always assembled a full snapshot from form controls.
- No diffing step existed between form values and latest loaded state.

## What changed
- `buildState()` now returns a patch object (changed fields only) by comparing form values against `latestState`.
- Nested sections are pruned too:
- `active` only includes changed effect categories.
- `trail_profiles` only includes changed profile keys and changed leaf fields.
- `trail_params` only includes changed groups/leaf fields.
- Save action now skips `POST /api/state` when patch is empty.
- Save success now refreshes state once from server to keep local cache/banner consistent.

## Design notes
- This keeps API backward compatible: server still accepts partial `payload` and applies only provided keys.
- It reduces coupling between UI form size and request size.
- It avoids patch-style hacks in backend; pruning is done at source (frontend request construction).

## Manual verification
1. Open settings page from tray.
2. Change only `Theme`, click Apply, verify request payload only contains `theme`.
3. Change only one trail numeric field (for example `k_meteor_rate`), click Apply, verify payload only contains the related `trail_params` leaf.
4. Click Apply without any change, verify no `/api/state` write request is sent.

