# WASM 插件目录支持自定义扫描路径

## 背景
- 之前 WASM 面板只能扫描固定目录（AppData 主目录 + 内置目录）。
- 用户无法在设置页持久化“自定义插件扫描目录”。

## 目标
- 在 Web 设置中支持配置可选的插件扫描路径。
- 持久化到 `config.json`。
- 让“插件目录扫描”和“导出全部插件”都使用该配置路径。

## 改动
- 配置模型新增 `wasm.catalog_root_path`，并打通 JSON 解析/序列化。
- 运行策略链路新增目录字段：
  - `wasm_set_policy` 支持 `catalog_root_path`。
  - `/api/wasm/policy` 接收并透传 `catalog_root_path`。
- 扫描路径解析升级：
  - `WasmPluginPaths::ResolveSearchRoots(const std::wstring& configuredRoot)`。
- 目录与导出接口升级：
  - `/api/wasm/catalog` 使用“默认目录 + 配置目录”联合扫描。
  - `/api/wasm/export-all` 使用同一套目录集合做导出源。
- WASM 面板升级：
  - 增加“扫描路径输入框 + 保存按钮”。
  - 增加保存成功/清空回退的状态提示。
  - 在元信息区域展示当前已配置扫描路径。

## 关键文件
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.h`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.Internal.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonKeys.Wasm.h`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.ApplySettings.cpp`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- `MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.cpp`
- `MFCMouseEffect/MouseFx/Server/SettingsStateMapper.cpp`
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/entries/wasm-main.js`
- `MFCMouseEffect/WebUI/i18n.js`
- `MFCMouseEffect/WebUI/styles.css`

## 行为说明
- `catalog_root_path` 为空时，自动回退为默认扫描目录集合。
- 目录扫描和“导出全部插件”会共用同一组扫描目录，避免行为不一致。
