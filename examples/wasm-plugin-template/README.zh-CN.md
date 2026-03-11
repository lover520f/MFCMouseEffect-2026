# MFCMouseEffect WASM 插件模板

这是 `MFCMouseEffect` 官方的 WASM v2（AssemblyScript）插件模板。

语言： [English](README.md) | [中文](README.zh-CN.md)

## 模板包含内容

- 稳定 ABI 辅助：点击/通用事件输入解析 + 命令缓冲输出写入。
- 可复用随机/颜色工具。
- 覆盖文本、图片、混合、脉冲原语与标准事件通道的完整样例矩阵。
- 默认构建 / 单样例构建 / 全样例构建 / 运行时样例同步脚本。

## 目录结构

```text
examples/wasm-plugin-template/
  assembly/
    common/
      abi.ts                  # ABI 常量与读写辅助
      random.ts               # 确定性随机辅助
    samples/
      effects/
        text-rise.ts
        text-burst.ts
        text-spiral.ts
        text-wave-chain.ts
        image-pulse.ts
        image-burst.ts
        image-lift.ts
        mixed-text-image.ts
        mixed-emoji-celebrate.ts
        button-adaptive.ts
        click-pulse-dual.ts
        click-polyline-zigzag.ts
        click-path-fill-clip-window.ts
        click-path-clip-lanes.ts
        click-ribbon-trace.ts
        click-glow-burst.ts
        click-sprite-burst.ts
        click-quad-atlas-burst.ts
        click-retained-quad-field.ts
        click-retained-ribbon-trail.ts
        click-retained-group-pass.ts
        move-stream-sparks.ts
        scroll-particle-burst.ts
        scroll-neon-burst.ts
        hold-orbit-pulse.ts
        hover-spark-ring.ts
      indicator/
        input-indicator-basic.ts
        input-indicator-keyviz.ts
    index.ts                  # 默认入口（当前导出 text-rise）
  scripts/
    build-lib.mjs             # 构建公共能力
    sample-presets.mjs        # 样例矩阵元数据
    build.mjs                 # 构建默认入口
    build-sample.mjs          # 按 key 构建单样例
    build-all-samples.mjs     # 构建全部样例
    sync-runtime-samples.mjs  # 同步官方样例到运行时插件目录
    clean.mjs                 # 清理 dist
  plugin.json                 # 默认清单模板
  asconfig.json
  package.json
  README.md
  README.zh-CN.md
```

## 安装与构建

```bash
pnpm install
pnpm run build
```

或使用 npm：

```bash
npm install
npm run build
```

默认产物：
- `dist/effect.wasm`
- `dist/effect.wat`
- `dist/plugin.json`

## 构建样例预设

构建单个样例：

```bash
pnpm run build:sample -- --sample text-burst
```

构建全部样例：

```bash
pnpm run build:samples
pnpm run sync:runtime-samples
```

样例产物目录：
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/effect.wat`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*`（自动复制 `image_assets` 引用的图片）

清理语义：
- `pnpm run build` 会重建 `dist/` 根目录产物，并保留 `dist/samples/`。
- `pnpm run build:sample -- --sample <key>` 只会重建 `dist/samples/<key>/`。
- `pnpm run build:samples` 会先清空 `dist/samples/`，再重建完整样例矩阵。
- `pnpm run sync:runtime-samples` 会重建官方样例产物、清理受管旧 sample/模板目录，并把当前官方样例重新发布到运行时扫描目录。
- `pnpm run clean` 会删除整个 `dist/` 目录树。

运行时同步参数：
- `pnpm run sync:runtime-samples -- --skip-build true`
- `pnpm run sync:runtime-samples -- --runtime-root /custom/plugin/root`

受管清理范围：
- 本模板官方样例/模板 id（`demo.*.v2`）
- 派生出的历史官方 id（`demo.*.v1`）
- 不在该 id 集合里的自定义插件目录会保留

## 全量样例矩阵

| key | 分类 | 行为说明 | image_assets |
| --- | --- | --- | --- |
| `text-rise` | 文本 | 单条上浮文本，按键影响漂移方向 | 否 |
| `text-burst` | 文本 | 左右双向文本爆发 | 否 |
| `text-spiral` | 文本 | 三段文本螺旋式扩散 | 否 |
| `text-wave-chain` | 文本 | 4 段波浪链式文本 | 否 |
| `image-pulse` | 图片 | 单图脉冲，按键影响 imageId | 是 |
| `image-burst` | 图片 | 3 张图片径向喷发 | 是 |
| `image-lift` | 图片 | 2 张图片上浮组合 | 是 |
| `mixed-text-image` | 混合 | 1 文本 + 1 图片 | 是 |
| `mixed-emoji-celebrate` | 混合 | 2 文本 + 2 图片庆祝效果 | 是 |
| `button-adaptive` | 混合 | 根据鼠标键位选择文本/图片资源 | 是 |
| `indicator-basic` | 指示器 | 键盘通道使用专门的 key-cap panel，鼠标/滚轮通道使用专门的 pointer shell + button zone + wheel slot helper，并消费 `indicator_*` 尾部中的 streak / modifier 上下文（不再额外发 `pulse/ripple` 命令；滚轮事件不会激活点击按键高亮） | 否 |
| `indicator-keyviz-style` | 指示器 | keyviz 风格指示器：键盘通道使用分层 keycap 卡片，鼠标/滚轮通道使用深色 pointer 卡片与着色强调区域，同时复用共享 `indicator_*` 尾部语义（不再额外发 `pulse/ripple` 命令） | 否 |
| `click-pulse-dual` | 脉冲 | 仅使用 `spawn_pulse` 组合 ripple + star 双层脉冲 | 否 |
| `click-polyline-zigzag` | 折线 | 仅使用 `spawn_polyline` 组合 3 条延迟锯齿闪电线 | 否 |
| `click-path-stroke-ribbon` | path stroke | 使用 `spawn_path_stroke` + 可选共享渲染尾部组合 2 条延迟曲线丝带描边 | 否 |
| `click-path-fill-badge` | path fill | 使用 `spawn_path_fill` + `even_odd` 镂空规则与可选共享渲染尾部组合 2 层填充徽章/火花图形 | 否 |
| `click-path-fill-clip-window` | path fill | 使用 `spawn_path_fill` + 共享渲染尾部再追加 `clip_rect` 尾部，生成 1 个被裁剪窗口限制的填充徽章图形 | 否 |
| `click-path-clip-lanes` | path clip | 同时输出 1 条带 `clip_rect` 的 `spawn_path_stroke` 和 1 条带 `clip_rect` 的 `spawn_ribbon_strip`，把 path lane 剩余的 clip helper 用一个专门样例补齐 | 否 |
| `click-ribbon-trace` | ribbon strip | 使用 `spawn_ribbon_strip` + 每点宽度与可选共享渲染尾部组合 2 条延迟中心线丝带轨迹 | 否 |
| `click-glow-burst` | 发光粒子批处理 | 使用 `spawn_glow_batch` + 可选共享渲染尾部组合 2 组批量发光粒子爆发 | 否 |
| `click-sprite-burst` | 精灵批处理 | 使用 `spawn_sprite_batch` + 共享渲染尾部再追加 `clip_rect` 尾部，组合 2 组批量图片精灵喷发 | 是 |
| `click-quad-atlas-burst` | quad 批处理 | 使用 `spawn_quad_batch` + 显式宽高、每项 atlas UV 裁剪，并在共享渲染尾部后追加 `clip_rect` 尾部，组合 2 组 textured-quad 爆发 | 是 |
| `click-retained-glow-field` | retained emitter | 左/中键 upsert 带共享渲染语义并附带裁剪窗口的持续 glow emitter，右键 remove 同 id emitter | 否 |
| `click-retained-sprite-fountain` | retained emitter | 左键 upsert 带共享渲染语义并附带裁剪窗口的持续 sprite emitter，右键 remove 同 id emitter | 是 |
| `click-retained-particle-field` | retained emitter | 左键 upsert 会扩张的 soft-glow 粒子场，中键切成会收缩的 solid-disc 粒子，两种 retained 粒子现在都会附带裁剪窗口，右键 remove 同 id emitter | 否 |
| `click-retained-quad-field` | retained quad | 左/中键 upsert 一组宿主管理的持续 textured-quad，支持 atlas 裁剪、速度/加速度漂移、共享渲染语义与裁剪窗口，右键 remove 同 id field | 是 |
| `click-retained-ribbon-trail` | retained ribbon | 左键用中心线点流 upsert 一条带裁剪窗口、由宿主管理的持续 ribbon trail，右键 remove 同 id trail | 否 |
| `click-retained-group-clear` | retained group | 左/中键把 glow+ribbon+quad 三类 retained 实例放进同一个 `group_id`，右键发出 `remove_group` 一次性清掉整组 retained 实例 | 是 |
| `click-retained-group-alpha` | retained group | 左键 upsert glow+ribbon+quad 并追加 `upsert_group_presentation(alpha=1, visible=true)`，中键把同一组 retained 整体调暗，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-clip` | retained group | 左键 upsert glow+ribbon+quad 并追加 `upsert_group_clip_rect`，中键平移同一组 retained 的 clip 窗口，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-layer` | retained group | 左键 upsert glow+ribbon+quad 并追加 `upsert_group_layer(blend_override=off, sort_bias=0)`，中键把同一组 retained 提升到新的 group layer，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-mask` | retained group | 左键 upsert glow+ribbon+quad 并追加 `upsert_group_clip_rect + mask tail(round-rect)`，中键把同一组 retained 切到椭圆 mask，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-transform` | retained group | 左键 upsert glow+ribbon+quad 并追加 `upsert_group_transform(offset=0,0)`，中键平移同一组 retained，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-local-origin` | retained group | 左键先设置 `upsert_group_local_origin(x,y)`，再用组内局部坐标 upsert glow+sprite+particle+ribbon+quad，并追加带 `rotation/scale/pivot/scale2d` tails 的 `upsert_group_transform`；中键平移同一组 retained 的 local origin，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-material` | retained group | 左键先创建 glow+sprite+particle+ribbon+quad 这 5 条 retained 通道，中键对同一 `group_id` 追加 `upsert_group_material(tint,intensity,style_tail,response_tail,feedback_tail,feedback_mode_tail,feedback_stack_tail)`，右键再通过 `remove_group` 清空整组 | 是 |
| `click-retained-group-pass` | retained group | 左键先创建 glow+sprite+particle+ribbon+quad 这 5 条 retained 通道，中键对同一 `group_id` 追加 `upsert_group_pass(pass_kind,pass_amount,response_amount,mode_tail,stack_tail,pipeline_tail,blend_tail,routing_tail,lane_response_tail,temporal_tail,temporal_mode_tail,tertiary_tail,tertiary_routing_tail,tertiary_lane_response_tail,tertiary_temporal_tail,tertiary_temporal_mode_tail,tertiary_stack_tail)`，让同一组 retained 实例在现有 routed/timed secondary stage 之上再叠加一个带独立 routing/time/stack 收敛语义的第三宿主阶段，右键再通过 `remove_group` 清空整组 | 是 |
| `move-stream-sparks` | 事件（移动） | 鼠标移动触发流式文字火花 | 否 |
| `scroll-particle-burst` | 事件（滚轮） | 滚轮彩色粒子爆发，已切换为帧驱动（`on_input` 存状态 + `on_frame` 连续发射） | 是 |
| `scroll-neon-burst` | 事件（滚轮） | 对齐 HTML 风格的霓虹滚轮喷射，帧驱动并使用 `spawn_image_affine` 增强粒子变换表现 | 是 |
| `hold-orbit-pulse` | 事件（长按） | 长按驱动环绕脉冲，按帧更新环形火花 | 否 |
| `hover-spark-ring` | 事件（悬停） | 悬停开始/结束触发并按帧维持火花环 | 否 |

`sample-presets.mjs` 是样例元数据唯一来源，包含：
- 样例 key
- 源码入口
- 插件 id/name/version
- manifest 路由提示（`input_kinds`/`enable_frame_tick`）
- 可选 `image_assets`

## 内置资源包（覆盖全部支持格式）

模板 `assets/` 已覆盖并接入宿主支持的全部图片格式：
- `.png`：`smile.png`、`confetti.png`、`crown.png`、`emoji-2.png`、`mix-a.png`、`btn-left.png`、`particle-glow.png`
- `.jpg`：`coin.jpg`、`mix-b.jpg`
- `.jpeg`：`emoji-1.jpeg`
- `.bmp`：`star.bmp`
- `.gif`：`cat.gif`、`emoji-3.gif`、`party.gif`、`btn-right.gif`
- `.tif`：`lift-a.tif`
- `.tiff`：`lift-b.tiff`、`btn-middle.tiff`

这些文件位于 `examples/wasm-plugin-template/assets`，样例构建时会自动复制到对应 `dist/samples/<sample_key>/assets/`。

## 清单（plugin.json）

最小字段：

```json
{
  "id": "demo.template.default.v2",
  "name": "Demo Template Default",
  "version": "0.1.0",
  "api_version": 2,
  "entry": "effect.wasm",
  "surfaces": ["effects"],
  "input_kinds": ["click"],
  "enable_frame_tick": false
}
```

可选图片资源映射：

```json
{
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif"
  ]
}
```

规则：
- 路径相对 `plugin.json`
- 支持格式：`.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`
- `imageId` 按数组下标映射（越界取模）
- 文件缺失/无效时，宿主自动回退到内置渲染资源
- 可选 `input_kinds` 用于限制插件接收的输入通道
- 可选 `surfaces` 用于声明插件面向的宿主表面
- 可选 `enable_frame_tick` 用于控制宿主是否驱动 `mfx_plugin_on_frame`

`surfaces` 可选值：
- `effects`
- `indicator`
- `all`（旧清单省略字段时默认按全量处理；新清单建议显式声明）

`input_kinds` 可选值：
- `click`
- `move`
- `scroll`
- `hold_start`
- `hold_update`
- `hold_end`
- `hover_start`
- `hover_end`
- `indicator_click`
- `indicator_scroll`
- `indicator_key`
- `all`（省略字段时默认值）

指示器载荷补充：
- `indicator_*` 仍以 `EventInputV2` 开头
- 可选 `IndicatorEventTailV1` 提供 `size_px` 与 `duration_ms`
- 可选 `IndicatorEventContextTailV2` 提供 `primary_code`、`streak`、`modifier_mask`、`detail_flags`
- `assembly/common/abi.ts` 已提供 `hasIndicatorContextTail(...)`、`readIndicatorStreak(...)` 等辅助函数
- `assembly/common/` 下的 motif/helper 可以按效果需要复用；当前指示器样式也已有自己的专用 helper：`assembly/common/input-indicator-style.ts`

## 运行时放置路径

将 `effect.wasm` + `plugin.json` 复制到插件目录：
- Debug：`<exe_dir>/plugins/wasm/<plugin_id>/`
- Release：`%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` 必须与 `plugin.json.id` 一致。

样例包部署时请复制整个样例目录（`effect.wasm` + `plugin.json` + `assets/`）。

避免把以下两类清单放进同一个扫描根（尤其是自定义改动后出现相同 id 时）：
- 模板默认清单（`dist/plugin.json`）
- 样例清单（`dist/samples/*/plugin.json`）

## ABI 提醒

当前 ABI：
- `api_version = 2`

必须导出：
- `mfx_plugin_get_api_version`
- `mfx_plugin_on_input`
- `mfx_plugin_on_frame`

推荐导出：
- `mfx_plugin_reset`

宿主兼容规则：
- 插件必须导出 `mfx_plugin_on_input` 与 `mfx_plugin_on_frame`。

二进制布局权威定义：
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

模板 ABI 辅助还提供：
- `writeSpawnImageAffine(...)`（`COMMAND_KIND_SPAWN_IMAGE_AFFINE`，88 字节命令）
- `writeSpawnPulse(...)`（`COMMAND_KIND_SPAWN_PULSE`，52 字节命令）
- `writeSpawnPolylineHeader(...)` 与 `writeSpawnPolylinePoint(...)`（`COMMAND_KIND_SPAWN_POLYLINE`，可变长度命令）
- `writeSpawnPathStrokeHeader(...)` 与 `writePathStrokeNode* (...)`（`COMMAND_KIND_SPAWN_PATH_STROKE`，可变长度 path stroke 命令）
- `writeSpawnPathFillHeader(...)`（`COMMAND_KIND_SPAWN_PATH_FILL`，可变长度 path fill 命令，复用同一套 path 节点写入函数）
- `writeSpawnRibbonStripHeader(...)` 与 `writeSpawnRibbonStripPoint(...)`（`COMMAND_KIND_SPAWN_RIBBON_STRIP`，支持每点宽度的可变长度中心线 ribbon/trail strip 命令）
- `writeSpawnGlowBatchHeader(...)` 与 `writeSpawnGlowBatchItem(...)`（`COMMAND_KIND_SPAWN_GLOW_BATCH`，可变长度批量粒子命令）
- `writeSpawnSpriteBatchHeader(...)` 与 `writeSpawnSpriteBatchItem(...)`（`COMMAND_KIND_SPAWN_SPRITE_BATCH`，可变长度批量精灵命令）
- `writeSpawnQuadBatchHeader(...)` 与 `writeSpawnQuadBatchItem(...)`（`COMMAND_KIND_SPAWN_QUAD_BATCH`，带显式宽高与 atlas UV 的可变长度批量 textured-quad 命令）
- `writeUpsertGlowEmitter(...)` 与 `writeRemoveGlowEmitter(...)`（`COMMAND_KIND_UPSERT_GLOW_EMITTER` / `COMMAND_KIND_REMOVE_GLOW_EMITTER`，跨平台 retained glow emitter 基线命令）
- `writeUpsertSpriteEmitter(...)` 与 `writeRemoveSpriteEmitter(...)`（`COMMAND_KIND_UPSERT_SPRITE_EMITTER` / `COMMAND_KIND_REMOVE_SPRITE_EMITTER`，跨平台 retained sprite emitter 基线命令）
- `writeUpsertParticleEmitter(...)` 与 `writeRemoveParticleEmitter(...)`（`COMMAND_KIND_UPSERT_PARTICLE_EMITTER` / `COMMAND_KIND_REMOVE_PARTICLE_EMITTER`，跨平台 retained particle emitter 基线命令）
- `writeUpsertRibbonTrailHeader(...)`、`writeUpsertRibbonTrailPoint(...)` 与 `writeRemoveRibbonTrail(...)`（`COMMAND_KIND_UPSERT_RIBBON_TRAIL` / `COMMAND_KIND_REMOVE_RIBBON_TRAIL`，跨平台 retained ribbon trail 基线命令）
- `writeUpsertQuadFieldHeader(...)`、`writeUpsertQuadFieldItem(...)` 与 `writeRemoveQuadField(...)`（`COMMAND_KIND_UPSERT_QUAD_FIELD` / `COMMAND_KIND_REMOVE_QUAD_FIELD`，跨平台 retained quad field 基线命令）
- `writeRemoveGroup(...)`（`COMMAND_KIND_REMOVE_GROUP`，只作用于 retained 实例的 group 生命周期命令）
- `writeUpsertParticleEmitterWithLifeTail(...)`、`writeUpsertParticleEmitterWithSpawnTail(...)`、`writeUpsertParticleEmitterWithDynamicsTail(...)`、`writeUpsertParticleEmitterWithSpawnAndDynamicsTailsAndSemantics(...)`、`writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(...)`（retained particle emitter 的显式生成形状/发射模式 tail、`drag/turbulence`，以及 `size/alpha/color over life` helper）
- `writeSpawnPathStrokeHeaderWithSemantics(...)`、`writeSpawnPathFillHeaderWithSemantics(...)`、`writeSpawnRibbonStripHeaderWithSemantics(...)`、`writeSpawnGlowBatchHeaderWithSemantics(...)`、`writeSpawnSpriteBatchHeaderWithSemantics(...)`、`writeSpawnQuadBatchHeaderWithSemantics(...)`、`writeUpsertGlowEmitterWithSemantics(...)`、`writeUpsertSpriteEmitterWithSemantics(...)`、`writeUpsertParticleEmitterWithSemantics(...)`、`writeUpsertRibbonTrailHeaderWithSemantics(...)`、`writeUpsertQuadFieldHeaderWithSemantics(...)`，用于追加可选共享渲染尾部（`blend_mode/sort_key/group_id`）
- `writeSpawnPathStrokeHeaderWithSemanticsAndClip(...)`、`writeSpawnPathFillHeaderWithSemanticsAndClip(...)`、`writeSpawnRibbonStripHeaderWithSemanticsAndClip(...)`，用于在共享渲染尾部之后继续追加新的可选 `clip_rect` 尾部（`left_px/top_px/width_px/height_px`）
  - 样例覆盖：`click-path-fill-clip-window` 负责 `spawn_path_fill + clip_rect`，`click-path-clip-lanes` 负责补齐 `spawn_path_stroke + clip_rect` 与 `spawn_ribbon_strip + clip_rect`
- `writeSpawnSpriteBatchHeaderWithSemanticsAndClip(...)`、`writeSpawnQuadBatchHeaderWithSemanticsAndClip(...)`、`writeUpsertQuadFieldHeaderWithSemanticsAndClip(...)`，用于让 sprite/quad 几何通道也能追加同一套可选 `clip_rect` 尾部
- `writeUpsertGlowEmitterWithSemanticsAndClip(...)`、`writeUpsertSpriteEmitterWithSemanticsAndClip(...)`、`writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(...)`，用于让 retained emitter 通道也能追加同一套可选 `clip_rect` 尾部
- `writeUpsertRibbonTrailHeaderWithSemanticsAndClip(...)`，用于让 retained ribbon trail 通道也能追加同一套可选 `clip_rect` 尾部
- `writeUpsertGroupPresentation(...)`（`COMMAND_KIND_UPSERT_GROUP_PRESENTATION`，只作用于 retained 分组展示语义的命令）
- `writeUpsertGroupClipRect(...)`（`COMMAND_KIND_UPSERT_GROUP_CLIP_RECT`，只作用于 retained 分组裁剪语义的命令）
- `writeUpsertGroupClipRectWithMaskTail(...)`（`COMMAND_KIND_UPSERT_GROUP_CLIP_RECT` + 可选 mask tail，用于 `rect|round_rect|ellipse`）
- `writeUpsertGroupLayer(...)`（`COMMAND_KIND_UPSERT_GROUP_LAYER`，只作用于 retained 分组层次语义的命令）
- `writeUpsertGroupLocalOrigin(...)`（`COMMAND_KIND_UPSERT_GROUP_LOCAL_ORIGIN`，只作用于 retained 分组局部原点语义的命令）
- `writeUpsertGroupMaterial(...)` / `writeUpsertGroupMaterialWithStyle(...)` / `writeUpsertGroupMaterialWithStyleAndResponse(...)` / `writeUpsertGroupMaterialWithStyleResponseAndFeedback(...)` / `writeUpsertGroupMaterialWithAllTails(...)` / `writeUpsertGroupMaterialWithFullTails(...)`（`COMMAND_KIND_UPSERT_GROUP_MATERIAL`，只作用于 retained 分组材质语义的命令，当前宿主语义是宿主拥有的 `tint override + intensity multiplier`，以及可选的宿主拥有 style tail：`soft_bloom_like|afterimage_like + style_amount`、可选的宿主拥有 response tail：`diffusion_amount|persistence_amount`、可选的宿主拥有 feedback tail：`echo_amount|echo_drift_px`、可选的宿主拥有 feedback-mode tail：`directional|tangential|swirl + phase_rad`，再加可选的宿主拥有 feedback-stack tail：`echo_layers + echo_falloff`）
- `writeUpsertGroupPass(...)` / `writeUpsertGroupPassWithMode(...)` / `writeUpsertGroupPassWithModeAndStack(...)` / `writeUpsertGroupPassWithAllTails(...)` / `writeUpsertGroupPassWithFullTails(...)` / `writeUpsertGroupPassWithRoutingTails(...)` / `writeUpsertGroupPassWithLaneResponseTails(...)` / `writeUpsertGroupPassWithTemporalTails(...)` / `writeUpsertGroupPassWithTemporalModeTails(...)` / `writeUpsertGroupPassWithTertiaryTails(...)` / `writeUpsertGroupPassWithTertiaryRoutingTails(...)` / `writeUpsertGroupPassWithTertiaryLaneResponseTails(...)` / `writeUpsertGroupPassWithTertiaryTemporalTails(...)` / `writeUpsertGroupPassWithTertiaryTemporalModeTails(...)`（`COMMAND_KIND_UPSERT_GROUP_PASS`，只作用于 retained 分组受控 pass 语义的命令，当前宿主语义是宿主拥有的 `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`，以及可选的 `directional|tangential|swirl + phase_rad` mode tail、`echo_layers + echo_falloff` stack tail、`secondary_pass_kind + secondary_pass_amount + secondary_response_amount` pipeline tail、`secondary_blend_mode(multiply|lerp) + secondary_blend_weight` blend tail、`secondary_route_mask(glow|sprite|particle|ribbon|quad)` routing tail、`secondary_glow|sprite|particle|ribbon|quad_response` lane-response tail、`phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor` temporal tail、`exponential|linear|pulse + temporal_strength` temporal-mode tail、`tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight` tertiary-stage tail、`tertiary_route_mask(glow|sprite|particle|ribbon|quad)` tertiary-routing tail、`tertiary_glow|sprite|particle|ribbon|quad_response` tertiary lane-response tail、`tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor` tertiary temporal tail 和 `tertiary_temporal_mode + tertiary_temporal_strength` tertiary temporal-mode tail）
- retained 命令现在还提供 `...FLAG_USE_GROUP_LOCAL_ORIGIN`，用于显式声明“当前实例坐标按 `upsert_group_local_origin(...)` 的组内原点解释”
- `writeUpsertGroupTransform(...)` / `writeUpsertGroupTransformWithTail(...)` / `writeUpsertGroupTransformWithTailAndPivot(...)` / `writeUpsertGroupTransformWithTailPivotAndScale2D(...)`（`COMMAND_KIND_UPSERT_GROUP_TRANSFORM`；基础写法只开放平移，可选 transform tails 还能追加 `rotation_rad + uniform_scale`、`pivot_x_px/pivot_y_px` 和 `scale_x/scale_y`。当前几何级旋转/缩放只作用于同时启用了 `group_local_origin` 的 retained glow/sprite/particle/ribbon/quad）
- `click-retained-group-clear` 这类分组 retained 样例需要 `runtime_max_commands >= 3`；`click-retained-group-alpha`、`click-retained-group-clip`、`click-retained-group-layer`、`click-retained-group-mask` 和 `click-retained-group-transform` 需要 `runtime_max_commands >= 4`
  `click-retained-group-material` 和 `click-retained-group-pass` 需要 `runtime_max_commands >= 5`，`click-retained-group-local-origin` 需要 `runtime_max_commands >= 7`

## 相关文档

- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/wasm-plugin-compatibility.zh-CN.md`
- `docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`
