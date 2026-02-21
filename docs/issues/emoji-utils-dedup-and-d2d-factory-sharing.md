# Emoji 工具函数去重 + D2D Factory 共享

日期：2026-02-21

## 变更概述

### 1. Emoji 工具函数集中化

**问题**：`NextCodePoint`、`IsEmojiCodePoint`、`HasEmojiStarter`、`IsEmojiOnlyText` 等 Emoji
检测工具函数在 5 个文件中存在完全相同的本地拷贝，修改一处其余不会同步更新。

**方案**：扩展已有的 `Settings/EmojiUtils.h/cpp`，补充 `HasEmojiStarter` 和 `IsEmojiOnlyText`，
然后删除以下 4 个文件中的本地重复，统一使用 `settings::` 命名空间：

| 文件 | 删除的本地函数 |
|------|--------------|
| `MouseFx/Windows/TextWindow.cpp` | `NextCodePoint`, `IsEmojiCodePoint`, `IsEmojiComponent`, `IsEmojiOnlyText`, `HasEmojiStarter` |
| `MouseFx/Layers/TextOverlayLayer.cpp` + `.h` | `NextCodePoint`, `IsEmojiCodePoint`, `HasEmojiStarter`, `IsEmojiOnlyText`（成员函数） |
| `MouseFx/Effects/TextEffect.cpp` | `NextCodePoint`, `IsEmojiCodePoint`, `HasEmojiStarter` |
| `UI/Settings/EmojiPreviewWnd.cpp` | `NextCodePoint`, `IsEmojiCodePoint`, `IsEmojiComponent` |

`OverlayHostService.cpp` 已经在使用 `settings::` 版本，无需修改。

### 2. D2D1/DWrite Factory 共享

**问题**：每个 `TextWindow` 实例在 `EnsureD2DResources()` 中独立调用
`D2D1CreateFactory` 和 `DWriteCreateFactory`，池化 8 个窗口就创建 8 套 Factory。

**方案**：新建 `MouseFx/Utils/D2DFactory.h/cpp`，使用 C++11 magic static 提供
进程级单例 Factory。`TextWindow` 改为调用 `SharedD2D1Factory()` / `SharedDWriteFactory()`。

**线程安全**：
- C++11 保证 `static` 局部变量初始化线程安全
- 使用 `D2D1_FACTORY_TYPE_SINGLE_THREADED`（与原有行为一致），无内部锁开销
- 所有 `TextWindow` 都通过 `WM_TIMER` 在 UI 线程渲染，无并发风险
- 不存在死锁可能

## 涉及文件

### 新增
- `MouseFx/Utils/D2DFactory.h` — 共享 D2D1/DWrite Factory 声明
- `MouseFx/Utils/D2DFactory.cpp` — 实现

### 修改
- `Settings/EmojiUtils.h/cpp` — 添加 `HasEmojiStarter`、`IsEmojiOnlyText`
- `MouseFx/Windows/TextWindow.h` — 移除 `d2dFactory_`、`dwriteFactory_` 成员
- `MouseFx/Windows/TextWindow.cpp` — 使用共享 Factory，清理 emoji 重复
- `MouseFx/Layers/TextOverlayLayer.h/cpp` — 清理 emoji 重复
- `MouseFx/Effects/TextEffect.cpp` — 清理 emoji 重复
- `UI/Settings/EmojiPreviewWnd.cpp` — 清理 emoji 重复
- `MFCMouseEffect.vcxproj` — 注册新文件
