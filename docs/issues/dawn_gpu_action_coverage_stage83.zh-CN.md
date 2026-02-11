# Dawn GPU 建议动作覆盖补全（Stage 83）

## 背景
- Web 设置页会根据后端 `gpu_status_banner.action.action_code` 显示建议动作按钮。
- 前端只识别了部分 action code，导致部分有效建议在 UI 中被隐藏。

## 问题
- 按钮是否显示取决于 `isGpuActionSupported(actionCode)`。
- 以下 code 未被覆盖：
  - `install_dawn_runtime`
  - `replace_runtime_binary`
  - `enable_dawn_build_flag`
  - `check_display_adapter`
- 这些场景下用户无法在同一操作区触发默认“重探测/刷新”流程。

## 改动
- 更新 `MFCMouseEffect/WebUI/app.js`：
  - 扩展 `isGpuActionSupported(...)`，加入上述 action code。
  - 保持现有执行策略不变：
    - 有专门处理逻辑的动作继续走专门分支；
    - 其它已支持动作统一走 probe/recheck 兜底流程。

## 结果
- 后端给出的常见建议动作都能在前端显示按钮，不再静默隐藏。
- Dawn GPU 排障链路更连续，且不引入额外调试 API。

