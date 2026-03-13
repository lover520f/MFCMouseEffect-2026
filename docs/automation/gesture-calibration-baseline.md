# Gesture Calibration Baseline

Updated: 2026-03-13 10:55:18 +0800

## Sweep Grid

- margins: 2 3 4 5 6
- custom min effective stroke length(px): 12 18 24 30
- dataset cases: 6 (windowed-right, v, w, ambiguous-reject, custom-order, short-noise-reject)

## Results

| Margin | Custom Min Len(px) | Pass | Total |
|---:|---:|---:|---:|
| 2 | 12 | 6 | 6 |
| 2 | 18 | 6 | 6 |
| 2 | 24 | 6 | 6 |
| 2 | 30 | 6 | 6 |
| 3 | 12 | 6 | 6 |
| 3 | 18 | 6 | 6 |
| 3 | 24 | 6 | 6 |
| 3 | 30 | 6 | 6 |
| 4 | 12 | 6 | 6 |
| 4 | 18 | 6 | 6 |
| 4 | 24 | 6 | 6 |
| 4 | 30 | 6 | 6 |
| 5 | 12 | 6 | 6 |
| 5 | 18 | 6 | 6 |
| 5 | 24 | 6 | 6 |
| 5 | 30 | 6 | 6 |
| 6 | 12 | 6 | 6 |
| 6 | 18 | 6 | 6 |
| 6 | 24 | 6 | 6 |
| 6 | 30 | 6 | 6 |

## Recommended Baseline

- ambiguity margin env (`MFX_GESTURE_AMBIGUITY_MARGIN`): **4**
- custom min effective stroke env (`MFX_GESTURE_CUSTOM_MIN_EFFECTIVE_STROKE_PX`): **18**
- pass: **6/6**

## Case Pass Rate

- windowed-right: 20/20
- preset-v: 20/20
- preset-w: 20/20
- ambiguous-reject: 20/20
- custom-order: 20/20
- short-noise-reject: 20/20

## W Probe (margin=4, min_len=18)

- reason=accepted best_id=w best=100.00 runner=10.77 delta=89.23

## Reproduce

```bash
./tools/platform/manual/run-macos-gesture-calibration-sweep.sh --skip-build
```
