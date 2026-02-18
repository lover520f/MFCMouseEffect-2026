# WASM 插件面板：命名统一与信息结构简化

## 背景
原有命名存在两类理解成本：
- “WASM 插件”分区名对用户不够语义化；
- “插件目录”“清单路径 + 已与配置路径同步”偏技术术语，不利于快速理解。

## 调整
1. 分区标题统一：
- `section_wasm_plugin`：`WASM 插件` -> `特效插件`。

2. 插件管理区命名统一：
- `title_wasm_block_catalog`：`插件目录` -> `插件信息`。
- `btn_wasm_refresh_catalog`：`刷新插件目录` -> `刷新插件列表`。
- `label_wasm_catalog_errors`：`目录错误` -> `插件扫描错误`。
- `label_wasm_catalog_roots`：`扫描目录` -> `插件扫描路径`。

3. 路径字段简化：
- 删除“已与配置路径同步”提示徽标。
- `label_wasm_manifest_path` 改为：`当前插件路径`。
- `label_wasm_configured_manifest_path` 改为：`插件配置路径`。

## 结果
- 用户先看到“特效插件/插件信息”，再看到“路径和扫描状态”，认知顺序更符合使用路径。
- 术语从技术导向改为用户导向，减少“目录/清单”混淆。
- 面板提示更精简，降低视觉噪音。

## 变更文件
- `MFCMouseEffect/WebUIWorkspace/src/shell/sections.js`
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `MFCMouseEffect/WebUI/i18n.js`
