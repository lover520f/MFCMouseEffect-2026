# Scroll Twinkle Starfield Effect

## Background
- Scroll effects currently provide `arrow` and `helix`.
- A particle-style starfield/twinkle visual was requested for scroll interactions.

## Goal
- Add a new scroll renderer `twinkle`.
- Keep it fully integrated with existing architecture:
  - renderer registry
  - effect factory typed route
  - settings metadata and web UI options
  - tray command mapping

## Implementation

### 1) New renderer
- Added `MFCMouseEffect/MouseFx/Renderers/Scroll/TwinkleRenderer.h`
- Uses directional one-shot particles (no center accumulation) with:
  - burst emission only at ripple start (avoid continuous center stacking)
  - velocity aligned to `RenderParams.directionRad`
  - constrained spray cone: total 60 degrees (`+-30 degrees`)
  - light turbulence + drag for natural spread
  - per-particle lifetime/size decay + subtle flicker
  - particle-dot + short streak (no fog blob / no star polygon fill)
- Registered as:
  - `REGISTER_RENDERER("twinkle", TwinkleRenderer)`

### 2) Scroll runtime wiring
- `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
  - includes `TwinkleRenderer.h` to ensure static registration.
  - adds `twinkle` input-shaping profile (`emitInterval=30ms`, `maxActive=3`, `maxDuration=220ms`) to suppress overdraw stacking.
  - keeps the existing theme application chain unchanged (including `chromatic` randomization behavior).
- `MFCMouseEffect/MouseFx/Effects/ScrollEffect.h`
  - adds twinkle profile constants and renderer-type check helper.

### 5) Quality tuning after visual review
- Reworked `twinkle` renderer to a directional spray particle style:
  - particle-dot + short streak rendering (no center fog blob)
  - one-shot directional burst with drag/turbulence
  - hard cone constraint at 60 degrees for cleaner directionality
  - keeps color source from current theme style (`stroke/glow`), so visual still follows theme configuration
- Input shaping adjusted to balanced spray behavior:
  - `emitInterval=30ms`, `maxActive=3`, `maxDuration=220ms`
- Direction mapping correction:
  - fixed vertical wheel sign mapping in `ScrollEffect` so up/down visual direction matches observed runtime wheel behavior.

### 3) Effect creation route
- `MFCMouseEffect/MouseFx/Core/Control/EffectFactory.cpp`
  - adds typed scroll effect entry:
    - `"twinkle" -> CreateScroll`

### 4) Settings and tray integration
- `MFCMouseEffect/MouseFx/Interfaces/EffectCommands.h`
  - adds `kCmdScrollTwinkle = 5004`.
- `MFCMouseEffect/Settings/SettingsOptions.h`
  - adds `ScrollMetadata` option:
    - value: `twinkle`
    - zh: `星尘喷流`
    - en: `Stardust Stream`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
  - adds label mapping, current-selection matching, and command->JSON mapping for `twinkle`.

## Validation
1. Open Settings Web UI and select scroll effect `Stardust Stream`.
2. Scroll wheel up/down continuously:
   - twinkling star particles should emit around cursor and move with scroll direction.
3. Switch between `arrow`, `helix`, `twinkle`:
   - all should apply immediately.
4. Verify tray menu selection also switches to `twinkle`.
