# 拖尾调参 UI（预设 + 高级参数）

## 用户侧效果
设置窗口（非 background 模式）里，“拖尾”这一行新增 **高级调参...** 按钮：
- 打开独立的“拖尾调参”窗口；
- 支持预设一键切换 + 手动微调；
- 写入 `config.json` 并立即生效（无需重启）。

## 预设
内置预设：
- `Default`
- `Snappy`
- `Long`
- `Cinematic`
- `Custom`（手动改过任何参数后建议用这个）

预设标签会保存到 `trail_style`。

## 暴露的参数
### 历史窗口（按拖尾类型）
- `duration_ms`：点位保留时长
- `max_points`：点位数量/密度

### Renderer 参数
- Streamer：`glow_width_scale`、`head_power`
- Electric：`fork_chance`、`amplitude_scale`
- Meteor：`spark_rate_scale`、`spark_speed_scale`

## 代码落点
- UI：`MFCMouseEffect/UI/Settings/TrailTuningWnd.cpp`
- 后端：`MFCMouseEffect/Settings/SettingsBackend.cpp`
- 配置结构：`docs/architecture/trail-profiles-config.zh-CN.md`

