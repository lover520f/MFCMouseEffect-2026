# Phase 55zzzzbo - macOS Scroll/Hover Color Profile Parity

## Background
- macOS scroll/hover overlays still used hardcoded color constants.
- Windows scroll/hover visual colors are theme-driven, so user theme/config changes are reflected consistently.

## Decision
- Keep existing macOS effect architecture and renderer contracts.
- Move scroll/hover overlay color selection to `MacosEffectRenderProfile`:
  - `ScrollRenderProfile` carries direction color slots.
  - `HoverRenderProfile` carries glow/tubes color slots.
- Resolve these colors from runtime config/theme in profile resolver, then consume only profile colors in style/render layers.

## Implementation
1. Profile contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.ScrollHoldHover.cpp`
- Added:
  - `ScrollRenderProfile::DirectionColor` for `horizontal+/horizontal-/vertical+/vertical-`
  - `HoverRenderProfile::ColorProfile` for `glowFill/glowStroke/tubesStroke`
- Resolver mapping:
  - scroll uses config-driven click button palettes (`left/middle/right`) and brightness-adjusted variant for horizontal negative direction.
  - hover uses config-driven click palettes (`left/middle`) and blended stroke mapping for tubes mode.

2. Scroll style/render path switch to profile input
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.mm`
- Removed hardcoded direction color constants from runtime path.

3. Hover style/render path switch to profile input
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayStyle.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayStyle.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.mm`
- Removed hardcoded hover glow/tubes colors from runtime path.

## Validation
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`

## Impact
- Capability: `特效（scroll + hover）`
- User-visible:
  - macOS scroll/hover colors now follow runtime theme/config profile resolution.
  - direction/type differences are preserved; colors are no longer fixed constants.
- Windows/Linux behavior unchanged.
