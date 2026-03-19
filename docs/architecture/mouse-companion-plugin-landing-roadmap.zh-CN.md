# Mouse Companion 插件化重建落地路线（2026-03-18）

## 1. 结论与边界
- 判定类型：`架构决策`（非临时补丁）。
- 当前基线：
  - 旧 pet 后端已清空，运行时处于 no-op 兼容态。
  - `mouse_companion` 前端与配置契约已保留，可作为新后端挂载面。
- 本路线决策：
  - 以“插件化宠物后端”作为主线；
  - 先做 `Native Plugin`（动态库/模块）落地；
  - `WASM` 作为后续可选适配层，不阻塞主线交付。

## 2. 目标（面向长期建设）
- 目标 A：宠物动作/风格可拔插，避免与单模型/单骨架强耦合。
- 目标 B：输入语义与 tauri 对齐，优先恢复 click 观感一致性。
- 目标 C：事件、动作、姿态、渲染链路具备可观测性与可回归验证能力。
- 目标 D：遵守现有工程约束（小文件、低耦合、文档同步、渐进演进）。

## 3. 对齐基准（tauri -> MFCMouseEffect）
- 语义映射基准：
  - `click` -> `click`
  - `hold` -> `hold`
  - `scroll` -> `scroll`
  - `hover` -> `idle`
  - `follow` -> `follow`
- 第一阶段可见性策略：
  - 先固定 `position_mode=fixed_bottom_left`，聚焦 click 观感对齐；
  - 再恢复 follow/idle/hold/scroll 的完整行为链。

## 4. 目标架构（插件优先）

### 4.1 分层
- `PetInputNormalizer`：输入事件归一化与门槛判定（press/travel/interval）。
- `PetActionStateMachine`：动作状态流转（click/hold/scroll/idle/follow）。
- `PetMotionSynthesizer`：语义动作到骨骼姿态的计算（clip + procedural 融合）。
- `PetRenderBridge`：平台渲染桥（macOS Swift/SceneKit、Windows 维持兼容）。
- `PetPluginHost`：插件生命周期、版本检查、热切换、回退。

### 4.2 插件形态（分期）
- `native_style_plugin`（主线）：
  - 提供动作策略、参数配置、骨骼语义映射、可选 clip 资源。
- `asset_pack_plugin`（并行）：
  - 纯资源包（动作 clip/材质/外观参数），供主线插件消费。
- `wasm_motion_adapter`（后续）：
  - 将 wasm 接口映射到同一 host 契约，作为扩展而非前置依赖。

## 5. 核心契约（先冻结再实现）

### 5.1 Host <-> Plugin API（v1 草案）
- `Initialize(const PetHostContext&)`
- `OnInput(const PetInputEvent&)`
- `Tick(uint64_t now_ms, float dt_sec)`
- `SamplePose(PetPoseFrame&)`
- `OnConfigChanged(const PetRuntimeConfig&)`
- `Shutdown()`

### 5.2 Manifest（v1 草案）
- 必填：`plugin_id`、`plugin_version`、`engine_api_version`、`plugin_type`
- 能力：`supports_click/hold/scroll/idle/follow`
- 资源：`model/action_library/appearance/effect_profile`
- 兼容：`min_host_version`、`max_host_version`

### 5.3 诊断契约（v1 草案）
- `runtime_present`
- `active_plugin_id`
- `active_action_name`
- `event_to_action_latency_ms`
- `pose_bone_count`
- `fallback_reason`
- `compatibility_status`

## 6. 分阶段路线图（建议按此执行）

## Phase 0：契约冻结与骨架搭建（1-2 天）
- 输出：
  - `PetPluginHost` 最小空实现（可加载“空插件”并输出诊断）。
  - Host/Manifest/诊断结构体与序列化路径。
- 验收门禁：
  - 编译通过；
  - `/api/state.mouse_companion_runtime` 能返回插件诊断基础字段；
  - 旧 no-op 兼容路径可回退。

## Phase 1：Click First（固定左下角，1-2 天）
- 输出：
  - 基于 `fixed_bottom_left` 的可见宠物展示；
  - click 门槛与 click 动作展示对齐 tauri；
  - 先保证乌萨奇点击动作“看得见、能回归”。
- 验收门禁：
  - 快速短按触发 click；
  - 长按或大位移不误触 click；
  - 诊断可读到 click 触发次数与最近动作。

## Phase 2：五类输入完整对齐（2-4 天）
- 输出：
  - `hold/scroll/idle/follow` 全部接入插件动作机；
  - 事件冲突窗口（如 scroll 抑制 click）与 tauri 语义一致。
- 验收门禁：
  - 五类事件都可独立触发对应动作；
  - 动作切换无明显抖动/锁死；
  - 失败时可自动回退到 idle 或 no-op。

## Phase 3：插件管理与热切换（2-3 天）
- 输出：
  - 插件发现、装载、卸载、版本校验、失败回退；
  - 配置层支持“按插件切换动作风格”。
- 验收门禁：
  - 错误插件不会拖垮主线程与渲染链路；
  - 热切换后配置与运行态一致。

## Phase 4：WASM 适配探索（可选）
- 输出：
  - 在不改 Host v1 契约的前提下接入 wasm 适配器。
- 验收门禁：
  - wasm 插件与 native 插件共用同一诊断与回退语义；
  - 任何 wasm 故障不影响 native 主线稳定性。

## 7. 测试态友好参数（强制）
- 默认保留“生产值 + 测试值”双档，不允许写死。
- 建议首批参数：
  - `click_max_press_ms`（prod: `220` / test: `260`）
  - `click_max_travel_px`（prod: `10` / test: `14`）
  - `hover_trigger_ms`（prod: `1500` / test: `320`）
  - `hold_trigger_ms`（prod: `260` / test: `120`）
- 切换方式：
  - 复用 `mouse_companion.use_test_profile`；
  - 在 `/api/state` 明确回显当前生效档位与参数。

## 8. 风险与回退策略
- 风险 1：插件 ABI 过早收敛导致后续扩展困难。
  - 应对：v1 只冻结最小必要字段，预留扩展字段与版本协商。
- 风险 2：渲染桥耦合过深导致跨平台迁移困难。
  - 应对：`PetRenderBridge` 只消费统一 `PetPoseFrame`，禁止跨层直接依赖。
- 风险 3：与旧文档/旧路径混淆。
  - 应对：本路线文档作为当前主入口；旧实现文档保留为历史参考。

## 9. 当前执行建议（从今天开始）
1. 先完成 `Phase 0`：冻结 v1 契约与最小 Host 骨架。
2. 立即进入 `Phase 1`：只做 click 对齐 + 左下角固定展示，先拿到“可见一致性”。
3. 通过后再推进 `Phase 2` 到五类输入完整对齐。

## 10. 关联文档
- 后端重置契约：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-backend-reset-contract.zh-CN.md`
- Click 对齐契约（历史阶段文档，后续纳入插件化实现）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-click-parity-tauri-contract.zh-CN.md`

## 11. 当前实现状态（2026-03-18）
- 已落地 `Phase 0` 第一批代码骨架：
  - `MouseFx/Core/Control/MouseCompanionPluginHostPhase0.{h,cpp}` 已接入 `AppController`，
  - `DispatchPet*` 事件已接入 Phase0 插件宿主计数与事件轨迹，
  - 运行时诊断新增插件化字段：`plugin_host_ready`、`active_plugin_id`、`compatibility_status`、`plugin_event_count`。
- 已补充 `Phase 0.5` 契约收敛骨架：
  - 新增 `MouseCompanionPluginV1Types.h` 与 `MouseCompanionPluginHostV1.h`，
  - `PetInputEvent / PetRuntimeConfig / PetPoseFrame` 已有强类型定义，
  - `Initialize / OnInput / Tick / SamplePose / OnConfigChanged / Shutdown` 已有 builtin native plugin 骨架，
  - 当前 `DispatchPet*` 事件与 `AppController` 已解析出的 resolved action/headTint/pose frame 都会并行提交给 v1 builtin plugin，先把 v1 宿主变成统一快照源，但暂时还不切换现网渲染消费方。
- 已进入 `Phase 2` 的第一步（hold 对齐首刀）：
  - 当前先不改 Phase0/现网可见宿主切换关系，仍保持 `click/idle` 稳定链路不动，
  - `hold` 先从“姿态对齐”切入：native semantic pose 已向 tauri `applyHoldProcedural` 收敛，重点补齐耳朵前压、手部收拢、腿部弯折、身体压扁，
  - 当前 pet 视觉时序已先行解耦：`holdReact` 进入条件改为稳定按压（prod `130ms` / test `90ms`）且指针速度低于阈值（prod `24px/s` / test `30px/s`），松开后继续复用 `mouse_companion.release_hold_ms` 做 release buffer，并在 scroll 后 `720ms` 内抑制 hold 进入，
  - 普通鼠标 `hold effect` 的共享 timer 仍保留原值（prod `260ms` / test `120ms`）；后续若继续推进完全一致性，需要把 pet 与 effect 的 hold 状态机彻底拆成独立契约。
- 已进入 `Phase 2` 的第二步（scroll 对齐首刀）：
  - `scrollReact` 不再只靠线性 `scrollPulse` 衰减，现已切到 tauri 风格的 impulse 包络：prod `0.72s` / test `0.56s`，`inRatio=0.42`、`holdRatio=0.16`，
  - native semantic pose 与 placeholder fallback 都补了基于幅度的手部 lift/twist、腿部外开与轻微 flap 调制；scroll 耳朵现按 tauri 的 `wingOpen + flap` 语义做翅膀式扇动，不再收成“向内夹”，
  - 当前仍未做“基于方向的额外滚动姿态偏置”；这一点若后续需要，再单独加在统一插件动作机上。
- 已进入 `Phase 2` 的第三步（follow 对齐首刀）：
  - macOS 视觉宿主已开始记录 follow 模式下的指针归一化 X，并把 real-model yaw 对齐到 tauri 的 `baseYaw - normalizedX * 0.28`，
  - fallback 视图同步复用该指针归一化量做轻量横向 follow tilt，
  - native follow 的额外前倾已明显收轻，避免视觉上更像 drag 而不是 follow。
- 已落地 `Phase 1` 第一批 click-first 状态机语义（后端）：
  - click 门槛：`press<=220ms && travel<=10px`，并接入 `scroll` 后短窗口抑制，
  - `position_mode=fixed_bottom_left` 下 move 路径保持 `idle`（不进入 follow），
  - click streak/head tint 诊断在 test-dispatch 与 `/api/state` 可观测。
- 已落地 `Phase 1` 可见性占位宿主（macOS）：
  - 新增 Swift 桥接：`Platform/macos/Pet/MacosMouseCompanionPhase1Bridge.swift`，
  - `fixed_bottom_left` 模式下可固定显示（左下角），并接收 `idle/follow/click/drag/hold/scroll` 动作更新，
  - 视觉宿主已升级为“模型优先 + 占位兜底”：优先加载 `Assets/Pet3D/source/pet-main.*`（当前 SceneKit 首选命中 `pet-main.usdz`），失败时回退 `phase1://placeholder/usagi`，
  - `/api/state.mouse_companion_runtime` 可读到 `model_loaded`、`loaded_model_path`、`loaded_model_source_format` 与 `pose_binding_configured`。
- 旧 action-library / effect-profile / appearance 运行时仍处于 Phase2+ 待恢复状态（本阶段聚焦 click-first 可见一致性与可观测性）。
