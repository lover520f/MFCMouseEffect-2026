# Stage55 - Non-Platform Win32 边界守卫脚本

## 判定

`架构风险`：随着跨平台迁移推进，若后续改动把 `windows.h/d3d11.h/d2d1.h` 或 MFC token 重新写入 `Platform` 目录之外，边界会被悄悄破坏，需要一个可重复执行的守卫。

## 目标

1. 增加自动化检查脚本，扫描 `vcxproj` 实际编译单元。
2. 发现 `Platform` 目录外的 Win32/MFC 直接依赖时快速失败。
3. 为后续阶段提供可复用的边界回归检测入口。

## 变更摘要

新增：

- `tools/architecture/Check-NonPlatformWin32Boundary.ps1`

检查范围：

1. 读取 `MFCMouseEffect/MFCMouseEffect.vcxproj` 中 `ClCompile` 项。
2. 对非 `Platform\` 编译单元扫描以下 token：
   - `#include <windows.h>`
   - `#include <d3d11.h>`
   - `#include <d2d1.h>`
   - `#include <dcomp.h>`
   - `#include <dxgi...>`
   - `Afx*` / `CWnd`

返回约定：

- 无违规：`exit 0` 并输出 `OK`
- 有违规：列出文件与 token，`exit 1`

## 使用方式

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/architecture/Check-NonPlatformWin32Boundary.ps1
```

## 验证

在当前代码基线执行脚本，输出：

- `OK: no direct Win32/MFC boundary violations in non-Platform compile units.`

## 收益

1. 将“平台边界”从口头约定转成可执行检查。
2. 降低后续重构中边界回退的概率，便于持续推进到 macOS/Linux 分包实现。
