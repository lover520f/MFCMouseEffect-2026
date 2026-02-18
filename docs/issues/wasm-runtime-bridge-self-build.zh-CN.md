# WASM Runtime DLL 自构建落地

## 背景

宿主侧已按动态桥接方式加载：
- `mfx_wasm_runtime.dll`

但仓库里此前没有可直接编译该 DLL 的工程，导致默认总是回退到 `null` runtime（除非用户手工放入外部 DLL）。

## 目标

把 `mfx_wasm_runtime.dll` 变成仓库内可复现构建产物：
- 跟随主工程一起编译，
- 输出到 `MFCMouseEffect.exe` 同目录，
- 安装包自动携带。

## 实现内容

### 1. 新增 runtime bridge 工程

新增工程：
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj`

新增桥接实现：
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeExports.cpp`
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.def`

新增三方运行时源码（仓库内 vendoring）：
- `MFCMouseEffect/WasmRuntimeBridge/third_party/wasm3/*`

### 2. 对齐宿主 ABI 导出

DLL 现在完整导出宿主所需符号：
- `mfx_wasm_runtime_create`
- `mfx_wasm_runtime_destroy`
- `mfx_wasm_runtime_load_module_file`
- `mfx_wasm_runtime_unload_module`
- `mfx_wasm_runtime_is_module_loaded`
- `mfx_wasm_runtime_call_get_api_version`
- `mfx_wasm_runtime_call_on_click`
- `mfx_wasm_runtime_reset_plugin`
- `mfx_wasm_runtime_last_error`

通过 `.def` 固定导出名，避免 x86/x64 名称修饰差异导致 `GetProcAddress` 失败。

### 3. 构建链路接线

已更新：
- `MFCMouseEffect.slnx`（纳入 runtime bridge 工程）
- `MFCMouseEffect/MFCMouseEffect.vcxproj`（增加 ProjectReference）

结果：
- 构建 `MFCMouseEffect` 时会联动构建 runtime bridge；
- `mfx_wasm_runtime.dll` 输出到 `x64/Debug` 或 `x64/Release`，与主 exe 同目录。

### 4. 安装包接线

已更新：
- `Install/MFCMouseEffect.iss`

新增：
- 打包前校验 `x64/Release/mfx_wasm_runtime.dll` 是否存在
- 安装包把 DLL 复制到 `{app}` 目录

## 运行结果

落地后：
- 不再需要单独下载 runtime DLL；
- 只要本地编译通过，`Cannot load mfx_wasm_runtime.dll` 这类回退原因应消失。

若仍回退：
- 查看 `wasm.runtime_fallback_reason` 与 `wasm.last_error`；
- 确认 `mfx_wasm_runtime.dll` 是否与 `MFCMouseEffect.exe` 同目录。
