# config.json 的拖尾 Profile（支持热重载）

## 目标
让高级用户无需重新编译即可调参（主要用于策略型拖尾：`line / streamer / electric / meteor / tubes`）：
- 拖尾点位保留时长：`duration_ms`
- 点位历史密度：`max_points`

并且通过 IPC/托盘命令在运行时热应用。

## JSON 结构
根节点新增可选字段：

### `trail_style`
预设名称标签：
- `default | snappy | long | cinematic | custom`

### `trail_profiles`
每个类型的历史窗口（点位保留时长/点数）：

```json
{
  "trail_style": "default",
  "trail_profiles": {
    "line":     { "duration_ms": 300, "max_points": 32 },
    "streamer": { "duration_ms": 420, "max_points": 46 },
    "electric": { "duration_ms": 280, "max_points": 24 },
    "meteor":   { "duration_ms": 520, "max_points": 60 },
    "tubes":    { "duration_ms": 350, "max_points": 40 }
  }
}
```

说明：
- 缺省字段会回落到内置默认值。
- 数值会做安全裁剪：
  - `duration_ms`：限制在 `[80, 2000]`
  - `max_points`：限制在 `[2, 240]`
- `scifi` 拖尾类型会视作 `tubes` 的别名。

### `trail_params`
renderer 专属参数（可选）：

```json
{
  "trail_params": {
    "streamer": {
      "glow_width_scale": 1.8,
      "core_width_scale": 0.55,
      "head_power": 1.6
    },
    "electric": {
      "amplitude_scale": 1.0,
      "fork_chance": 0.10
    },
    "meteor": {
      "spark_rate_scale": 1.0,
      "spark_speed_scale": 1.0
    }
  }
}
```

## 热重载
## 配置文件位置（重要）
- **Release**：优先使用 `%AppData%\\MFCMouseEffect\\config.json`
- **Debug**：使用 `[ExeDir]\\config.json`
详见：`docs/issues/text_encoding_and_path_fix.md`（配置路径与编码说明）。

### 托盘
右键托盘图标 → `重载配置 (Reload config)`。

### 后台模式（stdin JSON）
发送：
```json
{"cmd":"reload_config"}
```

## 代码落点
- 配置解析/保存：`MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- Profile 查询：`MFCMouseEffect/MouseFx/Core/EffectConfig.h`
- TrailEffect 注入 profile：`MFCMouseEffect/MouseFx/Core/EffectFactory.cpp`
- 热重载命令：`MFCMouseEffect/MouseFx/Core/AppController.cpp`
- 托盘菜单入口：`MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
