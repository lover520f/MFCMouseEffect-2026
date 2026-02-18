# WASM Phase2 真落地验收（点击渲染链）

## 目标

验证 WASM 点击命令已接入真实可见渲染链，而非仅日志化：
- `spawn_text`：可见文字特效
- `spawn_image`：可见图标/图片指令渲染
- 命令执行失败时自动回退到内置点击特效，不中断主流程

## 覆盖范围

对应 5 个提交：
1. `spawn_text` 命令执行器接入点击链路
2. `spawn_image` 命令执行器接入点击链路
3. `text_id/image_id` 资源映射层（确定性映射 + 缺省回退）
4. 渲染执行诊断与预算/异常联动
5. 本文档验收固化

## 验收前置

1. 构建主工程（Debug x64）成功。  
2. 使用模板生成插件产物：
   - `examples/wasm-plugin-template/dist/effect.wasm`
   - `examples/wasm-plugin-template/dist/plugin.json`
3. 将产物放入插件目录（按 `plugin.json.id` 建子目录）。

## 最小验收步骤

1. 调用 `POST /api/wasm/load-manifest` 加载 manifest。  
2. 调用 `POST /api/wasm/enable` 启用插件。  
3. 鼠标点击桌面任意位置。  
4. 调用 `GET /api/state` 检查 `wasm` 字段。

## 关键验收点

1. 插件与运行时状态
- `wasm.enabled = true`
- `wasm.plugin_loaded = true`
- `wasm.runtime_backend` 有值（`dynamic_bridge` 或 `null`）

2. 命令执行状态
- `wasm.last_command_count > 0`（插件有输出）
- `wasm.last_rendered_by_wasm = true`（命中并执行渲染）
- `wasm.last_executed_text_commands` 或 `wasm.last_executed_image_commands` 至少一个大于 0

3. 回退与健壮性
- 若 `wasm.last_render_error` 非空，点击仍有可见反馈（回退到内置点击特效）
- 主线程不崩溃、不卡死

## 已知限制（当前阶段）

1. `spawn_image` 当前走图标渲染器池（`star/ripple`），后续可扩展到外部图片资源。
2. 命令里的 `delay_ms` 暂未做精确定时调度，当前按即时渲染处理。
3. 物理参数（`vx/vy/ax/ay`）在现有渲染器中为近似映射，后续可升级为严格轨迹积分。
