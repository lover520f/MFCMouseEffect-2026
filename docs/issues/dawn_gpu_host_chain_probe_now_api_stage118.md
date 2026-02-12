# Dawn GPU Host Chain Probe-Now API (Stage 118)

## Summary
Added an explicit one-shot API to trigger host-chain probe refresh and return capability/policy/host-chain status together.

## Why
`BuildStateJson` is periodic, but GPU host-chain activation diagnosis needs an on-demand trigger to quickly validate current machine/runtime readiness during iterative testing.

## What changed
1. New API endpoint in web settings server:
- `POST /api/gpu/host_chain_probe_now`

2. Request body:
- optional `refresh` (bool, default `true`)

3. Response includes:
- `gpu_final_present_capability`
- `gpu_final_present_policy`
- `gpu_final_present_host_chain`

## Safety
- No default render path change.
- Endpoint is diagnostics-only; it does not force final-present takeover.

## Validation
- `Release|x64` build passed.

## Next
Use this endpoint during DComp host-chain bring-up to verify activation attempts/success/failure with minimal test steps.
