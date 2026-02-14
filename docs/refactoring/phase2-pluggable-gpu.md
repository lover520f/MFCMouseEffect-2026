# Phase 2: GPU 可插拔架构（IHoldRuntime）

## 概述

将 `HoldEffect` 从直接依赖 9 个 renderer 头文件 + `if/else` GPU 分支，重构为仅依赖 `IHoldRuntime` 接口。符合**开闭原则**和**依赖倒置原则**。

## 设计模式

- **策略模式 (Strategy)**：`IHoldRuntime` 定义统一的 `Start/Update/Stop/IsRunning` 接口
- **抽象工厂 (Abstract Factory)**：`HoldRuntimeRegistry` 支持按名称创建 runtime
- **适配器模式 (Adapter)**：`RippleBasedHoldRuntime` 包装 `OverlayHostService` 的 ripple 系统

## 新增文件

| 文件 | 目的 |
|------|------|
| `Interfaces/IHoldRuntime.h` | 策略接口：`Start/Update/Stop/IsRunning` |
| `Renderers/HoldRuntimeRegistry.h` | 抽象工厂 + `REGISTER_HOLD_RUNTIME` 宏 |
| `Effects/RippleBasedHoldRuntime.h` | 适配器声明 |
| `Effects/RippleBasedHoldRuntime.cpp` | 适配器实现 — 包装 CPU/GPU V2 ripple 路径 |

## 修改文件

| 文件 | 改动 |
|------|------|
| `Effects/HoldQuantumHaloGpuV2DirectRuntime.h` | 继承 `IHoldRuntime`，`override` 关键字，`ClampI/F` → `MathUtils.h` |
| `Effects/HoldEffect.h` | 移除 9 个 renderer include + GPU 分支 flag，换为 `unique_ptr<IHoldRuntime>` |
| `Effects/HoldEffect.cpp` | 完全移除 GPU/CPU 分支逻辑，委托给 `runtime_->Start/Update/Stop` |
| `MFCMouseEffect.vcxproj` | 添加 `RippleBasedHoldRuntime.cpp` |

## 关键设计决策

1. **FollowMode smoothing 保留在 HoldEffect** — 这是交互层策略，不下沉到渲染层
2. **CreateRuntime() 工厂函数** — 先查 Registry，再按类型名回退到 direct GPU 或 ripple
3. **诊断快照保留在 HoldEffect** — 作为可观测性钩子，不侵入 runtime 实现
4. **Renderer 头文件 include 移到 RippleBasedHoldRuntime.cpp** — HoldEffect 彻底解耦

## 验证

- ✅ Release x64 构建：0 错误，0 警告
