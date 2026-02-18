# WASM 文本 Emoji 渲染回退统一（2026-02）

## 背景
- 用户在 WASM 插件里输出 `中文 + emoji` 文本时，emoji 显示为方框（豆腐块）。
- 内置点击文字特效在 emoji 场景通常可正常显示，表现不一致。

## 根因
- `WASM spawn_text` 通过 `OverlayHostService::ShowText` 进入 `TextOverlayLayer`（GDI+ 文本层）。
- 内置点击文字特效在检测到 emoji 时，会转到 `TextWindowPool`（DirectWrite 路径，带更可靠字体回退）。
- 两条路径不一致，导致 WASM 文本没有走 emoji 友好渲染链。

## 修复方案
- 在 `OverlayHostService::ShowText` 增加统一路由：
  - 文本包含 emoji 时，直接使用共享 `TextWindowPool` 渲染；
  - 非 emoji 文本继续走 `TextOverlayLayer`。
- 在 `OverlayHostService::Shutdown` 中同步清理共享 `TextWindowPool`，避免资源残留。

## 影响文件
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

## 验证建议
1. 在 WASM 插件中输出包含 emoji 的文本（如 `财富 + 😄 + 🪙`）。
2. 点击触发后确认 emoji 不再是方框。
3. 再验证普通文本输出与内置点击文字效果无回归。

