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

## 手动测试
- 等待 idle 超时后再次打开设置页 → 不再崩溃。
- 连续打开两次设置页 → 旧页 API 失败（unauthorized），新页正常。
- 在最新页面应用设置 → 立即生效并写回 `config.json`。
