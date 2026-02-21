# WASM Demo Click Mixed Text Image (0.1.1) 执行型访问冲突修复

## 问题判定
- 结论：`Bug / 回归`
- 依据：运行期触发 `0xC0000005` 执行访问冲突，且崩溃位于 WASM bridge 调用链路，不属于设计行为。

## 现象
- 使用 `Demo Click Mixed Text Image (0.1.1)` 时，进程可能直接退出。
- 调试器典型报错：
  - `0xC0000005`（执行访问冲突）
  - 故障地址表现为随机可执行地址。

## 证据链
- 崩溃转储定位到 `mfx_wasm_runtime.dll` 的 wasm 运行时调用路径。
- 对返回地址符号化后命中：
  - wasm3 执行阶段 `op_MemGrow`
  - 上层调用点 `RuntimeBridgeContext::CallOnClick`
- 与“复杂点击插件执行时可能触发运行时内存增长”的行为一致。

## 根因
1. 宿主到 wasm 调用使用了变参接口（`m3_CallV` / `m3_GetResultsV`）。
- 在复杂路径下，这条链路 ABI/栈安全裕量较低，失败形态不稳定。

2. `on_click` 调用后输出拷贝仍使用调用前缓存的 memory 指针。
- 一旦插件执行触发 `memory.grow`，旧 linear memory 指针可能失效。

3. 运行时调用边界缺少 SEH 保护。
- 低层故障会直接击穿到进程层，无法进入受控错误处理。

## 修复方案
1. 变参调用改为强类型调用：
- `m3_Call`
- `m3_GetResults`

2. 强化输出内存拷贝安全：
- `on_click` 后重新获取 linear memory 指针
- 按调用后 memory 大小重新做边界校验
- 仅在校验通过后执行拷贝。

3. 新增受保护调用包装：
- `SafeM3Call`
- `SafeM3GetResults`
- 在 bridge 边界捕获 SEH，并转为可读错误信息。

4. 新增故障后恢复路径：
- 基于缓存的 wasm bytes 重建 runtime
- 重新解析导出函数
- 保证 bridge 进入“可继续运行”的状态。

5. 提升 runtime 栈空间：
- `64 KB -> 256 KB`，覆盖复杂插件执行深度。

## 变更文件
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.h`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`

## 验证结果
1. Release x64 构建通过：
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`

2. 运行时压力验证：
- `text-rise` 样例高频 `on_click` 调用可稳定运行，无进程崩溃。
- `mixed-text-image` 在导入前置条件不满足时返回确定性错误 `missing imported function`，不再硬崩。

## 说明
- 本次修复目标是“访问冲突消除 + runtime 边界稳态恢复”。
- 插件导入不匹配属于兼容性问题，需作为独立议题处理，不应再以崩溃形态暴露。

## 2026-02-21 追踪修复：样例仍提示“异常”

### 更新判定
- 结论仍为 `Bug`，并非表情包/图片资源本身导致。
- 直接原因：AssemblyScript 产物依赖的 `env.abort` 导入未被宿主链接。

### 证据
- `mixed-text-image/effect.wat` 存在导入：
  - `(import "env" "abort" (func ... (param i32 i32 i32 i32)))`
- 修复前首次调用 `on_click` 就返回 `missing imported function`。

### 追踪修复内容
- 在 bridge 模块加载阶段新增宿主导入链接：
  - `env.abort`，签名 `v(iiii)`（AssemblyScript abort ABI）
  - `env._abort`，签名 `v()`（兼容兜底）
- 导入链接在导出函数解析前完成。

### 追踪验证结果
- `mixed-text-image`：`1000` 次 `on_click` 全部成功。
- `image-affine-showcase`：`200` 次 `on_click` 全部成功。
- `text-rise`：`3000` 次 `on_click` 全部成功。

### 追踪改动文件
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.h`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`

## 2026-02-21 兼容性追踪：插件命令 kind=3 未渲染

### 更新判定
- 结论仍为 `Bug`，并非表情/图片文件损坏。
- 直接原因：宿主命令解析器仅接受 `kind=1/2`。
- 新样例包（`image-affine-showcase`、`mixed-text-image`）会输出 `kind=3,size=88` 的图片命令。

### 证据
- 直接调用运行时 `mfx_wasm_runtime_call_on_click` 可得到有效命令流：
  - `image-affine-showcase`：首条命令即 `kind=3,size=88`
  - `mixed-text-image`：先 `kind=1,size=56`，后 `kind=3,size=88`
- 宿主此前将 `kind=3` 判定为 `UnsupportedCommandKind`，导致渲染链路无可见效果。

### 兼容修复
- ABI 枚举补充：
  - `CommandKind::SpawnImageAffine = 3`
- 新增布局：
  - `SpawnImageAffineCommandV1`（`88` 字节，前缀为 `SpawnImageCommandV1`，尾部为仿射扩展字段）
- 解析器接受 `kind=3`（最小长度 `88`）。
- 执行器对 `kind=3` 复用现有图片渲染路径（使用基础图片字段）。
  - 仿射扩展字段先安全忽略，保留后续渲染器能力扩展空间。

### 兼容追踪改动文件
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmCommandBufferParser.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`

### 验证
- Release x64 构建通过：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`

## 2026-02-21 追踪：插件切换 API 可能“假成功”

### 更新判定
- 结论：`Bug`。
- 现象：Web WASM 面板切换 manifest 后，运行效果仍停留在上一个插件。

### 根因
- `POST /api/wasm/load-manifest` 之前通过 `HandleCommand("wasm_load_manifest")` 执行，再用 `Diagnostics().pluginLoaded` 判定成功。
- 当旧插件已加载时，即使新 manifest 加载失败，`pluginLoaded` 仍可能保持 `true`。
- 导致接口返回 `ok=true` 的假阳性，UI 认为切换成功，但实际 active plugin 未变化。

### 修复
- `WebSettingsServer` 的 load-manifest 路由保持 UI 线程安全分发：
  - 通过 `controller_->HandleCommand({"cmd":"wasm_load_manifest", ...})` 执行
  - 再用运行时诊断做成功判定：
    - `plugin_loaded == true`
    - `active_manifest_path` 与请求路径归一化后相同。
- 当 manifest 实际未切换成功时，接口返回：
  - `ok=false`
  - `error="manifest switch did not take effect"`
  - 避免继续出现“假成功”。
- `CommandHandler::HandleWasmLoadManifestCommand` 改为优先用 `nlohmann::json` 解析 `manifest_path`，仅在解析失败时回退 `JsonLite`。

### 追踪改动文件（切换链路）
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.cpp`

## 2026-02-21 前端追踪：下拉选择被强制回滚到当前插件

### 更新判定
- 结论：`Bug / 回归`。
- 现象：Web WASM 面板中，手动在下拉框选择其他插件后，选择值会在点击 `Load Selected` 前被回滚到当前 active 插件。
- 影响：看起来切换了，实际请求仍是旧 manifest，表现为“始终切不到新插件”。

### 根因
- 运行时前端包（`wasm-settings.svelte.js`）存在一段持续同步逻辑，会不断把 `selectedManifestPath` 覆盖为 `active_manifest_path`。
- 该逻辑造成选择锁定，阻断了真实切换链路。

### 修复
- 在 `WasmPluginFields.svelte` 中：
  - 移除持续覆盖 active 路径的行为
  - 增加 manifest 路径归一化匹配
  - 目录刷新时优先保持用户当前选择
  - `loadManifest` 成功后仅做一次目录刷新，并将选择对齐到新 active manifest。
- 重新构建 WASM 分区前端包，并同步运行时 webui 资产（`x64/Debug/webui`、`x64/Release/webui`）。

### 追踪改动文件（前端）
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `x64/Debug/webui/wasm-settings.svelte.js`（运行时生成资产）
- `x64/Release/webui/wasm-settings.svelte.js`（运行时生成资产）
