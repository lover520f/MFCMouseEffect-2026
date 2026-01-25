# 进程单例（Singleton）实现记录

## 问题背景
为了防止多个 `MFCMouseEffect` 进程同时运行导致资源冲突（如全局钩子冲突、托盘图标重复等），需要确保应用程序在同一时间内只有一个实例在运行。

## 解决方案

### 1. 基于命名互斥体（Mutex）的检查
在应用程序启动的最早阶段（`InitInstance`）创建一个全局命名的互斥体。

- **互斥体名称**：`Global\MFCMouseEffect_SingleInstance_Mutex`
- **实现逻辑**：
    - 调用 `CreateMutexW` 尝试创建互斥体。
    - 使用 `GetLastError()` 检查返回码。如果返回 `ERROR_ALREADY_EXISTS`，说明系统已经存在该互斥体，即已有另一个实例正在运行。
    - 若检测到已有实例，当前进程立即返回 `FALSE` 并退出，不显示任何 UI，确保静默处理。

### 2. 资源清理
在应用程序退出（`ExitInstance`）时，确保正确释放并关闭互斥体句柄。

```cpp
if (hMutex_)
{
    ReleaseMutex(hMutex_);
    CloseHandle(hMutex_);
    hMutex_ = nullptr;
}
```

## 相关文件
- [MFCMouseEffect.h](file:///f:/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MFCMouseEffect.h) (定义句柄成员)
- [MFCMouseEffect.cpp](file:///f:/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MFCMouseEffect.cpp) (实现创建与销毁逻辑)

## 验证方法
1. 启动程序，确认其正常运行。
2. 再次尝试从资源管理器或命令行启动程序。
3. 观察到第二个实例立即退出，且不会弹出重复的托盘图标或窗口。
