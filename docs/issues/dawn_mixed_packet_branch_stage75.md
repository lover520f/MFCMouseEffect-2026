# Dawn Mixed Non-Trail Packet 分支（Stage 75）

## 目标

继续推进 Dawn 接入：为 `non-trail mixed (ripple + particle)` 增加专用 packet submit 分支，避免该路径长期落回通用 empty submit。

## 调整

1. Runtime API 新增
- `TrySubmitMixedBakedPacket(uint32_t rippleVertices, uint32_t particleSprites, uint32_t uploadBytes, std::string* detailOut)`
- 当前通过 `mixed_pass` tagged command buffer 承载提交语义。

2. DawnCommandConsumer 增强
- 当 `!trail && nonTrailMixed` 且存在 ripple/particle 预处理结果时，优先调用 `TrySubmitMixedBakedPacket(...)`。
- 新增统计字段：
  - `mixedPacketSubmitAttempts`
  - `mixedPacketSubmitSuccess`
- detail 增补后缀：`_mixed_baked_packet`。

3. Web 诊断 JSON 增补
- `mixed_packet_submit_attempts`
- `mixed_packet_submit_success`

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
