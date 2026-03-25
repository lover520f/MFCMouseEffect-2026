# Windows Manual Handoff

## Purpose
- Synced short note for the current Windows-side manual step.
- Open this file directly from the Windows synced workspace root.

## Current Task
### Goal
- Build the latest synced Windows workspace and verify:
  - activating or hovering VMware no longer disables `trail` and `cursor decoration`
  - slow cursor movement keeps the trail continuous (no broken segments)
  - scroll effects use the correct direction and stay visually aligned with macOS

### Windows Working Path
- `F:\language\cpp\code\MFCMouseEffect`

### Step
1. Open `cmd` or PowerShell in `F:\language\cpp\code\MFCMouseEffect`.
2. Run `.\mfx.cmd build`.
3. Start the freshly built app: `.\x64\Release\MFCMouseEffect.exe`
4. Make sure `Effect Blacklist` does not contain `vmware.exe` or your VMware executable name.
5. Reproduce VMware path:
   - wake or activate the VMware window
   - move the cursor onto the VMware window and observe whether `trail` and `cursor decoration` already disappear
   - click once inside VMware
   - move the cursor back out of VMware
   - keep moving on the desktop and watch whether both `trail` and `cursor decoration` resume normally
6. Reproduce slow-move trail path:
   - on the desktop, move the mouse very slowly in a small circle
   - confirm the trail stays continuous (no broken segments even at slow speed)
7. Reproduce scroll direction path:
   - vertical wheel up: confirm the scroll effect points/moves upward
   - vertical wheel down: confirm the scroll effect points/moves downward
   - if your device supports horizontal wheel/tilt, test left and right once each
   - compare the overall scroll style with macOS and note only if you still see an obvious mismatch

### Send Back
- Reply with:
  - `已测，没问题`
  - or `已测，老问题`
  - if build fails, paste the last screenful of build output
