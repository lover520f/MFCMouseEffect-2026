# Dawn 诊断补充 Ripple 子类型计数（Stage 64）

## 目的

在 `diag=1` 场景下，避免仅依赖 `detail` 字符串判断 ripple 类型；直接提供 click/hover/hold 三类计数，便于快速核对命令流语义是否正确。

## 调整

1. 后端 JSON 增加三个字段：
- `ripple_click_commands`
- `ripple_hover_commands`
- `ripple_hold_commands`

覆盖接口：
- `/api/state`
- `/api/gpu/probe_now`
- `/api/gpu/bridge_mode`

2. 前端诊断日志与 banner 文案补充显示：
- 诊断行新增 `ripple(c/h/hd)=x/y/z`
- banner 在 consumer 段新增 `Ripple kinds` 摘要

## 影响

- 诊断时可直接确认是 click ripple 还是 hover/hold continuous 在驱动 non-trail 提交。
- 后续做 hold/hover 全量 Dawn 路由时，验证链路更直接。

## 变更文件

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `MFCMouseEffect/WebUI/app.js`
