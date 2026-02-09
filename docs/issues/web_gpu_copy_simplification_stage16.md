# Web GPU 文案优化（去技术化 + 防误导）

## 目标
- 降低设置页中 GPU 区域的术语密度。
- 避免“已启用 GPU = 全链路 GPU 绘制”这类误导。
- 保留必要诊断能力，但默认不向普通用户暴露内部状态码。

## 修改文件
- `MFCMouseEffect/WebUI/app.js`

## 关键调整
1. 顶部 GPU 横幅改为用户语义
- 用状态映射文案替代直接拼接后端原始字段。
- 对 `request_*`、`loader_missing`、`handshake_skipped_debugger` 等状态提供简明提示。

2. 减少技术细节直出
- 默认不显示 `state_code`。
- 如需诊断，可通过 `?diag=1` 显示状态码。
- 渲染路径使用可读标签，不再直接显示原始内部代号。

3. 文案命名更贴近用户理解
- `Render Backend` -> `Acceleration Mode` / `加速模式`
- `GPU Bridge Mode` -> `Compatibility Mode` / `兼容策略`
- `Recheck GPU` -> `Refresh Acceleration` / `刷新加速状态`
- 相关状态提示统一为“刷新/优化/兼容策略”语义。

4. 防误导补充
- 在 GPU 兼容模式下明确提示“特效绘制仍可能占用较多 CPU”。
- 自动回退时不再展示生硬的内部回退字段，改为可理解的稳定性说明。
- 前端根据 `render_pipeline_mode` 强制归一化“加速级别”显示：
  - `dawn_compositor` / `dawn_host_compat_layered` 统一展示为“部分 GPU（特效仍以 CPU 绘制为主）”；
  - 不再直接信任后端可能残留的“完整 GPU 合成”文案，避免误判当前能力边界。
- 当动作仅是“重新探测”时，不再把这类诊断提示拼到主结论后，减少噪音。

## 验证
- `node --check MFCMouseEffect/WebUI/app.js` 通过。
