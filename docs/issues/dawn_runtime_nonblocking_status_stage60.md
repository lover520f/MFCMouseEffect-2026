# Dawn runtime 状态无阻塞快照（Stage 60）

## 目的

降低 Dawn 初始化阶段对渲染/输入热路径的锁阻塞，缓解启动与 `cpu/auto -> dawn` 切换初期的鼠标卡顿。

## 背景

- `TryInitializeDawnRuntime()` 在后台线程执行时会持有 runtime 全局互斥锁较长时间。
- 渲染 tick 热路径每帧读取 runtime 状态，若直接阻塞等待该锁，会把卡顿放大到鼠标交互。

## 调整

1. 在 `DawnRuntime` 增加快照缓存与无阻塞查询：
- 新增 `GetDawnRuntimeStatusFast()`。
- 先 `try_lock` runtime 互斥锁：
  - 拿到锁：生成最新状态并更新快照。
  - 未拿到锁：直接返回最近快照，不阻塞热路径。
2. `OverlayHostWindow` 渲染命令收集路径改为调用 `GetDawnRuntimeStatusFast()`。
3. 纠正 modern ABI 诊断策略判定顺序，`prime=queue_ready` 时正确显示 `modern_queue_ready`。
4. 为兼容 VS/MSVC C++17 的 `atomic_store_explicit(shared_ptr)` 模板推导，快照写入使用显式 `shared_ptr<const DawnRuntimeStatus>` 中间变量。

## 影响

- 后台初始化仍按原逻辑进行，不改变 probe/prime 的行为语义。
- 前台热路径不再因 runtime 初始化持锁而同步阻塞，启动和切换期抖动会进一步收敛。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
