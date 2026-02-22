# WASM 多动作事件入口补齐（点击/移动/滚轮/长按/悬停）

## 问题判定
- 结论：`功能缺口`（不是单点崩溃 Bug）。
- 依据：此前 WASM 路线以 `mfx_plugin_on_click` 为主，非点击动作没有统一插件入口，无法满足“5 个动作可自定义”的目标。

## 目标
- 补齐 WASM 对以下动作的宿主链路支持，并收敛为 `on_event` 单入口：
  - 点击（Click）
  - 移动（Move）
  - 滚轮（Scroll）
  - 长按（HoldStart/HoldUpdate/HoldEnd）
  - 悬停（HoverStart/HoverEnd）

## 方案

### 1. ABI 收敛
- 使用统一事件输入结构 `EventInputV1`（28 bytes）。
- 统一事件种类：
  - `Click/Move/Scroll/HoldStart/HoldUpdate/HoldEnd/HoverStart/HoverEnd`
- 宿主移除 click 专用输入结构 `ClickInputV1` 的消费路径。

### 2. Runtime / Bridge 收敛
- runtime 接口只保留统一调用：
  - `CallOnEvent(...)`
- bridge 导出收敛为：
  - `mfx_wasm_runtime_call_on_event`
- wasm3 插件导出解析收敛为：
  - `mfx_plugin_on_event`（必需）

### 3. 宿主调用链收敛
- `WasmEffectHost` 统一走 `InvokeEvent(...) -> on_event`。
- 删除点击回退分支（不再调用 `on_click`）。
- `DispatchRouter` 与 `AppController` 的五动作触发链均可进入统一事件入口。

## 模板与文档同步
- 模板 ABI helper 保留通用事件读写（`assembly/common/abi.ts`）。
- 模板内置样例全部迁移为导出 `mfx_plugin_on_event`。
- 新增滚轮样例：`scroll-particle-burst`。
- 文档口径统一为“宿主 `on_event` 单入口”。

## 关键改动文件
- 宿主核心：
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmRuntime.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmEffectHost.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmEffectHost.cpp`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmBinaryCodec.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmBinaryCodec.cpp`
- runtime 与桥接：
  - `MFCMouseEffect/MouseFx/Core/Wasm/DllWasmRuntime.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/DllWasmRuntime.cpp`
  - `MFCMouseEffect/MouseFx/Core/Wasm/NullWasmRuntime.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/NullWasmRuntime.cpp`
  - `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.h`
  - `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`
  - `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeExports.cpp`
  - `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.def`
- 模板与文档：
  - `examples/wasm-plugin-template/assembly/common/abi.ts`
  - `examples/wasm-plugin-template/assembly/samples/*.ts`
  - `examples/wasm-plugin-template/assembly/index.ts`
  - `examples/wasm-plugin-template/README.md`
  - `examples/wasm-plugin-template/README.zh-CN.md`

## 验证
1. C++ 编译（Release x64）通过：
   - `mfx_wasm_runtime.vcxproj`
   - `MFCMouseEffect.vcxproj`
2. 模板样例构建通过：
   - `node ./scripts/build-all-samples.mjs`

## 兼容性结论
- 当前宿主要求插件必须导出 `mfx_plugin_on_event`。
- 仅导出 `mfx_plugin_on_click` 的旧插件在当前宿主不再可用。
