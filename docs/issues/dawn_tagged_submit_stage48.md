# Stage48: Tagged Empty-Command Submission

## Goal
- Prepare trail and non-trail paths for independent Dawn draw-pass migration without breaking current stable submit flow.
- Add source-aware submission tagging so diagnostics can distinguish submission origin.

## Changes
### 1) Tagged submit API
- Files:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added:
  - `TrySubmitEmptyCommandBufferTagged(const char* tag, std::string* detailOut)`
- Existing API kept for compatibility:
  - `TrySubmitEmptyCommandBuffer(...)` now delegates to tagged version with `tag=nullptr`.

### 2) Consumer uses source tag
- File:
  - `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- Empty-command submit now passes tag by source:
  - `trail`
  - `ripple`
  - `particle`
  - `mixed`
- Runtime detail now carries tag suffix:
  - `empty_command_buffer_submit_ok_trail`
  - `empty_command_buffer_submit_ok_ripple`
  - etc.

## Why This Stage Matters
- Current submit remains stable (no render-pass behavior change).
- Stage49+ can replace tagged empty-submit branch-by-branch with real draw-pass calls while keeping diagnostics and fallback behavior intact.

