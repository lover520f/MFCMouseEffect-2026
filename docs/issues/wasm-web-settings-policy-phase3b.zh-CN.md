# WASM 设置策略控制（Phase 3b）

## 变更摘要

本次完成 WASM Web 设置的第一批“策略控制”落地：
- 策略可持久化到 `config.json`；
- 启动行为由持久化配置驱动；
- 点击链路的回退行为可由用户控制。

## 已落地策略

新增 `wasm` 配置段：
- `enabled`：启动后是否启用 WASM Host
- `fallback_to_builtin_click`：WASM 已接管但未产出可渲染命令时，是否回退到内置点击特效
- `manifest_path`：插件清单路径，用于启动时恢复

## 运行时行为变化

1. 启动阶段
- `AppController::InitializeWasmHost()` 现在会应用持久化 WASM 配置：
  - 先做配置清洗
  - 若有 `manifest_path` 则尝试加载
  - 应用持久化 `enabled` 状态

2. 点击分发策略
- `DispatchRouter` 接入回退策略判断：
  - WASM 路由激活且策略关闭时，不再回退内置点击特效
  - WASM 路由未激活时，保持原有内置点击行为

3. 命令/API 策略入口
- 新增命令：`wasm_set_policy`
- 新增接口：`POST /api/wasm/policy`
- 既有 `wasm_enable/disable/load-manifest` 改为通过控制器持久化配置

## 配置编解码改动

`EffectConfig` 新增 `WasmConfig` 并补齐 JSON 编解码：
- 解析：`EffectConfigJsonCodec.Parse.Wasm.cpp`
- 序列化：`EffectConfigJsonCodec.Serialize.Wasm.cpp`
- 键定义：`EffectConfigJsonKeys.Wasm.h`

## Web 设置页改动

WASM 面板新增：
- 持久化启用状态显示
- `fallback_to_builtin_click` 开关与保存动作
- 已配置清单路径显示

## 验证

1. 前端构建
- `MFCMouseEffect/WebUIWorkspace` 下 `pnpm run build` 通过。

2. 原生构建
- `Release|x64` MSBuild 通过（含新增文件与策略路由）。

## 风险说明

- 若关闭回退策略且插件返回空命令，点击可能无可见特效（属于策略设计结果）。
- 持久化 `manifest_path` 可能过期；启动加载失败时会保留诊断信息，不会导致主流程崩溃。
