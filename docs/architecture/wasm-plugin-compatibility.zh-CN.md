# WASM 插件兼容策略

本文定义 `MFCMouseEffect` 的 WASM 插件前后兼容规则。

## 1. 版本标识

- 宿主契约版本：`plugin.json` 的 `api_version`
- ABI 入口函数：
  - `mfx_plugin_get_api_version()`
  - 当前期望值：`1`

两者都需要与宿主支持的 ABI 主版本一致。

## 2. ABI 兼容规则

对于 ABI v1：
- 现有导出函数名保持不变；
- 二进制结构体字段顺序保持不变；
- 新字段只允许“尾部追加”；
- 已存在枚举值不可修改含义。

若要修改布局或函数契约，必须升级 ABI 主版本（v2+）。

## 3. Manifest 兼容规则

`plugin.json` 必填字段：
- `id`（插件稳定身份）
- `name`
- `version`
- `api_version`
- `entry`（wasm 相对路径）

规则：
- 升级插件时 `id` 应保持稳定；
- `entry` 必须落在插件目录内；
- 宿主会忽略未知字段，作者可扩展私有元数据。

## 4. 运行时兜底行为

若 runtime bridge 无法加载：
- 宿主自动回退到 `Null` runtime；
- 插件不生效，但主程序保持稳定。

若插件执行超预算：
- 当前事件输出可能被丢弃或截断；
- 诊断会写入 `/api/state` 的 `wasm` 字段。

## 5. 给插件作者的升级建议

- 按 ABI 主版本维护分支（如 `v1`、`v2`）；
- 用 tag/release 固定模板基线；
- 宿主 ABI 主版本变化后重新编译插件；
- 用以下接口验证：
  - `POST /api/wasm/load-manifest`
  - `POST /api/wasm/enable`
  - `GET /api/state`（看 `wasm.plugin_api_version`、`wasm.last_error`）

## 6. 当前宿主兼容承诺

- v1 宿主只接收 ABI v1 插件；
- 宿主可增加诊断字段或可选 manifest 字段，不破坏 v1 插件。
