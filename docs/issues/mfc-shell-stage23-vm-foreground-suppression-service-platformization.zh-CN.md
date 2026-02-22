# MFC 壳解耦阶段 23：VM 前台抑制服务平台化（Win32 首落地）

## 1. 背景与目标

阶段 22 已收敛 `AppController` 的 Win32 类型暴露，但 VM 前台检测仍以 `VmForegroundDetector`（Core 头文件内联 Win32 API）存在，导致：

- Core 继续承载平台实现细节；
- 控制层对“前台是否应抑制特效”的能力无法按平台替换。

本阶段目标：

- 将“前台抑制判定”抽象为平台服务；
- 迁移 Win32 实现到 `Platform/windows/System`；
- 删除 Core 内旧的 Win32 检测实现。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- VM 抑制策略规则保持一致（进程名/类名/窗口标题 token 规则未改）；
- 仅改变服务实现位置与依赖方向。

## 3. 实施内容

### 3.1 Core 新增抑制服务接口与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/System/IForegroundSuppressionService.h`
- `MFCMouseEffect/MouseFx/Core/System/NullForegroundSuppressionService.h`

接口能力：

- `ShouldSuppress(uint64_t nowTickMs)`：按当前时间返回是否抑制特效。

### 3.2 Win32 平台实现下沉

新增：

- `MFCMouseEffect/Platform/windows/System/Win32VmForegroundSuppressionService.h`
- `MFCMouseEffect/Platform/windows/System/Win32VmForegroundSuppressionService.cpp`

实现要点：

- 查询前台窗口进程名、窗口类名、标题文本；
- 采用原有 VM token 集合匹配；
- 保留 800ms 缓存窗口，减少重复系统调用。

### 3.3 平台系统工厂扩展

修改：

- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.cpp`

新增工厂：

- `CreateForegroundSuppressionService()`

策略：

- Windows 返回 `Win32VmForegroundSuppressionService`；
- 非 Windows 返回 `NullForegroundSuppressionService`。

### 3.4 AppController 注入替换

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- `AppController` 新增 `foregroundSuppressionService_`；
- `UpdateVmSuppressionState()` 改用注入服务；
- 去除对 `VmForegroundDetector` 的直接依赖。

### 3.5 旧实现移除

删除：

- `MFCMouseEffect/MouseFx/Core/System/VmForegroundDetector.h`

### 3.6 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增项：

- `IForegroundSuppressionService`
- `NullForegroundSuppressionService`
- `Win32VmForegroundSuppressionService(.h/.cpp)`

## 4. 验证

构建验证（VS 2026 MSBuild，amd64 路径）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- VM token 与判定顺序保持一致；
- 缓存时间窗保持原值，避免行为突变。

当前边界：

- 仅迁移了 VM 前台抑制能力；
- Core 仍有其他 Win32 残留点（如 `OverlayCoordSpace`、`KeyChord`、部分 Renderer）。

## 6. 后续建议（阶段 24）

- 平台化 `OverlayCoordSpace`（移除 Core 对 `HWND` 的接口暴露）；
- 继续拆分 `KeyChord/KeyboardInjector` 的 Win32 键盘注入实现；
- 针对 VM 抑制路径补一组最小回归脚本（非 VM 前台、VM 前台、切换边界）。
