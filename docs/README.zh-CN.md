# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

## 文档索引
- 市场/README 展示：`docs/marketing/readme_language_switch.md`（语言切换文案的文本渲染）
- 问题记录：`docs/issues/emoji-support.md`（设置输入框与文本点击特效的表情渲染）
- 问题记录：`docs/issues/text-effect-motion.md`（切换到 DWrite 后的飘散路径一致性）
- 问题记录：`docs/issues/settings-emoji-preview.md`（设置页表情彩色预览叠层）
- 问题记录：`docs/issues/web-settings-server-lifecycle.zh-CN.md`（浏览器设置页：idle 重启崩溃 + token 轮换）
- 问题记录：`docs/issues/dawn_native_stage14_fluxfield_d2d_ui_switch.md`（FluxField D2D 实验开关接入设置页，不再依赖手工创建门控文件）
- 问题记录：`docs/issues/dawn_native_stage15_fluxfield_d2d_com_error_fuse_and_settings_only_gate.md`（FluxField D2D 路径加入 COM 异常熔断，并移除旧门控干扰）
- 问题记录：`docs/issues/dawn_native_stage16_hold_runtime_diag_write_throttle_fix.md`（移除长按更新阶段高频写盘诊断，修复长按卡顿和 CPU 占用上升）
- 问题记录：`docs/issues/dawn_native_stage17_fluxfield_d2d_binddc_clip_transform_fix.md`（修复 D2D BindDC 裁剪矩形与世界变换不一致导致的“GPU路径完全不可见”）
- 问题记录：`docs/issues/dawn_native_stage18_fluxfield_gpu_v2_cursor_state_anchor_fix.md`（使用 hold_state 光标坐标做 GPU-v2 D2D 定位锚点，修复定位不稳/不可见）
- 问题记录：`docs/issues/dawn_native_stage19_fluxfield_gpu_v2_multisurface_offscreen_guard.md`（为多屏 surface 增加离屏判定和 BindDC 裁剪钳制，避免离屏失败触发 GPU 路径熔断）
- 问题记录：`docs/issues/dawn_native_stage20_fluxfield_gpu_v2_d3d11_compute_pivot.md`（将 FluxField GPU-v2 切换为稳定的 D3D11 计算负载路线，并保留可见叠层反馈）
- 问题记录：`docs/issues/dawn_native_stage21_fluxfield_gpu_v2_visual_decouple.md`（保持 D3D11 GPU 计算链路不变，替换临时圆环为独立 FluxField 视觉渲染器）
- 问题记录：`docs/issues/dawn_native_stage22_neon_gpu_v2_d3d11_compute_route.md`（将 hold_neon3d_gpu_v2 从占位路线升级为真实 D3D11 GPU 计算路径，并保留 NeonHUD3D 视觉）
- 问题记录：`docs/issues/dawn_native_stage23_neon_gpu_v2_full_visual_gpu_path.md`（将 hold_neon3d_gpu_v2 升级为 D2D GPU 可视渲染优先，CPU 仅作安全回退）
- 问题记录：`docs/issues/dawn_native_stage24_neon_gpu_v2_direct_runtime_full_gpu.md`（`hold_neon3d_gpu_v2` 绕过 OverlayHost 的整屏 GDI 循环，改为 D3D11+DComp 直接运行时，优先解决长按性能与延迟）
- 问题记录：`docs/issues/dawn_native_stage25_neon_gpu_v2_direct_runtime_visibility_fix.md`（修复 direct runtime “不卡但无图像”：在线程内初始化 COM，并调整 presenter 窗口样式兼容性）
- 问题记录：`docs/issues/dawn_native_stage26_neon_gpu_v2_render_loop_and_coord_fix.md`（修复“进入 GPU 路径但持续无渲染”：补齐 runtime 线程消息泵，并对长按起点坐标改为优先 GetCursorPos）
- 问题记录：`docs/issues/dawn_native_stage27_neon_gpu_v2_presenter_error_telemetry.md`（为 presenter 增加 API 级失败原因采集，并把详细 reason 回写到 runtime 诊断）
- 问题记录：`docs/issues/dawn_native_stage28_neon_gpu_v2_swapchain_prereq_order_fix.md`（修复 RenderFrame 前置条件顺序错误，确保懒创建 swapchain 在首帧可执行）
- 问题记录：`docs/issues/dawn_native_stage29_neon_gpu_v2_full_d3d11_presenter.md`（将 Neon GPU-v2 presenter 改为纯 D3D11 着色器渲染 + DComp 呈现，移除 `CreateBitmapFromDxgiSurface` 失败链路）
- 问题记录：`docs/issues/dawn_native_stage30_neon_gpu_v2_hardware_adapter_and_alpha_cleanup.md`（优先显式选择硬件适配器创建设备，并收紧 shader 透明覆盖，减少 WARP 探测噪音与整块暗底）
- 问题记录：`docs/issues/dawn_native_stage31_neon_gpu_v2_formal_hud3d_shader.md`（将临时测试圈样式替换为正式 Neon HUD3D 分层 shader 视觉，并保持全 GPU 呈现链路）
- 问题记录：`docs/issues/dawn_native_stage32_neon_gpu_v2_angular_wrap_fix.md`（修复角度包裹数学导致的白色扇形伪像，避免指数项异常爆亮）
- 问题记录：`docs/issues/dawn_native_stage33_quantum_halo_gpu_v2_rename_and_high_fidelity_shader.md`（将 Neon GPU-v2 正式更名为 Quantum Halo GPU-v2，补齐旧 id 兼容别名，并升级为更高复杂度多层 shader 视觉）
- 问题记录：`docs/issues/dawn_native_stage34_gpu_fallback_notice_webui_and_false_positive_fix.md`（移除阻塞式回退弹窗，修复别名映射导致的误判回退提示，并将真实回退信息改为 Web 设置页状态栏提示）
- 问题记录：`docs/issues/dawn_native_stage35_gpu_display_name_normalization.md`（统一设置元数据/托盘/Web 的 GPU 与 CPU 显示命名：GPU 保留 GPU 标识并去掉 v2，CPU 去掉 (CPU) 后缀）
- 问题记录：`docs/issues/dawn_native_stage36_remove_fluxfield_gpu_d2d_experimental_switch.md`（移除已失效的 FluxField GPU D2D 实验开关，清理 UI/配置/运行时链路，避免误导性设置项）
- 问题记录：`docs/issues/dawn_native_stage37_fluxfield_gpu_single_route_with_cpu_fallback.md`（将 FluxField GPU 路径改为“GPU 视觉优先，失败才 CPU 兜底”，并保证同一帧只显示一条渲染路线；同时在选项名明确 CPU-only / Auto-fallback 语义）
- 问题记录：`docs/issues/dawn_native_stage38_fluxfield_single_option_in_settings.md`（设置项中移除 FluxField 的 CPU-only 入口，只保留一个“GPU 优先 + 自动 CPU 兜底”的用户选项）
- 问题记录：`docs/issues/dawn_native_stage39_fluxfield_gpu_visibility_matrix_anchor_fallback.md`（修复 FluxField GPU 在光标映射离屏时“判定成功但不绘制”的不可见问题，改为回退到矩阵锚点继续渲染）
- 问题记录：`docs/issues/dawn_native_stage40_follow_mode_and_flux_label_copy_cleanup.md`（统一长按跟随模式文案语义：将 smooth 明确为“光标优先（推荐）”，并将 FluxField GPU 标签统一为“CPU兜底”）
- 架构整理：`docs/architecture/tray-and-appcontroller-refactor.zh-CN.md`（托盘菜单表驱动 + AppController 解耦）
- 架构整理：`docs/architecture/settingswnd-emoji-split.zh-CN.md`（SettingsWnd 表情处理拆分）
- 架构整理：`docs/architecture/ui-folder-structure.zh-CN.md`（UI 文件夹结构整理）
- 架构整理：`docs/architecture/mousefx-folder-structure.zh-CN.md`（MouseFx 文件夹结构整理）
- 架构整理：`docs/architecture/trail-effects-differentiation.zh-CN.md`（拖尾差异化：历史配置 + renderer 拆分）
- 架构整理：`docs/architecture/trail-profiles-config.zh-CN.md`（config.json：trail_profiles + reload_config）
- 架构整理：`docs/architecture/trail-tuning-settings-ui.zh-CN.md`（设置窗口：拖尾预设 + 高级调参）
- 架构整理：`docs/architecture/web-settings-ui.zh-CN.md`（浏览器设置页：内置 loopback HTTP server）
- 安装包：`docs/install/installer-packaging-20260204.md`（Inno Setup 更新，2026-02-04）

## 功能简介
- 全局鼠标点击可视化（Windows）：低级鼠标钩子 `WH_MOUSE_LL` + GDI+ 分层窗口波纹。
- 点击穿透：不会阻挡下层窗口。

## 构建与运行
1. 用 Visual Studio 打开 `MFCMouseEffect.slnx`（或生成的 `.sln`）。
2. 选择 `x64` + `Debug`（推荐测试时用）。
3. 编译并运行 `MFCMouseEffect`，输出在 `x64\Debug\MFCMouseEffect.exe`（Release 在 `x64\Release\...`）。
4. Debug 启动后 ~250ms 会在当前光标位置自动打一发“自检波纹”，之后任意点击都会有波纹。
5. 如果要点击管理员/提权窗口，最好也以管理员身份运行本程序，保证完整的钩子权限。

## 发布版无窗口（托盘常驻）
Release 版本已改为“完全不创建主框架窗口”，只创建隐藏宿主窗口用于托盘图标，因此启动不会闪出管理窗口。
- 入口：`x64\Release\MFCMouseEffect.exe`
- 托盘图标：右键菜单“退出”（双击也可退出）
- Debug 仍保留主窗口便于调试（不影响波纹渲染）
说明：托盘菜单现在会根据当前语言显示中文或英文（不再中英混排）。

## 设置（浏览器页，非 background 模式）
托盘菜单适合快速切换。复杂配置（包含高级调参）通过托盘 **设置...** 打开浏览器设置页（程序内置 loopback HTTP server）。
- 配置持久化：写入同目录 `config.json`
- 立即生效：保存后立刻应用
- 详细说明：`docs/architecture/web-settings-ui.zh-CN.md`

## 外观自定义
- 主要文件：`MFCMouseEffect/MouseFx/Styles/RippleStyle.h`、`MFCMouseEffect/MouseFx/Windows/RippleWindow.cpp`。
- 关键参数：
  - 时长：`RippleStyle::durationMs`
  - 半径：`startRadius`、`endRadius`
  - 窗口大小：`windowSize`
  - 按键配色：在 `RippleWindow::StartAt(...)` 的 switch 中调整 fill/stroke/glow

## 运行说明（宏观点）
- **UAC/管理员窗口**：如果需要在管理员应用（高权限窗口）里也完整生效，建议本程序同样“以管理员身份运行”。
- **托盘 vs 后台模式**：托盘模式可交互；后台模式不提供托盘 UI（完全由父进程/IPC 控制）。
- **IPC 控制**：后台模式通过 stdin JSON 接收父进程命令；stdin 关闭时会自动退出（用于父进程生命周期托管）。
- **配置持久化**：`config.json` 与 exe 同目录；主题与当前各分类特效会在 tray/IPC 修改时写回。
- **安全软件影响**：部分安全软件可能会拦截全局钩子或分层窗口渲染，导致不生效或间歇性失效。

## 故障排查
- **完全没波纹（Debug）**：连自检波纹都没有，说明启动失败。看弹窗里的 `Stage/Error/Message`。
  - `Stage: dispatch window`、`Error: 1400 (无效的窗口句柄)`：代码已修复，Clean+Rebuild 后运行 `x64\Debug\MFCMouseEffect.exe`。
  - 其他错误：按提示，一般是权限或系统策略。
- **钩子报错**：弹窗或 VS Output 有 `MouseFx: global hook start failed. GetLastError=...`。如果点击的是管理员窗口，请用“以管理员身份运行”启动本程序；安全软件可能拦截钩子。
- **高 DPI 偏位**：启动时已启用 DPI 感知；确保用最新编译的二进制。
- **运行错了二进制**：曾经生成过 `MFCMouseEffect\x64\Debug\...` 的旧输出，现已改为 `x64\Debug\...`。请 Clean + Rebuild 后运行新路径。
- **虚拟副屏/平板副屏偏移**：部分虚拟显示器软件/驱动会导致坐标映射异常或 DPI 坐标系不一致，表现为特效偏移/消失。参见：`docs/issues/virtual-display-coordinates.zh-CN.md`。
  - 2026-01：已默认启用坐标归一化，大多数虚拟副屏场景已恢复正常；若仍有偏移，请附上副屏软件/驱动与 DPI 信息反馈。

## SDI/单窗口说明
- 项目已改为 SDI：一个顶层主窗口承载视图；波纹渲染仍在独立的透明分层窗口中，与主框架解耦。
- 如果需要多个窗口，可按需创建多个独立顶层框架（非 MDI 子窗），互不嵌套。
