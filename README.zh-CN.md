# MFCMouseEffect / 鼠标点击波纹效果

语言：[English](README.md) | [中文](README.zh-CN.md)

Windows 上的全局鼠标点击波纹特效（类似 `keyviz` 鼠标可视化），基于 MFC + Win32 实现。

## 功能介绍
- 监听全局鼠标点击（左/右/中键）。
- 在点击位置绘制短促的波纹动画。
- 使用可点击穿透的分层窗口，不会拦截用户操作。

## 构建运行
1. 在 Visual Studio 中打开 `F:\language\cpp\code\MFCMouseEffect\MFCMouseEffect.slnx`（或生成的 `.sln`）。
2. 编译并运行 `MFCMouseEffect` 项目（推荐 x64）。
3. 运行后点击任意位置，会看到波纹动画。

## 架构（低耦合）
所有波纹相关逻辑都封装在 `MFCMouseEffect\MouseFx\`，通过一个控制器对外暴露：

- `MFCMouseEffect\MouseFx\AppController.*`
  - 生命周期控制：初始化 GDI+、创建消息窗口、启动/停止全局钩子、维护波纹窗口池。
- `MFCMouseEffect\MouseFx\GlobalMouseHook.*`
  - 安装 `WH_MOUSE_LL` 钩子，并把点击事件发送到消息窗口（钩子回调轻量）。
- `MFCMouseEffect\MouseFx\RippleWindow.*`
  - 每次点击都会创建一个分层窗口，用 GDI+ 渲染波纹、调用 `UpdateLayeredWindow`。
  - `WM_NCHITTEST -> HTTRANSPARENT` 让窗口对点击透明。
- `MFCMouseEffect\MouseFx\RippleWindowPool.*`
  - 小型窗口池，避免频繁分配。
- `MFCMouseEffect\MouseFx\RippleStyle.h`
  - 统一配置时长、尺寸、颜色。

MFC 层只做启动/停止调用：
- `mouseFx_ = std::make_unique<mousefx::AppController>(); mouseFx_->Start();`
- `mouseFx_->Stop();`

## 外观自定义
编辑 `MFCMouseEffect\MouseFx\RippleStyle.h` 和 `MFCMouseEffect\MouseFx\RippleWindow.cpp`：
- 时间：`RippleStyle::durationMs`
- 半径：`startRadius`、`endRadius`
- 窗口大小：`windowSize`
- 颜色：
  - 左键：蓝色
  - 右键：橙色
  - 中键：绿色

## 说明与限制
- 仅在 Windows 上有效（依赖 Win32 钩子 + 分层窗口）。
- 如果目标窗口是管理员/高权限，建议本程序也用相同权限运行，避免钩子失效。

## 文档
详细构建、运行、定制与排查说明：
- English：`docs/README.md`
- 中文：`docs/README.zh-CN.md`
