# WASM 插件：目录导入（后端校验 + 复制到主目录）

## 背景
之前的导入流程依赖“先扫描到清单，再导入所选清单”，对用户不直观。  
目标改为：用户直接选择插件目录，系统自动校验并导入。

## 方案
1. Web 设置页提供“导入插件目录”按钮。
2. 点击后由后端弹系统目录选择器（不是前端手动拼路径）。
3. 选择目录后，后端执行校验：
   - 目录内必须存在 `plugin.json`；
   - `plugin.json` 必须通过 manifest 校验；
   - `entry` 对应的 `.wasm` 文件必须真实存在且是普通文件。
4. 校验通过后，将插件目录复制到默认主目录：
   - `%AppData%\MFCMouseEffect\plugins\wasm\<plugin_id_或目录名>`

## 接口
- `POST /api/wasm/import-from-folder-dialog`
  - 请求：`{ "initial_path": "..." }`（可选）
  - 响应：
    - 成功：`ok=true`，返回 `manifest_path`、`primary_root_path`
    - 取消：`ok=false` + `cancelled=true`
    - 失败：`ok=false` + `error`

## 实现点
- 原生目录选择器：
  - `MFCMouseEffect/MouseFx/Core/System/NativeFolderPicker.h`
  - `MFCMouseEffect/MouseFx/Core/System/NativeFolderPicker.cpp`
- 路由接入：
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- 导入校验增强（entry wasm 存在性校验）：
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- 前端动作与文案：
  - `MFCMouseEffect/WebUI/app.js`
  - `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
  - `MFCMouseEffect/WebUI/i18n.js`

## 结果
- 用户不需要手动定位 `manifest_path`。
- 导入前即可拦截缺失 `plugin.json` 或缺失 `entry wasm` 的坏插件。
- 导入后自动刷新插件目录并选中导入结果，能继续“一键加载”。
