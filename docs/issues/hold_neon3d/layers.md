# Hold Neon3D - Layer Map (Concept Port)

This effect is a layer-by-layer port of the provided HTML canvas reference ("Hold Concept Demo v2") into GDI+.

## Draw Order (Bottom -> Top)

1. **Glass Shell Ring**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DRings.h` → `DrawGlassRing(...)`
   - Notes: multi-pass thick strokes approximate "glass depth" + fog glow.

2. **Micro Streaks (Inner Wall Scan)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawMicroStreaks(...)`
   - Notes: dense short arc segments starting at 12 o'clock, only ~30% circumference.

3. **Inner Hatch (Directional Texture Near Head)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawInnerHatch(...)`
   - Notes: only appears in a sector near `headAng`, faded in by progress.

4. **Crystal Seed (Center)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawCrystalSeed(...)`
   - Notes: path gradient fill + edge glow + inner core + vertical highlight.

5. **Tendrils (Trunk + Branches + Endpoints + Sparks)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawBranchTendrils(...)`
   - Notes:
     - One trunk Bezier (jittered by normals) then forks into `branchCount` branches.
     - Endpoints concentrate near the bright sector around `headAng`.
     - `bundleBiasRad` makes the trunk direction start different per hold instance.

6. **Subtle Progress Base Arc**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DRings.h` → `DrawProgressArcBase(...)`

7. **Woven Energy Band (Cyan/Purple/Mint)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawWovenBand(...)`
   - Notes:
     - Tail anchored at 12 o'clock; brightness concentrates near head to keep the "concept focus" region.
     - Fake depth via `z` then draw back-to-front (sort by `z`).
     - Uses a smaller `minor` wobble in GDI+ to keep the weave mostly inside the glass shell thickness.

8. **Capsule Head (Bright Short Segment)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DRings.h` → `DrawCapsuleHead(...)`
   - Notes: fixed sweep trailing `headAng`, multi-pass glow + bright core + head dot.

9. **Percent Label (Clamped to Top Sector)**
   - Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h` → `DrawPercentLabel(...)`
   - Notes:
     - Clamp tightened for ~220px window to avoid clipping.
     - `outwardPx` is computed by renderer to stay inside the window bounds.

## Color Palette
- Code: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DColor.h` → `MakeNeonPalette(...)`
- Notes: derives cyan/purple/mint via HSL hue offsets to keep the concept look even in chromatic/random themes.
