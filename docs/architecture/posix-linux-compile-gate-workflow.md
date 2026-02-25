# POSIX Linux Compile Gate Workflow

## Priority Command
Run this to verify Linux package targets stay buildable from current host:

```bash
./tools/platform/regression/run-posix-linux-compile-gate.sh
```

Optional:

```bash
./tools/platform/regression/run-posix-linux-compile-gate.sh --build-dir /tmp/mfx-platform-linux-build --jobs 12
```

Default behavior builds two compile lanes:
- default lane: `MFX_ENABLE_POSIX_CORE_RUNTIME=OFF`
- core-runtime lane: `MFX_ENABLE_POSIX_CORE_RUNTIME=ON`

Optional (faster local loop, lower coverage):

```bash
./tools/platform/regression/run-posix-linux-compile-gate.sh --skip-core-runtime
```

## What It Verifies
1. Linux package configure with cross-host support enabled.
2. Linux shell/runtime targets compile successfully on default lane:
   - `mfx_shell_linux`
   - `mfx_entry_posix`
3. Linux shell/runtime targets compile successfully on core-runtime lane:
   - `mfx_shell_linux`
   - `mfx_entry_posix`

## Packaging Layout (Keep This Boundary)
- Entry orchestrator:
  - `tools/platform/regression/run-posix-linux-compile-gate.sh`
- Linux gate implementation:
  - `tools/platform/regression/lib/linux_gate.sh`
- Shared assertions/log helpers:
  - `tools/platform/regression/lib/common.sh`

## Extension Rule
1. Keep the entry script orchestration-only.
2. Keep Linux gate build logic in `lib/linux_gate.sh`.
3. If target set or flags change, update this doc and the corresponding `docs/refactoring/*` phase note in the same commit.
