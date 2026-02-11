# Dawn Ripple Kind Packet 分支（Stage 76）

## 目标

进一步加快 GPU 接入，把 `ripple` 的三类语义（click / hover / hold）拆分为独立 packet submit 通道，避免长期共用同一条 `ripple_pass` 语义。

## 调整

1. Runtime 新增 API
- `TrySubmitRippleClickBakedPacket(...)`
- `TrySubmitRippleHoverBakedPacket(...)`
- `TrySubmitRippleHoldBakedPacket(...)`

内部复用统一 helper：`TrySubmitRipplePacketByTag(...)`，按 tag 区分：
- `ripple_click_pass`
- `ripple_hover_pass`
- `ripple_hold_pass`

2. CommandConsumer 路由增强
- 在 `nonTrailRippleOnly` 场景下，按命令类型占比选择专用 packet：
  - click-only -> click packet
  - hover-only -> hover packet
  - hold-only -> hold packet
  - 其余混合 -> generic ripple packet

3. 统计增强
- 新增计数字段：
  - `rippleClickPacketSubmitAttempts/success`
  - `rippleHoverPacketSubmitAttempts/success`
  - `rippleHoldPacketSubmitAttempts/success`

4. Web 状态输出增强
- `/api/state` 同步增加：
  - `ripple_click_packet_submit_attempts/success`
  - `ripple_hover_packet_submit_attempts/success`
  - `ripple_hold_packet_submit_attempts/success`

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
