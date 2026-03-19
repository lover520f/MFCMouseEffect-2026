# Mouse Companion Action Clip Contract (P2)

## 目标
- 为“复用已有骨架动作”提供稳定契约，不依赖模型内置帧动画。
- 让动作资产和模型资产解耦：模型走 `glb`，动作走独立 JSON Clip 库。
- 与语义参数驱动层（`pet-effects.json`）配合时，Clip 负责“可复用动作片段”，语义层负责“事件级动作风格与强度”。
  - 参考：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-procedural-effect-profile-contract.zh-CN.md`

## 运行时链路
1. `AppController` 启动时尝试加载默认动作库：
   - `Assets/Pet3D/source/pet-actions.json`
   - `MFCMouseEffect/Assets/Pet3D/source/pet-actions.json`
2. `PetCompanionRuntime::LoadActionLibraryFromJson(...)` 读取并持有 `ActionLibrary`。
3. `IActionSynthesizer::SetActionLibrary(...)` 接收动作库；`BindSkeleton(...)` 后构建轨道到骨骼索引映射。
4. macOS Swift SceneKit 宿主会并行读取同一动作库：
   - `clickReact` 走 one-shot；
   - `idle/follow` 走循环 clip；
   - 然后再叠加宿主侧程序动作（例如 tauri 对齐的 idle 耳朵/手臂波动）。
5. 每帧 `Update(...)`：
   - 当前 Action 存在 Clip：采样 Clip 输出 `BonePose`。
   - 当前 Action 无 Clip：回退程序动作（呼吸/头部跟随/点击脉冲）。

## JSON 契约

顶层：
```json
{
  "version": 1,
  "bone_remap": {},
  "clips": []
}
```

`clips[]` 元素字段：
- `action`：
  - 基础：`idle | follow | clickReact | drag`
  - 扩展：`hoverReact | holdReact | scrollReact`
  - 大小写不敏感，且支持下划线别名（例如 `click_react/hover_react/hold_react/scroll_react`）
- `duration`：秒，`<=0` 时自动取该 Clip 最大关键帧时间
- `loop`：是否循环
- `tracks`：骨骼轨道数组

`tracks[]` 元素字段：
- `bone`：骨骼名（运行时会按归一化名称匹配骨骼）
- `keyframes`：关键帧数组

`bone_remap`（可选）：
- 用于“动作轨道骨骼名”和“模型骨骼名”不一致时的显式重映射。
- 格式：`{ "<trackBone>": "<targetBone>" | ["<targetBone1>", "<targetBone2>", ...] }`
- 运行时匹配顺序：
  1. 轨道骨骼名直接匹配
  2. 运行时内置同义词兜底（例如 chest/upperchest/spine2）
  3. `bone_remap` 候选列表按顺序尝试

`keyframes[]` 元素字段：
- `t`：时间（秒，必填）
- `pos`：`[x,y,z]` 可选
- `rot`：`[x,y,z,w]` 可选
- `scale`：`[x,y,z]` 可选
- `interp`：`step | linear`（默认 `linear`）

说明：
- 缺失的 `pos/rot/scale` 使用 `Transform` 默认值（位移 0、旋转单位四元数、缩放 1）。
- 关键帧按 `t` 自动排序。
- 非法轨道会被跳过；Clip 里若无有效轨道则该 Clip 丢弃。

## 插值规则
- `step`：保持左关键帧值直到下一个关键帧。
- `linear`：位置/缩放线性插值，旋转使用归一化线性插值（NLERP）。

## 骨骼匹配规则
1. 先按归一化骨骼名精确匹配（小写）。
2. 未命中时做弱匹配（`contains`）兜底。
3. 轨道未匹配到骨骼则忽略，不影响其他轨道和运行时稳定性。

## 当前样例
- 仓库已提供样例动作库：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Assets/Pet3D/source/pet-actions.json`
- 该样例覆盖 `idle/follow/clickReact/drag/hoverReact/holdReact/scrollReact`，并包含 `bone_remap`（`Chest -> Spine`）示例，可直接验证重映射链路。
- 2026-03-17 可见性调优：
  - `hoverReact/holdReact/scrollReact` 关键帧幅度已增强（更明显的头/躯干旋转与 hold 压缩），用于避免“运行时已切动作但体感像只有两种姿态”。
- 2026-03-19 SceneKit 对齐补充：
  - Swift 宿主不再只解析 `clickReact`；
  - 真实模型已接入 `idle/follow` clip 采样；
  - `idle` 下还会叠加 tauri 同款 ear/hand procedural，因此看到耳朵在 idle 轻摆是预期行为。
  - pose binding 也已从“精确骨骼名”扩展到“语义映射”，例如 `left_ear/right_ear/left_hand/right_hand/left_leg/right_leg` 会匹配 `DEF-Ear.L`、`DEF-Arm.L.*`、`DEF-Leg.L.*` 这类 Blender 导出骨骼链，避免程序动作因为资产命名差异失效。
  - 语义 pose binding 当前按“代表骨骼”落点：对于 `DEF-Ear.L/.001/.002` 这类导出骨骼链，不再把同一个 idle 程序旋转同时施加到整条链上，而是优先选根耳骨，避免 idle 耳朵出现链式叠加颤动。

## 回归检查
- 编译：`cmake --build build-macos --target mfx_entry_runtime_common -j 6`
- 启动后检查：
  1. 默认模型加载成功时，动作库可自动加载（文件存在前提）。
  2. move/click/drag 时姿态有可见变化。
  3. 删除 `pet-actions.json` 后仍可运行（程序动作兜底）。

## 契约覆盖率校验（新增）

为避免“动作文件可读但骨骼名映射失效”这类隐性问题，当前已提供测试态覆盖率报告：

1. 开启测试路由环境变量：`MFX_ENABLE_MOUSE_COMPANION_TEST_API=1`
2. 调用：`POST /api/mouse-companion/test-dispatch`
   - 支持事件：`status | move | scroll | button_down | button_up | click | hover_start | hover_end | hold_start | hold_update | hold_end`
   - 返回新增字段：`action_coverage`
3. `action_coverage` 关键字段：
   - `ready` / `error`
   - `expected_action_count` / `covered_action_count` / `missing_action_count`
   - `total_track_count` / `mapped_track_count` / `overall_coverage_ratio`
   - `missing_actions[]`
   - `missing_bone_names[]`
   - `actions[]`（每个 action 的轨道映射细节）

说明：
- `missing_action_count=0` 仅表示当前门禁动作集（现为 7 类：`idle/follow/click_react/drag/hover_react/hold_react/scroll_react`）都有 Clip，不代表所有轨道都能映射。
- `missing_bone_names` 用于定位具体骨骼命名偏差（建议优先通过 `bone_remap` 修正）。
- 该校验是“契约可行性门禁”，不影响运行时兜底策略（未映射轨道会被跳过，系统继续运行）。

## `/api/state` 运行态可观测字段（新增）

- 常规设置页读取：`GET /api/state`
- 路径：`mouse_companion_runtime.action_coverage`
- 当前字段：
  - 汇总：`ready/error/expected_action_count/covered_action_count/missing_action_count/skeleton_bone_count/total_track_count/mapped_track_count/overall_coverage_ratio`
  - 缺失项：`missing_actions[]/missing_bone_names[]`
  - 明细：`actions[]`（`action_name/clip_present/track_count/mapped_track_count/coverage_ratio/missing_bone_tracks[]`）

说明：
- 该字段由运行时在动作库加载后缓存并输出，模型或动作库不可用时会回退到“未就绪 + error”状态，避免 UI 看到陈旧覆盖率结果。

## 一键 proof 门禁（新增）

`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-mouse-companion-proof.sh`

当前脚本门禁同时覆盖：
- 资产加载就绪：`model/visual/action/appearance/pose_binding`
- 动作切换序列：`idle -> follow -> scroll_react -> drag -> follow -> click_react -> hover_react -> follow -> hold_react -> hold_react -> follow`
- 动作契约覆盖：`expected_action_count=7 && missing_action_count=0 && missing_bone_names=[] && overall_coverage_ratio=1.0`

说明（2026-03-17）：
- 由于 hover 触发阈值已从旧值下调（生产档 `1500ms`，测试档 `320ms`），proof 对首帧 `idle` 判定允许 `idle/follow/hover_react` 三种结果，避免定时器竞争导致伪失败。
