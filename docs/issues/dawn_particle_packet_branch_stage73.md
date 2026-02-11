# Dawn Particle Packet Submit 分支（Stage 73）

## 目标

继续推进 Dawn 接入，把 `particle` 路径也接入 packet submit 分支，与 `trail/ripple` 对齐，便于后续把粒子真实顶点上传与绘制替换进去。

## 调整

1. Runtime 新增 API
- `TrySubmitParticleBakedPacket(uint32_t bakedSprites, uint32_t uploadBytes, std::string* detailOut)`
- 当前先走 tagged empty command buffer（`particle_pass`），输出可诊断 detail：
  - 成功：`particle_packet_submit_ok_s{sprites}_u{bytes}`
  - 失败：`particle_packet_submit_fail_*`

2. DawnCommandConsumer 增强
- 在“仅粒子非 trail 路径”优先走 `TrySubmitParticleBakedPacket(...)`。
- 新增统计字段：
  - `particlePacketSubmitAttempts`
  - `particlePacketSubmitSuccess`
- detail 增补：`_particle_baked` / `_packet`。

3. Web 诊断 JSON 增补
- `particle_packet_submit_attempts`
- `particle_packet_submit_success`

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
