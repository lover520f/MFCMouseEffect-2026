# MFC 壳解耦阶段 21：Foreground Process Service 平台化（Win32 首落地）

## 1. 背景与目标

阶段 20 已将时间源从 Core 中收敛，但前台进程解析仍残留在 Core：

- `InputAutomationEngine` 直接持有 `ForegroundProcessResolver`；
- `WebSettingsServer` 直接创建 resolver 查询活动进程；
- resolver 内部包含 Win32 API（窗口与进程查询），导致 Core 继续承担平台职责。

本阶段目标：

- 抽象前台进程解析能力为 Core 接口；
- 将 Win32 实现下沉到 `Platform/windows/System`；
- 统一由平台工厂创建服务，控制层和 Web API 通过注入使用。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 自动化映射规则、作用域匹配和 Web API 协议保持不变；
- 仅替换前台进程名来源路径与依赖方向。

## 3. 实施内容

### 3.1 Core 新增前台进程服务接口与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/System/IForegroundProcessService.h`
- `MFCMouseEffect/MouseFx/Core/System/NullForegroundProcessService.h`

能力：

- `CurrentProcessBaseName()` 返回当前前台进程基础名（小写、去空白）。

### 3.2 Win32 平台实现下沉

新增：

- `MFCMouseEffect/Platform/windows/System/Win32ForegroundProcessService.h`
- `MFCMouseEffect/Platform/windows/System/Win32ForegroundProcessService.cpp`

实现要点：

- 使用 `GetForegroundWindow + GetWindowThreadProcessId + QueryFullProcessImageNameW` 解析前台进程；
- 保留 200ms 缓存窗口，避免每次输入事件都触发进程查询；
- 通过互斥保护缓存状态，避免输入线程与 Web API 线程并发读取时的数据竞争；
- 输出统一做 `Utf16 -> UTF-8`、`trim`、`lower`。

### 3.3 平台工厂扩展 System 服务

修改：

- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.cpp`

新增工厂：

- `CreateForegroundProcessService()`

策略：

- Windows 返回 `Win32ForegroundProcessService`；
- 非 Windows 返回 `NullForegroundProcessService`。

### 3.4 控制层与自动化引擎改为依赖注入

修改：

- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- `InputAutomationEngine` 不再持有 Win32 resolver，实现改为注入 `IForegroundProcessService*`；
- `AppController` 新增 `foregroundProcessService_`，统一通过平台工厂创建并注入自动化引擎；
- `AppController` 新增 `CurrentForegroundProcessBaseName()`，供上层 API 复用。

### 3.5 WebSettingsServer 改为复用 AppController 服务

修改：

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`

关键变更：

- `/api/automation/active-process` 不再直接 new resolver，改为走 `controller_->CurrentForegroundProcessBaseName()`；
- 自动化应用目录缓存时间源改为 `std::chrono::steady_clock`，去掉 `GetTickCount64` 直接依赖。

### 3.6 旧实现清理与工程文件同步

删除：

- `MFCMouseEffect/MouseFx/Core/System/ForegroundProcessResolver.h`

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

同步新增项：

- `IForegroundProcessService`
- `NullForegroundProcessService`
- `Win32ForegroundProcessService(.h/.cpp)`

## 4. 验证

构建验证（VS 2026 MSBuild，amd64 路径）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

说明：

- `Release|x64` 首次链接出现 `LNK1104`（目标 exe 被占用），已自动结束 `MFCMouseEffect.exe` 后重试通过。

## 5. 风险与边界

已控制：

- 输入自动化与手势逻辑未变，仅替换进程名查询来源；
- 非 Windows 端有 `NullForegroundProcessService` 兜底，不阻断编译。

当前边界：

- 前台进程查询能力目前仅 Win32 有实装；
- 非 Windows 平台 `app scope` 的 `process:*` 规则在当前阶段默认无法命中具体进程名。

## 6. 后续建议（阶段 22）

- 为 macOS/Linux 补齐 `ForegroundProcessService` 真正实现（对齐 `process:*` 自动化行为）；
- 将系统信息类能力（前台进程、活动窗口、会话状态）进一步收敛为 `Platform/System` 子模块统一注册点；
- 增加一组自动化作用域回归用例，覆盖 `global + process:* + chain trigger` 的优先级路径。
