# Web 设置页诊断瘦身与本地文件导出

## 问题
- 设置页诊断信息过重：
- banner 每次刷新拼接大量运行时字段。
- 诊断模式几乎常开且轮询频率高。
- 大 JSON 常驻页面，导致设置页卡顿。

## 改动
- `MFCMouseEffect/MouseFx/Server/LocalDiagStateWriter.h`
- `MFCMouseEffect/MouseFx/Server/LocalDiagStateWriter.cpp`
- 将本地状态快照写入逻辑独立成专用写入器，`WebSettingsServer` 仅负责 HTTP/schema/state 组装。
- `MFCMouseEffect/WebUI/app.js`
- 恢复诊断开关：仅当 URL 带 `diag=1` 时启用诊断。
- 诊断轮询间隔由 `500ms` 调整为 `2500ms`。
- 诊断流内存上限由 `280` 行降到 `120` 行。
- 顶部 banner 只保留必要诊断字段（命令总量 + queue/encoder 就绪）。
- 去掉导出相关 UI 交互，避免多余流程。
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- 在 `BuildStateJson()` 链路增加自动本地落盘（带节流）。
- EXE 直接把当前状态写到：
- `.<exe_dir>/.local/diag/web_state_auto.json`
- 不新增任何调试 API 端点。
- `MFCMouseEffect/WebUI/index.html`
- 移除状态导出按钮和对应文本区。
- `.gitignore`
- 增加 `/.local/`，导出文件不进 git。

## 为什么能缓解卡顿
- 普通设置模式不再渲染重诊断内容。
- 诊断模式下也降低了刷新频率和字符串拼接量。
- 全量状态由 EXE 直接生成到本地文件，排查时直接读取即可。

## 手工验证
1. 不带 `diag=1` 打开设置页：不显示 diag 面板，且无高频诊断轮询。
2. 带 `diag=1` 打开设置页：diag 面板出现，刷新内容明显更轻。
3. 打开设置页并执行一次重载/刷新状态流程。
4. 在 exe 目录下确认文件存在：`.local/diag/web_state_auto.json`。
