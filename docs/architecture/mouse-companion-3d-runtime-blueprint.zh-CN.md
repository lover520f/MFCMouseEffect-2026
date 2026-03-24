# Mouse Companion 3D Runtime Blueprint (P2)

## 目标与边界
- 目标：构建高性能、小体积、可扩展的 3D 鼠标伴宠运行时。
- 动作策略：不依赖模型内置帧动画；由宿主动作合成器驱动骨架姿态。
- 资产策略：外部格式可多样，内部运行时统一标准为 `glTF 2.0 / .glb`。

## 当前落地（P0/P1/P2）
- 新增核心模块目录：`MFCMouseEffect/MouseFx/Core/Pet`
- 已落地文件：
  - `PetTypes.h`
  - `PetAssetFormat.h/.cpp`
  - `PetInterfaces.h`
  - `PetActionSynthesizer.cpp`
  - `PetGlbSkeletonLoader.h/.cpp`
  - `PetSkeletonModelRuntime.cpp`
  - `PetNullModelRuntime.cpp`
  - `PetCompanionRuntime.h/.cpp`
  - `Core/Control/PetDispatchFeature.h/.cpp`
- 已接入 `Platform/CMakeLists.txt`（POSIX core runtime 构建链路）。
- 已接入 `MFCMouseEffect.vcxproj`（Windows 工程源文件清单，避免后续链接回归）。
- 已接入 macOS Swift 可视桥：
  - `MFCMouseEffect/Platform/macos/Pet/MacosMouseCompanionSwiftBridge.h`
  - `MFCMouseEffect/Platform/macos/Pet/MacosMouseCompanionBridge.swift`
  - `AppController` 生命周期与状态更新路径已打通 `create/show/hide/load/update` C API。

## 分层设计
1. 资产层（Import）
- `IModelAssetImporter::ImportToCanonicalGlb(...)`
- 负责将 `vrm/gltf/usdz/fbx/...` 统一导入为 canonical `.glb`。

2. 模型运行时层（Model Runtime）
- `IPetModelRuntime`
- 负责加载 canonical 资产、接收骨架姿态、应用外观覆写。

3. 动作合成层（Action Synthesizer）
- `IActionSynthesizer`
- 接收行为请求（`Idle/Follow/ClickReact/Drag`）并逐帧输出 `SkeletonPose`。
- 新增 `BindSkeleton(const SkeletonDesc*)` 契约，明确动作合成器与骨架描述的绑定时机。
- P0 默认实现为 `CreateDefaultActionSynthesizer()`，已升级为“骨架索引驱动的程序姿态”而非模型内置帧动画。

4. 编排层（Controller）
- `PetCompanionRuntime`
- 负责把输入帧 -> 动作合成 -> 模型姿态应用串起来。

5. 控制路由层（Dispatch Feature）
- `PetDispatchFeature`
- 负责把 `DispatchRouter` 的 `click/move/button-down/button-up` 映射到伴宠动作请求接口。

## 接口契约（P0）
- `ModelFormat`：
  - 支持识别：`glb/gltf/usdz/vrm/fbx`
  - canonical 判定：仅 `glb` 为运行时内部标准
- `SkeletonPose`：
  - 当前包含基础 locomotion/强度指标（`locomotionForward/locomotionTurn/actionIntensity`）
  - `BonePose` 现支持 `boneIndex`（索引优先）+ `name`（回退匹配）双通道
  - 后续可扩展到完整骨骼局部变换列表与约束上下文
- `IActionSynthesizer`：
  - 生命周期契约：模型加载成功后必须调用 `BindSkeleton`，再进入 `Update` 逐帧阶段。
  - 输出契约：优先输出带 `boneIndex` 的 `BonePose`，保证渲染桥和运行时可 O(1) 槽位映射。
- `AppearanceOverrides`：
  - `skinVariantId`
  - `enabledAccessoryIds`
  - `textureOverridePaths`

## P1 增量（已完成）
- `AppController` 已内置伴宠运行时实例（已切换到 `CreateDefaultPetModelRuntime()`，不再是 Null runtime）。
- 输入事件映射已打通：
  - `move` -> `Follow`（拖拽态为 `Drag`）
  - `click` -> `ClickReact`
  - `button down/up` -> `Drag` / `Follow` 切换
- 伴宠帧输入 `dt` 已做边界钳制（`1/240s ~ 1/15s`），避免异常 tick 造成动作突变。

## P2 增量（已完成）
- 默认模型运行时从 `Null` 升级为 `CreateDefaultPetModelRuntime()`（骨架可加载、姿态可消费）。
- 姿态通道已升级为索引优先：
  - `ApplyPose` 会优先使用 `boneIndex` 直达骨架槽位，`name` 仅作为兼容回退，
  - 运行时维护了按骨架索引排列的局部姿态缓存（为后续渲染桥直接消费做准备）。
- 新增最小 `glb` 骨架解析链路：
  - 解析 `glb` header/chunk（v2，JSON chunk）
  - 提取 `skins[joints]` 与 `nodes` 父子关系
  - 构建 `SkeletonDesc`（骨骼名、父索引、源节点索引）
- Windows real renderer 现在也已接入最小 `glb` 节点树摘要链路：
  - 解析 `pet-main.glb` 的 header + JSON chunk
  - 产出 `node index / parent / children / path`
  - 首轮节点匹配从 `assetNodeMatchGraphProfile` 开始优先消费这组真实节点摘要，并通过独立的 naming/matcher 小组件做 token 标准化和候选命中；命中结果继续补充 `parent/depth/semanticTag`，供 `world-space / joint-hint / frame / adornment / overlay` 这条显示链直接消费，失败时仍回退 preview
  - Windows real renderer 现在还会把这组真实节点树绘制成一层 `scene-graph overlay`，并通过逻辑 anchor 到命中节点的虚线连接，把 preview contract 和真实 `glb` 节点拓扑先桥接到同一张画布上；`assetNodeWorldSpaceProfile` 也开始把命中节点中心当成真实 world-space 输入，后面的 pose/joint-hint/显示链不再只吃 preview anchor，且新增的 `model scene pose projector + topology projector` 会把这些 world-space 命中结果及 `body->head/appendage/overlay/grounding` 的相对拓扑再反投影回对应 scene 群组，让第一版真实 3D 拓扑开始影响现有 stylized preview 本体；同时新增 `model proxy layer`，把这些 resolved 节点直接渲染成一层简化模型壳层/骨架层，并补上 proxy silhouettes + proxy surface ribbons + core shell + proxy hull，再把 hull 的包围范围反投影到底部 shadow/pedestal/glow footprint，并把 click/hold/scroll/drag/follow 等 overlay 重新投影到同一层 proxy shell 上；最新又把 head silhouette 反投影回 face/detail 层，把 `body/head/tail` 以及 torso/appendage 的关键 frame rect 一并重投影到对应 proxy silhouettes，并继续通过独立的 appendage/adornment projectors 把 limb cuffs/pads/bridges、overlay/grounding anchors、badge lane 和 accessory 几何一起压回 preview 主体；随后又把 ear polygons/root cuffs/occlusion caps 以及 mouth/highlight/whisker 等表达标量接入独立 projector，并让 proxy shell 覆盖率同时回流到 body/head/tail/accent/pedestal/accessory/action-overlay 的 fill、stroke、alpha 权重以及 preview body/head/appendage/detail/adornment 各层的独立显隐比例；上一轮已经补出独立的 `proxy action layer`、`proxy frame layer`、`proxy detail layer`、`proxy adornment layer`，这一轮继续补出独立的 `proxy contour layer`，让 ears/torso contour blocks 也先在 proxy 壳层上绘制，再进一步压低旧 preview body/head/detail 的存在感，同时让 action layer 反过来驱动 body tilt、anchor scale、glow、badge 与 accessory 等动态参数，并在 render 前最后一跳统一压低旧 `actionOverlay` 的 alpha/stroke，让真实 proxy 动作层继续挤压旧 preview overlay 的存在感，从而把首版真实 3D 可见层往“主动作可见层”再推进一步
- `AppController::Start()` 现在会尝试加载默认模型路径：
  - `Assets/Pet3D/source/pet-main.glb`
  - `MFCMouseEffect/Assets/Pet3D/source/pet-main.glb`
  - 未命中时安全降级，不影响现有功能。
- 新增 importer 工厂：`CreateDefaultModelAssetImporter()`
  - 当前 `glb` 走直通
  - `gltf/usdz/vrm/fbx` 已定义为输入格式，但转换链路尚未实现（显式返回 warning）
- 动作合成器已完成骨架绑定升级：
  - `PetCompanionRuntime::LoadCanonicalModel` 成功后会触发 `actionSynthesizer->BindSkeleton(CurrentSkeleton())`，
  - 默认动作合成器按骨架语义槽位（hips/spine/chest/neck/head）输出程序姿态，彻底摆脱模型内置帧依赖。

## P3 增量（已完成，最小可见）
- macOS Swift/SceneKit 可视桥已接入：
  - 透明顶层窗口 + 跟随鼠标位置；
  - 运行时加载 `.glb` 模型（最小可见链路）；
  - 接收动作状态（`actionCode/intensity/boneCount`）并驱动可视反馈（姿态倾斜/脉冲/摆动）。
- 并发安全已按 Swift 6 要求收敛（主线程/`MainActor` 调用路径已通过编译）。

## P4 增量（已完成：索引直连 + 姿态回落）
- C++ 运行时姿态已直连 Swift 渲染桥：
  - `PetCompanionRuntime` 提供 `LastPose()`；
  - `AppController` 每帧发送 `boneIndex + position/rotation/scale` 数组，不再走骨骼名逐帧匹配。
- 新增一次性骨骼绑定：
  - `mfx_macos_mouse_companion_configure_pose_binding_v1` 在模型骨架就绪后建立“骨架索引 -> SceneKit 节点”映射缓存。
- Swift 侧已补齐姿态回落策略：
  - 每帧先将绑定骨骼恢复到 `rest local transform`，
  - 再应用本帧更新骨骼姿态，避免动作切换时残留变换。
- `update_state_v1` 的动作强度/跟随反馈仍保留，作为姿态链路的视觉兜底。

## P5 增量（已完成首版：外部动作资产）
- 新增独立动作资产契约：`ActionLibrary`（JSON Clip 库）：
  - 资产文件与模型解耦，模型仍走 canonical `glb`，动作独立走 `pet-actions.json`。
  - 合成器新增 `SetActionLibrary(...)`，可按 action 采样骨骼关键帧。
- 启动默认加载动作库（存在则加载，不存在则安全降级）：
  - `Assets/Pet3D/source/pet-actions.json`
  - `MFCMouseEffect/Assets/Pet3D/source/pet-actions.json`
- 运行时策略：
  - 当前 action 存在 Clip：使用 Clip 采样结果输出骨骼姿态；
  - 不存在 Clip：自动回退到程序动作通道（不影响可用性）。
- 详细契约见：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-action-clip-contract.zh-CN.md`

## 为什么先做这个骨架
- 避免把格式差异、渲染桥接、动作语义耦合在一起。
- 先稳定接口边界，再迭代具体 importer/IK/render backend，回归成本最低。
- 对后续 VRM 接入保持开放：VRM 只作为 importer 输入之一，不改变核心控制面。

## P6 增量（已完成首版：外观可配）
- 新增外观配置契约与加载器：
  - `PetAppearanceProfile`（`pet-appearance.json`），支持 default/preset 两种组织方式。
- C++ 运行时链路：
  - `AppController` 启动时尝试加载默认外观配置；
  - `PetCompanionRuntime::ApplyAppearance(...)` 保存当前外观覆写并透传到模型运行时。
- macOS 渲染桥链路：
  - 新增 `mfx_macos_mouse_companion_apply_appearance_v1(...)`；
  - 支持配件节点显隐（`enabledAccessoryIds`）；
  - 支持材质纹理覆写（`textureOverridePaths`，`Material=Path`/`Material:Path`）；
  - 未覆写材质回退原始 diffuse 内容。
- 详细契约见：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-appearance-contract.zh-CN.md`

## P7 增量（已完成：可插拔导入管线 + 首个真实后端）
- 导入层已从“单体 importer 内置转换逻辑”升级为“可插拔转换器契约”：
  - 新增 `IModelFormatConverter`、`ModelConversionResult`；
  - 新增工厂 `CreateDefaultModelFormatConverter()` 与 `CreateModelAssetImporter(...)`。
- 默认转换器实现改为组合管线：
  - `glb` 直通转换器；
  - `vrm` 真实转换后端（校验 GLB header，导出/复用 `canonical/<stem>.glb`）；
  - `gltf` 真实转换后端（复制外部资源，导出/复用 `canonical/<stem>.glb`）；
  - `usdz/fbx` tool-backed 真实转换后端（命令模板可配，失败自动回落 sidecar）；
  - 非 `glb`（`gltf/usdz/vrm/fbx`）sidecar 探测兜底转换器。
- `DefaultModelAssetImporter` 现在只负责：
  1. 输入格式识别；
  2. 委托转换器得到 canonical `.glb`；
  3. 调用 `ValidateCanonicalGlb` 做 fail-fast 校验。
- 失败诊断已细化：
  - 引入 `converter.vrm.*` / `converter.gltf.*` / `converter.usdz.*` / `converter.fbx.*` warning 前缀，区分 source 缺失、header/JSON 非法、命令执行失败、资源复制、缓存复用、写入失败等路径。
- 详细契约见：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-model-import-pipeline-contract.zh-CN.md`

## P8 增量（已完成首版：WebSettings 主链路）
- WebSettings 主链路已接入 `mouse_companion`：
  - Workspace 分区新增 `Mouse Companion`（`section_mouse_companion`）；
  - apply payload 新增 `mouse_companion` 对象并进入后端设置应用链路；
  - schema/state 均新增 `mouse_companion` 契约（范围与当前值）。
- 配置持久化已进入 `EffectConfig`：
  - 新增 `MouseCompanionConfig`（`enabled/model_path/size/offset/smoothing/test-profile`）；
  - JSON codec 已支持 `mouse_companion` 读写与 sanitize。
- 控制器编排已支持基础运行时联动：
  - `AppController::SetMouseCompanionConfig(...)` 已接入；
  - `mouse_companion.enabled=false` 时停用 pet dispatch + 释放可视宿主；
  - `mouse_companion.size_px` 现在既影响 macOS 可视宿主创建尺寸，也能通过 `mfx_macos_mouse_companion_panel_configure_v1(...)` 在宿主持续运行时即时重配；已加载模型会先把画板重置回新的 `size_px` 基线，再同步重跑 `normalizeModelTransform() + fitCanvasToModel()`，并在设置瞬间使用 live transform 的投影边界立刻缩放画板、随后再补两帧渲染态精修，避免只把模型缩小而外层画板仍停留在旧大尺寸，同时保证 `strict` 边界限制立即跟随新容器尺寸；
  - SceneKit 运行时边界快照不再临时把可见 `SCNView` 刷成洋红色抠图，改为直接利用透明背景 + alpha 扫描，避免用户在设置尺寸时看到红紫色闪屏；
  - 模型加载优先使用 `mouse_companion.model_path`，缺失时回落默认候选路径。

## P8 增量（已完成第二版：参数真正生效）
- dispatch 运行时参数已接通：
  - `smoothing_percent`：输入游标先平滑，再进入伴宠动作合成 tick；
  - `follow_threshold_px`：小位移抖动可被阈值过滤，减少高频无效 follow 调度；
  - `release_hold_ms`：主键释放后可保持短暂 drag 尾段，再回到 follow。
- 测试档位已进入运行时分流：
  - 当 `use_test_profile=true` 时，`test_smoothing_percent` 与 `test_press_lift_px` 覆盖生产值生效。
- macOS 可视桥参数已接通：
  - 新增 `mfx_macos_mouse_companion_configure_follow_profile_v1(...)`；
  - `offset_x / offset_y / press_lift_px / edge_clamp_mode` 会进入窗口跟随位姿更新，不再只是配置落库字段。

## P9 增量（已完成首版：资产路径可配置 + 定向热重载）
- 配置契约扩展（`mouse_companion`）：
  - 新增 `action_library_path`（动作库 JSON 路径）；
  - 新增 `appearance_profile_path`（外观配置 JSON 路径）。
- 配置契约扩展（`mouse_companion`）：
  - 新增 `edge_clamp_mode`（`strict | soft | free`，默认 `soft`）；
  - WebSettings 与运行时均支持实时切换边界策略，不需要重启 host。
- 端到端链路已打通：
  - `EffectConfig` + JSON parse/serialize + settings state/schema + `apply_settings` 均已支持新字段；
  - WebSettings `Mouse Companion` 分区已可编辑两条路径并随 `Apply` 持久化。
- 运行时策略：
  - `SetMouseCompanionConfig(...)` 会对路径变更做差异化处理；
  - 仅模型路径变化时走模型重载；
  - 仅动作库/外观路径变化时走定向热重载（不强制重载模型）。
- 导入回退策略：
  - 动作库/外观解析默认优先使用配置路径；
  - 配置路径不存在时，回退到内置候选路径（保持可用性）。

## P10 增量（已完成首版：Web 动作探针）
- `Mouse Companion` 分区新增动作探针面板（Action Probe）：
  - 可直接触发 `status/move/button_down/button_up/click` 以及一键 `run sequence`；
  - 探针参数支持 `x/y/button` 输入，便于做确定性动作切换验证。
- 探针面板与运行时诊断联动：
  - 每次探针调用成功后，会刷新伴宠运行时快照与 `action_coverage` 显示，
  - 可在不手动移动鼠标的情况下确认 `idle -> follow -> drag -> follow -> click_react` 链路是否可达。
- 安全边界：
  - 面板调用的是测试路由 `/api/mouse-companion/test-dispatch`，
  - 仅在显式开启 `MFX_ENABLE_MOUSE_COMPANION_TEST_API=1` 时可用；未开启时 UI 会给出明确提示。

## P11 增量（已完成首版：Web 二级 Tab 结构）
- `Mouse Companion` 分区已从“单长表单”升级为二级 Tab：
  - `Basic`：启用、模型/动作库/外观路径、尺寸；
  - `Follow`：边界策略、偏移、抬升、平滑/阈值/释放保持、测试档；
  - `Probe`：动作探针与结果反馈；
  - `Runtime`：运行时诊断与覆盖率细节。
- 兼容性约束：
  - 仍复用原字段 ID（例如 `mc_model_path`、`mc_probe_*`、`mc_runtime_*`），`read/write/apply` 契约不变；
  - tab 状态在 `render` 刷新后保持，不会因状态轮询跳回默认页。
- 可用性增强：
  - 支持鼠标点击切换；
  - 支持键盘 `ArrowLeft/ArrowRight` 在二级 tab 间快速切换。
  - 激活 tab 会持久化到本地 UI 状态存储（`mouse-companion.v1`），刷新后保持上次停留页签。
- 可维护性增强：
  - 二级 tab 的分区模板（`Basic/Follow/Probe/Runtime`）已抽取到 `WebUIWorkspace/src/mouse-companion/section-template.js`；
  - 二级 tab 的状态归一化、键盘切换与面板显隐同步已抽取到 `WebUIWorkspace/src/mouse-companion/tab-controller.js`；
  - 表单字段读写与 range 绑定已抽取到契约模块 `WebUIWorkspace/src/mouse-companion/form-contract.js`；
  - 运行态诊断的 normalize + DOM 渲染逻辑已抽取到 `WebUIWorkspace/src/mouse-companion/runtime-diagnostics.js`；
  - 动作探针的输入读取、dispatch、结果反馈状态机已抽取到 `WebUIWorkspace/src/mouse-companion/probe-controller.js`；
  - 后续新增配置字段时，优先在契约模块增量扩展，避免在入口文件散落式修改。

## 后续阶段（P7+）
1. P7-next（真实转换后端扩展）
- 在默认管线继续增强 `usdz/fbx` 命令后端适配（多命令候选、后端检测与错误语义分层）。
- 继续细化转换失败诊断（区分“不支持/依赖缺失/转换失败/产物校验失败”）。
2. P7-next（导入校验扩展）
- 增加骨架层级、尺寸/朝向归一化等强校验条目。

## 回归与验收（P0/P1/P2/P3/P4/P5）
- 编译通过：`Platform` 目标可编译新模块。
- 行为不变：现有效果、输入指示器、自动化映射路径不应受影响。
- 文档可检索：`current.md` 与本 P2 文档均可定位该决策。
