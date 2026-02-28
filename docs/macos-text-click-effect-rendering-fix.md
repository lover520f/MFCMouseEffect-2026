# macOS Floating Text Click Effect Rendering Fix

## Background
The user reported that the "Floating Text" click effect had no visual output on macOS. After applying fixes, the user noted that the text remained static without floating and eventually described the font size as too large and the starting position as too far from the cursor compared to Windows.

## Root Causes
Three distinct issues were identified within `MacosTextEffectFallback.mm`:

### 1. Zero Initial Alpha & Font Sizing
- **Alpha Timing:** The initial `alphaFactor` was 0.0, fading in over 15% of the duration. This caused the text to be completely invisible for the first ~120ms of the animation.
- **Font Sizing:** The initial fix boosted the minimum macOS font size to 24px because the original 8pt font became `~10px`, which is too small for Chinese characters on `CATextLayer`. However, Windows directly scales `fontSize * (96/72)`, leaving sizing under user control.

### 2. Disconnected Animation Dispatch Chain (Static Text)
- **Local Shared Pointer Destruction:** `StartTextAnimation` created a `std::shared_ptr<std::function>` called `step` to run the animation ticks. This pointer was a local variable.
- **Dangling Weak Pointer:** The `dispatch_after` block captured `step` as a `std::weak_ptr` (`weakStep`). When `StartTextAnimation` returned, the strong reference went out of scope, destroying the function object immediately. Upon execution of the delayed block, `weakStep` resolved to `nullptr`, terminating the animation after only one frame.

### 3. Layout Misalignment with Windows
- **Panel Size:** macOS used a hardcoded 240x120 panel, whereas Windows scales the backing render surface proportionally (`fontSize * 8`, min 200).
- **Positioning offset:** The macOS implementation originally placed the 120px tall panel centered vertically, but `CATextLayer` rendered its text at the *top edge* of that panel. This offset place the floating text ~60px physically above the actual mouse click coordinate.
- **Vertical Alignment:** Windows used `DWRITE_PARAGRAPH_ALIGNMENT_CENTER`, ensuring text sat in the exact center of its rendering bounds. macOS lacked vertical centering on its `CATextLayer`.

## Solution

1. **Alpha Timing:** Altered `alphaFactor` to start at `0.5` and fade to `1.0` over only 8% of the duration, ensuring instant visibility on click.
2. **Animation Lifeline:** Modified the block capture in `dispatch_after` to hold `step` as a *strong* reference. The retain cycle naturally breaks when the target `t >= 1.0`, or when the global animation generation state shifts. This resolves the static text bug.
3. **Aligned Layout Logic with Windows Implementation:**
   - Introduced proportional panel sizing `std::max(200.0, fontSize * 8.0)`.
   - Stripped the hardcoded 24px font size minimum, relying entirely on the user's `config.fontSize` with equivalent 96/72 scaling.
   - Refactored panel placement to perfectly center (X and Y) on the click origin.
   - Vertically centered the `CATextLayer` inside the panel by calculating the exact text height (`baseFontSize * 2.0`) and pushing its Y-origin down by half the remaining space.

## Validation
After these changes, the `MacosTextEffectFallback` implementation identically matches the sizing, positioning, and smooth displacement curvature as seen in `Win32TextEffectFallback`. The animation chain remains robust without orphaned dispatches.
