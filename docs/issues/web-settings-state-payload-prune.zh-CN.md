# Web 设置页 state 精简（按差异提交）

## 背景
- 之前 Web 设置页点击“应用”时，总是提交完整 `/api/state` 负载。
- 即使只改 1 个字段，也会把 `active`、`trail_profiles`、`trail_params` 等全部带上，排查和 review 都很重。

## 根因
- `MFCMouseEffect/WebUI/app.js` 里的 `buildState()` 一直构造“全量快照”。
- 缺少“表单值 vs 最近状态”的差异计算步骤。

## 改动
- `buildState()` 改为返回“仅变化字段”的 patch。
- 嵌套对象也做了裁剪：
- `active` 只保留发生变化的分类键。
- `trail_profiles` 只保留变化的 profile 和叶子字段。
- `trail_params` 只保留变化的分组和叶子字段。
- 如果 patch 为空，前端不再发送 `POST /api/state`。
- 应用成功后会主动刷新一次状态，保证本地缓存和顶部 GPU banner 与服务端一致。

## 设计说明
- 保持向后兼容：后端 `apply_settings` 本来就按“字段是否存在”处理，支持部分 payload。
- 降低耦合：请求大小不再绑定 UI 表单总字段数。
- 从源头解决：在前端构建请求时做裁剪，不在后端打补丁式过滤。

## 手工验证
1. 从托盘打开设置页。
2. 只改 `Theme` 后应用，请求体应只包含 `theme`。
3. 只改一个拖尾数值（如 `k_meteor_rate`）后应用，请求体应只包含对应 `trail_params` 叶子字段。
4. 不做任何改动直接点“应用”，不应发送 `/api/state` 写请求。

