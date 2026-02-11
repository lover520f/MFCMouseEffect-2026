# Dawn Trail Packet Submit 分支（Stage 72）

## 目标

继续推进 Dawn 接入：在命令消费路径中给 `trail` 增加独立 packet submit 分支，和 ripple 分支对齐，便于后续逐步替换为真实几何上传/绘制。

## 调整

1. 新增 Runtime API
- `TrySubmitTrailBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut)`
- 语义与 ripple packet 对齐：当前先走 tagged empty command buffer（`trail_pass`），并输出可诊断 detail。

2. DawnCommandConsumer 增强
- 当存在 trail geometry 时，优先调用 `TrySubmitTrailBakedPacket(...)`。
- 新增统计字段：
  - `trailPacketSubmitAttempts`
  - `trailPacketSubmitSuccess`
- detail 增补后缀：`_trail_baked_packet`。

3. Web 诊断 JSON 增补
- `trail_packet_submit_attempts`
- `trail_packet_submit_success`

## 附带修复

- 新增 `.gitattributes`，对 `MFCMouseEffect/WebUI/*.{js,css,html}` 固定 `CRLF`。
- 规范化 `MFCMouseEffect/WebUI/app.js` 行尾为 `CRLF`，避免 VS 行尾不一致弹窗。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `.gitattributes`
- `MFCMouseEffect/WebUI/app.js`
