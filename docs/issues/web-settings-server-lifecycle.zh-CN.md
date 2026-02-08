# 浏览器设置页：Server 生命周期问题与 Token 轮换（2026-02-04）

## 现象
- 迁移到浏览器设置页后，托盘 **设置...** 偶发导致进程退出。
- 多次打开设置页后，**多个旧 token 依然有效**；期望是 **只有最新 token 生效**。

## 复现（常见流程）
1. 托盘 → **设置...** 启动本地服务。
2. 放置一段时间，触发 idle 自动停止。
3. 再次点击托盘 **设置...** → 偶发崩溃退出。
4. 多次打开设置页 → 旧页面仍能正常调用 API。

## 根因
1. **Idle 停止后 monitor 线程未 join**
   - `StartMonitor()` 创建 `monitorThread_`。
   - idle 超时后，monitor 线程调用 `http_->Stop()` 并退出，但线程对象仍是 joinable。
   - 下次 `StartMonitor()` 重新赋值 `monitorThread_` 时触发 `std::terminate()` → 进程退出。
2. **token 从未轮换**
   - `token_` 只在构造时生成，之后一直复用，导致旧 URL 全部有效。
3. **跨线程读取 config**
   - `WebSettingsServer` 在 HTTP 线程直接读取 `AppController::config_`，与 UI 线程写入存在数据竞争。

## 修复
- `StartMonitor()` 启动新线程前，先 join 旧的 `monitorThread_`。
- 新增 `AppController::GetConfigSnapshot()`（通过 `WM_MFX_GET_CONFIG` 在 dispatch 线程拷贝配置），Web JSON 构建只用快照。
- 增加 token 互斥锁 + `RotateToken()`，并在每次托盘打开设置页时轮换 token。

## 行为变化
- 每次点击托盘 **设置...** 都会生成新 token。
- 旧页面会出现 `unauthorized`，需要从托盘重新打开。
- Ready/服务已关闭/token 失效/错误等状态信息只在左上角提示条显示（底部 toast 移除）。

## 2026-02-08 补充：断连状态同步
- 问题：页面初始化成功后会显示“就绪”，但如果服务后来被关闭或连接中断，左上角可能仍停留在“就绪”，容易让用户误判为 UI bug。
- 修复：
  - WebUI 增加轻量健康检查（`/api/state`，3 秒轮询，切回标签页时立即探测）。
  - 状态机新增 `offline`，连接断开后切换为“已与本地服务断开连接，请从托盘重新打开”。
  - `Stop` 成功后立即切到 `stopped`；依赖服务的按钮保持可点击，但会在断连状态下弹窗提示并中止请求。
  - 断连时使用更明显的横幅样式（红色告警基调），与 `Ready` 明确区分。

## 2026-02-08 补充2：断连后按钮点击弹窗
- 需求：用户在断连状态下点击“应用”等按钮时，需要明确弹窗提示，避免误以为按钮无响应或 UI 异常。
- 实现：
  - 新增统一拦截器 `blockActionWhenDisconnected()`。
  - 对 `重载/应用/恢复默认/关闭监听` 四个按钮，在发送请求前先检查连接状态。
  - 若状态为 `offline/stopped/unauthorized`，立即弹窗提示并中止请求。
  - 同时保留左上角横幅，形成“横幅 + 弹窗”双重反馈。
  - 弹窗从浏览器原生 `alert` 改为 WebUI 主题弹窗（`dialog.js`），视觉风格与设置页一致。

## 手动测试
- 等待 idle 超时后再次打开设置页 → 不再崩溃。
- 连续打开两次设置页 → 旧页 API 失败（unauthorized），新页正常。
- 在最新页面应用设置 → 立即生效并写回 `config.json`。
