# WASM 设置页：插件导入/导出（Phase3g）

## 目标
补齐可直接使用的插件管理动作：
- 把当前选中插件导入到主插件目录；
- 把当前扫描到的全部插件导出到时间戳目录。

## 行为定义
1. 导入所选插件
- 前端按钮：`添加到主目录`
- 数据来源：插件目录下拉框当前选中的 `manifest_path`。
- 后端行为：以 manifest 所在目录为插件根，复制到主目录：
  - `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id_或目录名>`
- 返回：目标 manifest 路径 + 主目录路径。

2. 导出全部插件
- 前端按钮：`导出全部插件`
- 数据来源：当前插件目录扫描结果（所有已发现插件）。
- 导出目录：
  - `%AppData%\\MFCMouseEffect\\exports\\wasm\\all-YYYYMMDD-HHMMSS`
- 返回：导出目录路径 + 导出数量。

## API
- `POST /api/wasm/import-selected`
  - 请求：`{ "manifest_path": "..." }`
  - 响应：`ok`、`error`、`manifest_path`、`primary_root_path`
- `POST /api/wasm/export-all`
  - 请求：`{}`
  - 响应：`ok`、`error`、`export_path`、`count`

## 实现位置
- 新增服务：
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- 路由接入：
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- 前端动作与文案：
  - `MFCMouseEffect/WebUI/app.js`
  - `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
  - `MFCMouseEffect/WebUI/i18n.js`

## 说明
- 导入策略是“复制到主目录”，不会删除源目录。
- 导出当前版本为“目录快照”，本阶段不做 zip。
- 当没有可导出插件时，后端返回 `ok=false` 和明确错误信息。
