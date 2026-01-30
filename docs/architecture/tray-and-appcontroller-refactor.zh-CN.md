# Tray/AppController 整理（2026-01-30）

## 背景
在不动 `MouseFx/Renderers/*`（渲染实现基本可用）的前提下，把“胶水代码”集中区做了一次收敛，目标是：
- 降低 `AppController` 的职责密度（把“工具性/工厂性逻辑”抽出去）。
- 托盘菜单改为表驱动，避免大量重复 `AppendMenu + Check + switch(cmd)`。
- 保持行为兼容：托盘仍通过 `AppController::HandleCommand` 走同一条配置持久化链路。

## 变更摘要
### 1) Config 路径解析独立
- 新增：`MFCMouseEffect/MouseFx/ConfigPathResolver.h`
- 新增：`MFCMouseEffect/MouseFx/ConfigPathResolver.cpp`
- `AppController` 不再内嵌 `GetConfigDirectory()`，改为调用 `ResolveConfigDirectory()`。

### 2) Effect 创建逻辑抽出为工厂
- 新增：`MFCMouseEffect/MouseFx/EffectFactory.h`
- 新增：`MFCMouseEffect/MouseFx/EffectFactory.cpp`
- `AppController::CreateEffect(...)` 只做委托：`EffectFactory::Create(...)`。

### 3) 轻量 JSON 字段提取独立
- 新增：`MFCMouseEffect/MouseFx/JsonLite.h`
- 新增：`MFCMouseEffect/MouseFx/JsonLite.cpp`
- `AppController` 不再内嵌 `ExtractJsonValue()`。

### 4) Renderer 静态注册显式落地（不改渲染实现）
渲染器通过头文件里的静态对象进行注册（`REGISTER_RENDERER`）。为了避免“某些头文件没被包含导致未注册”，新增一个专用编译单元：
- 新增：`MFCMouseEffect/MouseFx/Renderers/RendererLinkage.cpp`

### 5) 托盘菜单表驱动 + 命令 ID 统一出口
- 新增：`MFCMouseEffect/TrayMenuCommands.h`（托盘命令 ID 统一定义）
- 新增：`MFCMouseEffect/TrayMenuBuilder.h`
- 新增：`MFCMouseEffect/TrayMenuBuilder.cpp`
- `MFCMouseEffect/TrayHostWnd.cpp` 只负责：
  - 获取 `mouseFx` 指针
  - 通过 `TrayMenuBuilder::BuildTrayMenu(...)` 构建菜单
  - 根据 cmd 做 “Exit/Settings/StarRepo” UI 操作
  - 其余 cmd：由 `TrayMenuBuilder` 生成 JSON / theme，再调用 `AppController`

## 设计原则/模式落地
- **SRP（单一职责）**：路径解析、JSON 提取、Effect 创建、托盘菜单构建分别独立。
- **OCP（开闭）**：托盘菜单新增项主要是追加表项；不需要再复制粘贴多段逻辑。
- **Factory Method / Simple Factory**：`EffectFactory` 集中创建逻辑。
- **Builder（轻量）**：`TrayMenuBuilder` 负责“组装菜单对象”。

## 回归验证（手动）
1. 启动 Release 或 Debug，托盘右键菜单能正常弹出。
2. 切换 Click/Trail/Scroll/Hold/Hover 后，对应效果立即变化。
3. 切换 Theme 后，主题色生效且能持久化到 `config.json`。
4. 退出/打开设置窗口行为保持不变。

