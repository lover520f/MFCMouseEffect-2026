# Phase 3: GPU Header → H/CPP Separation

## 目标

将 8 个 header-only GPU 文件拆分为 H（声明）+ CPP（实现），减少编译耦合和编译时间。

## 改动文件

### Batch 1 — GPU Pipeline
| 文件 | 原大小 | Header 行数 | .cpp 行数 |
|------|--------|-------------|-----------|
| `QuantumHaloGpuV2ShaderPipeline` | 493 行 | ~70 | ~340 |
| `QuantumHaloGpuV2Presenter` | 515 行 | ~90 | ~350 |
| `QuantumHaloGpuV2ComputeEngine` | 181 行 | ~50 | ~130 |

### Batch 2 — D2D Backend
| 文件 | 原大小 | Header 行数 | .cpp 行数 |
|------|--------|-------------|-----------|
| `HoldQuantumHaloGpuV2D2DBackend` | 315 行 | ~50 | ~260 |
| `FluxFieldHudGpuV2D2DBackend` | 246 行 | ~45 | ~210 |
| `FluxFieldGpuV2ComputeEngine` | 191 行 | ~50 | ~140 |

### Batch 3 — DirectRuntime + Neon3DFx
| 文件 | 原大小 | Header 行数 | .cpp 行数 |
|------|--------|-------------|-----------|
| `HoldQuantumHaloGpuV2DirectRuntime` | 254 行 | ~48 | ~200 |
| `Neon3DFx` | 585 行 | ~60 | ~520 |

## 额外修复

1. **`Neon3DMath.h` include 路径错误**：`../RenderUtils.h` → `MouseFx/Renderers/RenderUtils.h`（原路径从 `Neon3D/` 向上一级到 `Hold/`，而非目标 `Renderers/`；之前因 header-only 未暴露问题）
2. **vcxproj 重复条目**：`RippleBasedHoldRuntime.cpp` 出现两次，移除重复项

## 消除的重复

所有 8 个文件中的本地 `ClampI`、`ClampF`、`HexHr`、`ComputeProgress` 等 helper 函数已替换为集中化的 `MathUtils.h`（`ClampInt`、`ClampFloat`）。

## 验证

- Build: **0 errors, 0 warnings** (Release x64)
- 每个 batch 独立构建验证后再进入下一批次
