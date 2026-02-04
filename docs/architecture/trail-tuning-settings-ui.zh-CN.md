# 拖尾调参 UI（预设 + 高级参数）

## 状态（2026-02）
该 MFC “拖尾调参”窗口目前已定位为 **遗留/不推荐入口**。托盘菜单的 **设置...** 已默认改为打开浏览器设置页：
- `docs/architecture/web-settings-ui.zh-CN.md`

## 用户侧效果
设置窗口（非 background 模式）里，“拖尾”这一行新增 **高级调参...** 按钮：
- 打开独立的“拖尾调参”窗口；
- 支持预设一键切换 + 手动微调；
- 写入 `config.json` 并立即生效（无需重启）。
 - 窗口复用了主设置页的自绘标题区风格（不再是默认 Caption 皮肤）。

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
- Streamer：`glow_width_scale`、`core_width_scale`、`head_power`
- Electric：`fork_chance`、`amplitude_scale`
- Meteor：`spark_rate_scale`、`spark_speed_scale`

## 代码落点
- UI（遗留）：`MFCMouseEffect/UI/Settings/TrailTuningWnd.Core.cpp`
- UI 标题区/外框（遗留）：`MFCMouseEffect/UI/Settings/TrailTuningWnd.Chrome.cpp`
- UI 模型（遗留）：`MFCMouseEffect/UI/Settings/TrailTuningWnd.Model.cpp`
- 后端：`MFCMouseEffect/Settings/SettingsBackend.cpp`
- 配置结构：`docs/architecture/trail-profiles-config.zh-CN.md`
