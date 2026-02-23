# Stage58 - Legacy MFC UI 归档出主树

## 判定

`架构尾项`：`MFCMouseEffect/UI` 下仍保留 27 个历史 MFC Doc/View 与设置窗口文件，这批代码已不在当前构建链中，但仍位于主源目录，容易与当前平台化架构混淆。

## 目标

1. 将未参与构建的 legacy UI 代码从主源目录隔离到归档区。
2. 保留历史实现（不删除），避免知识丢失。
3. 让主树只承载当前生效的架构路径。

## 变更摘要

### 1) 目录归档

移动：

- `MFCMouseEffect/UI` -> `Legacy/windows-mfc-ui/UI`

说明：

- 仅路径迁移，不修改该批源码内容。
- 当前 `MFCMouseEffect.vcxproj` 不包含这些文件，迁移不影响构建输入。

### 2) 归档说明文档

新增：

- `Legacy/windows-mfc-ui/README.md`

内容：

1. 说明该目录为历史 MFC UI 归档。
2. 指明当前生效路径为 `Platform/windows` + `MouseFx/Core` 架构。
3. 避免后续误把归档代码当作活动实现继续耦合。

## 验证

1. `Release|x64` 构建通过：`0 error / 0 warning`。
2. 边界守卫脚本通过：
   - `tools/architecture/Check-NonPlatformWin32Boundary.ps1`
   - 输出：`OK: no direct Win32/MFC boundary violations in non-Platform compile units.`

## 收益

1. 主代码树进一步收敛到当前平台化架构，降低维护噪音。
2. Windows 迁移余量中的“legacy UI 物理残留”已清零（历史代码保留在归档区）。
