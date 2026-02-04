# Web 设置页（Loopback HTTP）

## 背景
MFC 的设置界面在“高级调参”（尤其是拖尾参数）场景下：
- 很难做到好看、可扩展
- 布局/字体/DPI 容易出现截断与对齐问题
- UI 代码容易和渲染逻辑耦合

因此本项目现在更偏向使用 **浏览器页面** 作为设置入口：由程序内置一个 **本地 loopback HTTP server** 提供页面与 API。

## 使用流程
1. 托盘菜单 → **设置...**
2. 程序启动本地服务器，绑定 `127.0.0.1` 的随机端口
3. 自动打开默认浏览器：`http://127.0.0.1:<port>/?token=<token>`
4. 页面通过 `/api/*` 读取/写入配置，保存后立即生效

## 应用方式
- 目前为 **手动应用**：修改参数后点击 **应用**。
- **重载** 会从磁盘重新读取 `config.json`。
- **恢复默认** 会重置为默认值（随后刷新）。
- **关闭监听** 会停止本地服务器（需要从托盘重新打开）。
## 交互提示
- 顶部按钮提供 hover 提示（Reload / Apply / Star）。
- 文本内容提示使用英文逗号分隔。
- 拖尾调参新增 **停留淡出开始/结束(ms)**，控制鼠标停住后的收敛速度。

## 安全说明
- 只绑定 **loopback**（`127.0.0.1`），不对外网暴露。
- `/api/*` 必须携带 `X-MFCMouseEffect-Token`，并与 URL 中的 `token` 一致。
  - 这是一个轻量防护，主要用于避免“本机任意网页/脚本”对 loopback 发起 CSRF 调用（比如你打开了某个网页，它可以尝试请求 `http://127.0.0.1:<port>/api/*`）。

## 打包与热更新（磁盘覆盖优先）
- 源码 UI 位于：`MFCMouseEffect/WebUI/`
- 编译后会拷贝到：`$(OutDir)\\webui\\`（例如 `x64\\Release\\webui\\`）
- 服务器优先读取磁盘文件，所以直接修改 `webui\\` 下的文件并刷新浏览器即可更新 UI（无需重新编译）
- 若 `webui\\` 缺失，则会回退到内置 RCDATA（核心文件）

## API 列表
- `GET /`：静态页面（`/index.html`），优先从 `$(OutDir)\\webui\\` 读取（磁盘可覆盖），否则走 RCDATA fallback
- `GET /app.js`、`GET /styles.css`：静态资源（同上规则）
- `GET /api/schema`：下拉选项元数据（主题、各分类特效列表等）
- `GET /api/state`：当前配置（语言/主题/各分类启用项 + 拖尾调参）
- `POST /api/state`：应用配置（内部转换为 `{"cmd":"apply_settings","payload":...}`）
- `POST /api/reload`：从磁盘重载 `config.json`（内部转换为 `{"cmd":"reload_config"}`）
- `POST /api/reset`：恢复默认（内部转换为 `{"cmd":"reset_config"}`）
- `POST /api/stop`：关闭本地服务器（按需启动）

## 资源占用
- 服务器线程在 `accept()` 阻塞等待（不是轮询）。
- 另外有空闲超时自动停止，以减少后台占用。

## 关键实现位置
- 服务器：`MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- HTTP 主循环：`MFCMouseEffect/MouseFx/Server/HttpServer.cpp`
- 静态页面与资源：`MFCMouseEffect/WebUI/`（PostBuild 拷贝到 `$(OutDir)\\webui\\`）
- 内置 fallback：`MFCMouseEffect/res/MFCMouseEffect.rc2`（RCDATA）
- 应用设置：`MFCMouseEffect/MouseFx/Core/AppController.cpp`（处理 `apply_settings`）

## 手工验证清单
- 托盘 → 设置，浏览器能打开并正常加载
- Save & Apply 后立刻生效并写回 `config.json`：
  - 语言/主题
  - 点击/拖尾/滚轮/长按/悬停 的启用项
  - 文本内容（逗号分隔）
  - 拖尾 profile 与 renderer 参数
- Reload 按钮可以从磁盘重载并刷新页面

## 常见问题排查
- **下拉框为空 / 页面提示 “Load failed”：** 打开浏览器开发者工具（Network），查看 `/api/schema`、`/api/state` 的返回。
  - Unauthorized：通常是 URL 缺少 `?token=...` 或请求头未携带 token。
  - 500：服务器会返回具体错误字符串，把它贴出来便于定位。
- **编码错误（`invalid UTF-8 byte`）**：服务端已加 UTF-8 校验与 ACP 转换兜底。
