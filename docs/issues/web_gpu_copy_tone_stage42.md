# Stage42: Web GPU 文案收紧与安装脚本补齐

## 变更目标
- 去掉 Web 端 GPU 状态里的误导性表达（例如“仍以 CPU 为主”）。
- 保留真实状态字段（queue/encoder/prime/diag）供诊断，不在主文案里放过度技术细节。
- 安装脚本预留 GPU 运行时 DLL 文件复制条目（不影响当前未发版流程）。

## 代码变更
- `MFCMouseEffect/WebUI/app.js`
  - `pipelineLabel` 调整为中性描述：
    - `dawn_host_compat_layered` => `GPU 合成路径（宿主兼容模式）`
    - `dawn_compositor` => `GPU 合成路径（Compositor 模式）`
  - `gpuSummaryText` 的 GPU 激活文案改为简洁确认：
    - `已启用 GPU 后端 / GPU backend is active`
  - `accelerationLabelText` 文案改为 `GPU 合成已启用 / GPU compositor active`
  - queue 未就绪提示改为“降级路径”而非“CPU主路径”。

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `BuildGpuAccelerationJson` 中英文文案收敛，去掉“CPU-heavy/effects still CPU-rasterized”。
  - `BuildGpuBannerJson` 的 `gpu_active` 主文案改为简洁确认：
    - `当前已启用 GPU 后端 / GPU backend active.`

- `Install/MFCMouseEffect.iss`
  - 在 `[Files]` 增加可选 DLL 复制：
    - `webgpu_dawn.dll`
    - `d3dcompiler_47.dll`
  - 均使用 `skipifsourcedoesntexist`，不阻断当前本地开发流程。

## 说明
- 本阶段只收敛文案和安装脚本条目，不改变渲染路径逻辑。
- 诊断细节仍保留在 `diag=1` 输出，便于后续定位 runtime 问题。
