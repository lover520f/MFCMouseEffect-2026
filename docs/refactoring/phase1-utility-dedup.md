# Phase 1: Utility Function Deduplication

## 概述

将分散在 15+ 个源文件中的 20+ 个重复工具函数，统一抽取到 `MouseFx/Utils/` 目录下的集中化头文件中，消除代码重复，遵循 DRY 原则。

## 新增文件

| 文件 | 内容 | 类型 |
|------|------|------|
| `Utils/MathUtils.h` | `ClampInt`, `ClampFloat` | inline header-only |
| `Utils/TimeUtils.h` | `NowMs` | inline header-only |
| `Utils/StringUtils.h` | `TrimAscii`, `ToLowerAscii`, `Utf8ToWString`, `Utf16ToUtf8`, `IsValidUtf8`, `EnsureUtf8` | 声明 |
| `Utils/StringUtils.cpp` | 上述 string 函数实现 | 编译单元 |

## 修改文件清单

### 核心层 (Core)
- **AppController.cpp** — 移除 `TrimAscii`, `ToLowerAscii`, `Utf8ToWString`, `ClampInt`, `ClampFloat` 共 5 个本地定义
- **EffectConfig.cpp** — 移除 `ToLowerAsciiLocal`, `ClampFloat`, `ClampInt` 共 3 个本地定义；统一 `ToLowerAsciiLocal` → `ToLowerAscii`

### 服务层 (Server)
- **WebSettingsServer.h/cpp** — 移除 `TrimAscii`, `Utf16ToUtf8`, `IsValidUtf8`, `EnsureUtf8`, 成员 `NowMs()` 共 5 个定义
- **HttpServer.cpp** — 移除 `ToLowerAscii`, `TrimAscii` 共 2 个定义

### 样式层 (Styles)
- **ThemeStyle.h/cpp** — `ToLowerAscii` 声明和定义迁移至 `StringUtils`，头文件改为 `#include "MouseFx/Utils/StringUtils.h"` 保持向后兼容

### 窗口层 (Windows)
- **TrailWindow.cpp** — 移除 static `NowMs()`
- **TextWindow.cpp** — 移除 static `NowMs()`
- **ParticleTrailWindow.cpp** — 移除 static `NowMs()`
- **OverlayHostWindow.cpp** — 移除 static `NowMs()`

### 效果层 (Effects)
- **ScrollEffect.cpp** — 移除 static `NowMs()`

### 图层 (Layers)
- **RippleOverlayLayer.h/cpp** — 移除成员 `NowMs()` 声明和定义
- **TextOverlayLayer.h/cpp** — 移除成员 `NowMs()` 声明和定义

## 设计决策

1. **MathUtils / TimeUtils 使用 inline**：这些函数体极小（1-3 行），inline 避免调用开销，无需额外 `.cpp`
2. **StringUtils 使用 H/CPP 分离**：字符串函数涉及 Win32 API 调用，体积较大，分离可减少头文件依赖和编译时间
3. **ThemeStyle.h 保留 re-export**：通过 `#include "MouseFx/Utils/StringUtils.h"` 保持对 `ToLowerAscii` 的向后兼容
4. **命名空间**：所有函数放在 `mousefx` 命名空间内，与项目一致

## 验证

- ✅ Release x64 构建：0 错误，0 警告
