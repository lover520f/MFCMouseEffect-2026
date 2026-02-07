# OverlayHost Phase 6: Legacy Ripple Window 清理

## 1. 目标

在 Phase 5 已完成 ripple 家族 Host-only 后，删除已不可达的旧窗口实现，避免后续误用和重复维护。

## 2. 清理内容

### 2.1 删除旧实现文件

- `MouseFx/Windows/RippleWindow.h`
- `MouseFx/Windows/RippleWindow.cpp`
- `MouseFx/Windows/RippleWindowPool.h`
- `MouseFx/Windows/RippleWindowPool.cpp`

### 2.2 更新工程文件

- 从 `MFCMouseEffect.vcxproj` 移除上述头/源文件条目
- 从 `MFCMouseEffect.vcxproj.filters` 移除对应过滤器条目

### 2.3 注释同步

- `MFCMouseEffect.cpp` 中 DPI 注释从 `RippleWindow` 改为 `Overlay host layers`
- `TubesHoverRenderer.h` 中时间基准注释去除 `RippleWindow` 表述

## 3. 行为边界

- ripple/scroll/hold/hover/star 均已走 `OverlayHostWindow` 路径
- text 仍保留 emoji 回退到 `TextWindowPool`（既有策略，不在本阶段改动）

## 4. 验证

- `MSBuild /t:ClCompile /p:Configuration=Release /p:Platform=x64` 通过
- 编译告警仅为既有项：`EffectConfig.cpp` 中未使用局部变量 `e`
