# Web 设置页阶段 4.3：网络访问与健康检查模块抽离（`app.js` 收敛）

## 背景
阶段 4.2 后，`app.js` 仍承担 API 调用细节、401 处理和心跳探测逻辑，入口文件还包含明显的“通信层实现细节”。

本阶段目标是把网络访问和健康检查抽到独立模块，`app.js` 仅保留页面编排职责。

## 本次改动

### 1. 新增 Web API 模块
- 新文件：`MFCMouseEffect/WebUI/web-api.js`
- 对外 API：
  - `window.MfxWebApi.create(options)`
- 实例能力：
  - `apiGet(path)`
  - `apiPost(path, payload)`
  - `probeConnection()`
  - `startHealthCheck()`
  - `stopHealthCheck()`
- 设计：
  - 统一注入 token 请求头。
  - 401 场景统一抛出带 `code='unauthorized'` 的错误。
  - 心跳探测通过回调上报 `online/offline/unauthorized` 状态。

### 2. 页面接入
- 文件：`MFCMouseEffect/WebUI/index.html`
- 变更：
  - 在 `app.js` 前新增 `<script src="/web-api.js"></script>`

### 3. `app.js` 责任收敛
- 文件：`MFCMouseEffect/WebUI/app.js`
- 变更：
  - 删除内联 `fetch` 细节、401 解析、健康检查定时器代码。
  - 通过 `MfxWebApi.create(...)` 创建客户端并注入：
    - `token`
    - `onUnauthorized`（映射到现有 `showUnauthorized`）
    - `onConnectionState`（映射到现有 `markConnection`）
  - 保留 `reload/apply/reset/stop` 的业务编排不变。

## 兼容性
- 不改后端接口和 payload。
- 前端状态提示语义与阶段 4.2 保持一致。
- 连接状态与按钮可用性策略不变。

## 验证记录
1. `node --check` 通过：
   - `MFCMouseEffect/WebUI/web-api.js`
   - `MFCMouseEffect/WebUI/app.js`
2. `MSBuild x64 Debug` 通过，PostBuild 日志确认 `web-api.js` 已复制到 `x64/Debug/webui`。
3. 手动流程回归点：
   - reload / apply / reset / stop 的状态更新与错误处理保持一致。
   - 连接探测仍会在恢复可见时触发即时探测。

## 后续（阶段 4.4 建议）
1. 继续拆分 `app.js` 的“按钮动作处理器”，把 `reload/apply/reset/stop` 事件绑定收敛到独立模块。
2. 在不改协议的前提下，为页面编排层补充更清晰的初始化顺序约束（依赖检查 + 降级提示）。
