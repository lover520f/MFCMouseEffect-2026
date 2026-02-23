# Stage59 - DispatchRouter 功能路由下沉（WASM / 自动化 / 输入指示）

## 判定

`架构演进`：当前 `DispatchRouter.cpp` 同时承载消息分发、WASM 执行细节、自动化触发、输入指示调用，职责混合，导致功能分包边界不清晰。

## 目标

1. 将 `WASM`、`鼠标手势/自动化`、`键鼠指示` 三条路由细节从 `DispatchRouter` 下沉到独立模块。
2. 保持现有事件行为不变（包括 WASM 回退策略、长按流状态、自动化调用时机）。
3. 不破坏现有 `AppController` 与平台工厂边界。

## 变更摘要

### 1) 新增路由子模块（Core/Control）

新增文件：

- `MFCMouseEffect/MouseFx/Core/Control/WasmDispatchFeature.h`
- `MFCMouseEffect/MouseFx/Core/Control/WasmDispatchFeature.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AutomationDispatchFeature.h`
- `MFCMouseEffect/MouseFx/Core/Control/AutomationDispatchFeature.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/InputIndicatorDispatchFeature.h`
- `MFCMouseEffect/MouseFx/Core/Control/InputIndicatorDispatchFeature.cpp`

职责：

1. `WasmDispatchFeature`
   - 统一封装 WASM 事件调用与命令执行回写；
   - 维护 hold 流状态（`HoldStart/Update/End` 的 button + active）。
2. `AutomationDispatchFeature`
   - 统一封装输入自动化链路（click/move/scroll/button down/up/reset）。
3. `InputIndicatorDispatchFeature`
   - 统一封装输入指示器 click/scroll 通知。

### 2) DispatchRouter 简化为编排器

更新：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.h`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

要点：

1. 移除 `DispatchRouter` 内部的 WASM 执行实现与 hold 状态字段。
2. `OnClick/OnMove/OnScroll/OnButtonDown/OnButtonUp/OnTimer` 改为调用 feature handler。
3. 保持原有“WASM 渲染失败时按配置回退内置特效”的逻辑不变。

### 3) 工程接线同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

1. 新增 3 个 header 与 3 个 cpp 的工程编译项。
2. filters 归档到 `头文件\MouseFx` 与 `源文件\MouseFx`。

## 行为对齐说明

本次是结构重排，不是功能改写，保持以下外部行为一致：

1. click/scroll/move 的 WASM 路由与内置特效回退策略一致。
2. hold 定时触发后的 WASM `HoldStart/Update/End` 调用顺序一致。
3. 自动化引擎与输入指示器的触发时机保持一致。

## 验证

1. 构建命令（Release x64）：

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m
```

2. 结果：
   - 首次构建遇到 `LNK1104`（`MFCMouseEffect.exe` 被占用）；
   - 自动结束占用进程后重试构建通过：`0 error / 0 warning`。

## 收益

1. 四大功能分包边界更清晰：`特效`、`键鼠指示`、`WASM`、`鼠标手势/自动化`。
2. `DispatchRouter` 复杂度降低，后续按功能继续拆分成本更低。
3. 行为保持不变的前提下，增强可维护性与可测试性。
