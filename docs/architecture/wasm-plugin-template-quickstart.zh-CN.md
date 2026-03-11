# WASM 插件模板快速开始

模板目录：
- `examples/wasm-plugin-template`

## 1. 构建

```bash
cd examples/wasm-plugin-template
npm install
npm run build
```

或使用 pnpm：

```bash
pnpm install
pnpm run build
```

产物：
- `dist/effect.wasm`
- `dist/plugin.json`

样例预设：

```bash
npm run build:sample -- --sample text-burst
npm run build:samples
npm run sync:runtime-samples
```

清理行为：
- `build` 会重建 `dist/` 根目录产物，同时保留 `dist/samples/`。
- `build:sample` 会重建指定 `dist/samples/<sample_key>/` 样例包。
- `build:samples` 会先清空 `dist/samples/`，再重建完整样例矩阵。
- `sync:runtime-samples` 会重建官方样例产物，清理运行时扫描目录里的受管旧 sample/模板目录，再重新发布当前官方样例。

样例产物：
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*`（声明 `image_assets` 时）

运行时同步参数：
- `npm run sync:runtime-samples -- --skip-build true`
- `npm run sync:runtime-samples -- --runtime-root /custom/plugin/root`

受管清理范围：
- 当前官方样例/模板 id（模板预设里的 `demo.*.v2`）
- 由此派生的历史官方 id（`demo.*.v1`）
- 非官方自定义插件目录会保留

## 2. `spawn_image` 资源（可选）

`plugin.json` 可声明 `image_assets`：
- 路径相对 `plugin.json`
- 支持格式：`.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`
- `imageId` 通过数组下标映射（越界取模）
- 缺失或非法资源会回退内置图片渲染器

`plugin.json` 还可声明路由与帧调度提示：
- `input_kinds`：可选字符串数组，限制宿主输入通道
  （`click/move/scroll/hold_start/hold_update/hold_end/hover_start/hover_end/indicator_click/indicator_scroll/indicator_key/all`）
- `enable_frame_tick`：可选布尔值，控制宿主是否按帧调用 `mfx_plugin_on_frame`

## 3. 放置插件

按 `plugin.json.id` 建目录：
- Debug 默认：`<exe_dir>/plugins/wasm/<plugin_id>/`
- Release 默认：`%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`
- 额外扫描根：`<exe_dir>/plugins/wasm`（便携兜底）
- Debug 便捷扫描：`examples/wasm-plugin-template/dist`
- 设置页可追加扫描根（`WASM 插件 -> 插件扫描路径`）

至少复制 `effect.wasm` + `plugin.json`；使用 `image_assets` 时要一起复制 `assets/`。

说明：
- 模板默认清单 id 为 `demo.template.default.v2`
- 样例清单位于 `dist/samples/*`，id 形如 `demo.*.<sample>.v2`
- 重复 id 会按扫描根优先级去重；配置的 `catalog_root_path` 会覆盖默认扫描根

## 4. 加载与启用

```bash
POST /api/wasm/load-manifest
{
  "manifest_path": "C:\\path\\to\\plugins\\wasm\\demo.template.default.v2\\plugin.json"
}
```

```bash
POST /api/wasm/enable
```

## 5. ABI 契约

必需导出：
- `mfx_plugin_on_input(input_ptr, input_len, output_ptr, output_cap)`
- `mfx_plugin_on_frame(input_ptr, input_len, output_ptr, output_cap)`

建议导出：
- `mfx_plugin_get_api_version() -> 2`
- `mfx_plugin_reset()`

ABI 定义：
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

模板辅助说明：
- `assembly/common/abi.ts` 提供 `writeSpawnImageAffine(...)`，对应 `spawn_image_affine` 命令。
- `assembly/common/abi.ts` 还提供 `writeSpawnPulse(...)`，对应 `spawn_pulse` 命令（宿主侧 `ripple/star` 通用原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnPolylineHeader(...)` 与 `writeSpawnPolylinePoint(...)`，对应 `spawn_polyline` 命令（可变长度折线原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnPathStrokeHeader(...)`、`writePathStrokeNodeMoveTo(...)`、`writePathStrokeNodeLineTo(...)`、`writePathStrokeNodeQuadTo(...)`、`writePathStrokeNodeCubicTo(...)`、`writePathStrokeNodeClose(...)`，对应 `spawn_path_stroke` 命令（可变长度 path stroke 原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnPathFillHeader(...)` 和 `writeSpawnPathFillHeaderWithSemantics(...)`，对应 `spawn_path_fill` 命令（可变长度 path fill 原语，复用同一套 path 节点写入函数）。
- `assembly/common/abi.ts` 还提供 `writeSpawnRibbonStripHeader(...)`、`writeSpawnRibbonStripHeaderWithSemantics(...)` 与 `writeSpawnRibbonStripPoint(...)`，对应 `spawn_ribbon_strip` 命令（支持每点宽度的可变长度中心线 ribbon/trail strip 原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnGlowBatchHeader(...)` 与 `writeSpawnGlowBatchItem(...)`，对应 `spawn_glow_batch` 命令（面向高密度爆发/喷射的批量发光粒子原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnSpriteBatchHeader(...)` 与 `writeSpawnSpriteBatchItem(...)`，对应 `spawn_sprite_batch` 命令（面向高密度彩屑/贴纸/图片喷发的批量精灵原语）。
- `assembly/common/abi.ts` 还提供 `writeSpawnQuadBatchHeader(...)` 与 `writeSpawnQuadBatchItem(...)`，对应 `spawn_quad_batch` 命令（面向显式宽高图片碎片与 atlas 裁剪的批量 textured-quad 原语）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGlowEmitter(...)` 与 `writeRemoveGlowEmitter(...)`，对应 `upsert_glow_emitter` / `remove_glow_emitter` 命令（跨平台 retained glow emitter 基线能力）。
- `assembly/common/abi.ts` 还提供 `writeUpsertSpriteEmitter(...)` 与 `writeRemoveSpriteEmitter(...)`，对应 `upsert_sprite_emitter` / `remove_sprite_emitter` 命令（跨平台 retained sprite emitter 基线能力，支持 image/fallback 粒子）。
- `assembly/common/abi.ts` 还提供 `writeUpsertParticleEmitter(...)` 与 `writeRemoveParticleEmitter(...)`，对应 `upsert_particle_emitter` / `remove_particle_emitter` 命令（跨平台 retained particle emitter 基线能力，当前支持 `soft_glow / solid_disc` 两种风格）。
- `assembly/common/abi.ts` 还提供 `writeUpsertRibbonTrailHeader(...)`、`writeUpsertRibbonTrailHeaderWithSemantics(...)`、`writeUpsertRibbonTrailPoint(...)` 与 `writeRemoveRibbonTrail(...)`，对应 `upsert_ribbon_trail` / `remove_ribbon_trail` 命令（跨平台 retained ribbon/trail 基线能力，宿主管理中心线 strip 生命周期）。
- `assembly/common/abi.ts` 还提供 `writeUpsertQuadFieldHeader(...)`、`writeUpsertQuadFieldHeaderWithSemantics(...)`、`writeUpsertQuadFieldItem(...)` 与 `writeRemoveQuadField(...)`，对应 `upsert_quad_field` / `remove_quad_field` 命令（跨平台 retained quad field 基线能力，宿主管理 textured-quad 集群生命周期）。
- `assembly/common/abi.ts` 还提供 `writeRemoveGroup(...)`，对应 `remove_group` 命令（只作用于 retained 实例的分组生命周期原语，用于清空当前 manifest 下共享一个 `group_id` 的 retained 实例）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupPresentation(...)`，对应 `upsert_group_presentation` 命令（只作用于 retained 分组展示语义，用于更新当前 manifest 下共享一个 `group_id` 的 retained 实例的 `alpha/visible`）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupClipRect(...)`，对应 `upsert_group_clip_rect` 命令（只作用于 retained 分组裁剪语义，用于更新当前 manifest 下共享一个 `group_id` 的 retained 实例组级裁剪窗口）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupClipRectWithMaskTail(...)`，用于让 `upsert_group_clip_rect` 继续追加可选 group-mask tail（`rect|round_rect|ellipse`，作用在 retained 最终有效裁剪边界上）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupLayer(...)`，对应 `upsert_group_layer` 命令（只作用于 retained 分组层次语义，用于更新当前 manifest 下共享一个 `group_id` 的 retained 实例的 `blend override/sort bias`）。
- `assembly/common/abi.ts` 现在同时提供 `writeUpsertGroupTransform(...)`、`writeUpsertGroupTransformWithTail(...)`、`writeUpsertGroupTransformWithTailAndPivot(...)` 和 `writeUpsertGroupTransformWithTailPivotAndScale2D(...)`，对应 `upsert_group_transform` 命令。基础写法仍然只更新平移偏移 `offset_x_px/offset_y_px`；可选 transform tails 还能继续追加 `rotation_rad + uniform_scale`、`pivot_x_px/pivot_y_px` 以及 `scale_x/scale_y`。当前宿主解释仍然分阶段推进：平移会作用到同一 manifest 下共享 `group_id` 的所有 retained 实例，而 transform tails 带来的几何重算现在会作用于同时启用了 `group_local_origin` 的 retained glow/sprite/particle/ribbon/quad。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupLocalOrigin(...)`，对应 `upsert_group_local_origin` 命令（只作用于 retained 分组局部原点语义，用于为当前 manifest 下共享一个 `group_id` 的 retained 实例组维护一个屏幕坐标原点；具体实例再通过 `...FLAG_USE_GROUP_LOCAL_ORIGIN` 显式切到组内局部坐标系）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupMaterial(...)`、`writeUpsertGroupMaterialWithStyle(...)`、`writeUpsertGroupMaterialWithStyleAndResponse(...)`、`writeUpsertGroupMaterialWithStyleResponseAndFeedback(...)`、`writeUpsertGroupMaterialWithAllTails(...)` 和 `writeUpsertGroupMaterialWithFullTails(...)`，对应 `upsert_group_material` 命令（只作用于 retained 分组材质语义，用于更新当前 manifest 下共享一个 `group_id` 的 retained glow/sprite/particle/ribbon/quad 的宿主拥有 `tint override + intensity multiplier`、可选的宿主拥有 style tail：`soft_bloom_like|afterimage_like + style_amount`、可选的宿主拥有 response tail：`diffusion_amount|persistence_amount`、可选的宿主拥有 feedback tail：`echo_amount|echo_drift_px`、可选的宿主拥有 feedback-mode tail：`directional|tangential|swirl + phase_rad`，以及可选的宿主拥有 feedback-stack tail：`echo_layers + echo_falloff`）。
- `assembly/common/abi.ts` 还提供 `writeUpsertGroupPass(...)`、`writeUpsertGroupPassWithMode(...)`、`writeUpsertGroupPassWithModeAndStack(...)`、`writeUpsertGroupPassWithAllTails(...)`、`writeUpsertGroupPassWithFullTails(...)`、`writeUpsertGroupPassWithRoutingTails(...)`、`writeUpsertGroupPassWithLaneResponseTails(...)`、`writeUpsertGroupPassWithTemporalTails(...)`、`writeUpsertGroupPassWithTemporalModeTails(...)`、`writeUpsertGroupPassWithTertiaryTails(...)`、`writeUpsertGroupPassWithTertiaryRoutingTails(...)`、`writeUpsertGroupPassWithTertiaryLaneResponseTails(...)`、`writeUpsertGroupPassWithTertiaryTemporalTails(...)`、`writeUpsertGroupPassWithTertiaryTemporalModeTails(...)` 与 `writeUpsertGroupPassWithTertiaryStackTails(...)`，对应 `upsert_group_pass` 命令（只作用于 retained 分组受控 pass 语义，用于更新当前 manifest 下共享一个 `group_id` 的 retained glow/sprite/particle/ribbon/quad 的宿主拥有 `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`，以及可选的 `directional|tangential|swirl + phase_rad` mode tail、`echo_layers + echo_falloff` stack tail、`secondary_pass_kind + secondary_pass_amount + secondary_response_amount` pipeline tail、`secondary_blend_mode(multiply|lerp) + secondary_blend_weight` blend tail、`secondary_route_mask(glow|sprite|particle|ribbon|quad)` routing tail、`secondary_glow|sprite|particle|ribbon|quad_response` lane-response tail、`phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor` temporal tail、`exponential|linear|pulse + temporal_strength` temporal-mode tail、`tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight` tertiary-stage tail、`tertiary_route_mask(glow|sprite|particle|ribbon|quad)` tertiary-routing tail、`tertiary_glow|sprite|particle|ribbon|quad_response` tertiary lane-response tail、`tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor` tertiary temporal tail、`tertiary_temporal_mode + tertiary_temporal_strength` tertiary temporal-mode tail 和 `tertiary_echo_layers + tertiary_echo_falloff` tertiary stack tail）。
- `assembly/common/abi.ts` 还提供 `writeUpsertParticleEmitterWithSpawnTail(...)`、`writeUpsertParticleEmitterWithDynamicsTail(...)`、`writeUpsertParticleEmitterWithSpawnAndDynamicsTailsAndSemantics(...)`、`writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(...)`，用于 retained particle emitter 的显式 `cone|radial` 发射、`point|disc|ring` 生成形状、`drag/turbulence`，以及 `size/alpha/color over life`。
- `assembly/common/abi.ts` 还提供 `writeSpawnPathStrokeHeaderWithSemantics(...)`、`writeSpawnPathFillHeaderWithSemantics(...)`、`writeSpawnGlowBatchHeaderWithSemantics(...)`、`writeSpawnSpriteBatchHeaderWithSemantics(...)`、`writeUpsertGlowEmitterWithSemantics(...)`、`writeUpsertSpriteEmitterWithSemantics(...)`、`writeUpsertParticleEmitterWithSemantics(...)`、`writeUpsertRibbonTrailHeaderWithSemantics(...)`、`writeUpsertQuadFieldHeaderWithSemantics(...)`，用于追加可选共享渲染尾部（`blend_mode/sort_key/group_id`）。
- `assembly/common/abi.ts` 还提供 `writeSpawnPathStrokeHeaderWithSemanticsAndClip(...)`、`writeSpawnPathFillHeaderWithSemanticsAndClip(...)`、`writeSpawnRibbonStripHeaderWithSemanticsAndClip(...)`，用于在 path 通道里继续追加可选 `clip_rect` 尾部。
  - 样例覆盖：`click-path-fill-clip-window` 负责 `spawn_path_fill + clip_rect`，`click-path-clip-lanes` 负责补齐 `spawn_path_stroke + clip_rect` 和 `spawn_ribbon_strip + clip_rect`。
- `assembly/common/abi.ts` 还提供 `writeSpawnSpriteBatchHeaderWithSemanticsAndClip(...)`、`writeSpawnQuadBatchHeaderWithSemanticsAndClip(...)`、`writeUpsertQuadFieldHeaderWithSemanticsAndClip(...)`，用于让 sprite/quad 几何通道也能追加同一套 `clip_rect` 尾部。
- `assembly/common/abi.ts` 还提供 `writeUpsertGlowEmitterWithSemanticsAndClip(...)`、`writeUpsertSpriteEmitterWithSemanticsAndClip(...)`、`writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(...)`，用于让 retained emitter 通道也能追加同一套 `clip_rect` 尾部。
- `assembly/common/abi.ts` 还提供 `writeUpsertRibbonTrailHeaderWithSemanticsAndClip(...)`，用于让 retained ribbon trail 通道也能追加同一套 `clip_rect` 尾部。

## 6. 内置样例 Key

- `text-rise`
- `text-burst`
- `text-spiral`
- `text-wave-chain`
- `image-pulse`
- `image-burst`
- `image-lift`
- `mixed-text-image`
- `mixed-emoji-celebrate`
- `button-adaptive`
- `click-pulse-dual`
- `click-polyline-zigzag`
- `click-path-stroke-ribbon`
- `click-path-fill-badge`
- `click-path-fill-clip-window`
- `click-path-clip-lanes`
- `click-ribbon-trace`
- `click-glow-burst`
- `click-sprite-burst`
- `click-quad-atlas-burst`
- `click-retained-glow-field`
- `click-retained-sprite-fountain`
- `click-retained-particle-field`
- `click-retained-ribbon-trail`
- `click-retained-quad-field`
- `click-retained-group-clear`
- `click-retained-group-alpha`
- `click-retained-group-clip`
- `click-retained-group-layer`
- `click-retained-group-mask`
- `click-retained-group-transform`
- `click-retained-group-local-origin`
- `click-retained-group-material`
- `click-retained-group-pass`
- `move-stream-sparks`
- `scroll-particle-burst`
- `scroll-neon-burst`
- `hold-orbit-pulse`
- `hover-spark-ring`

模板详情：
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`

## 7. 排错

- `load-manifest` 失败：检查 `entry` 与文件路径。
- 无运行时输出：确认 `mfx_wasm_runtime.dll` 是否已构建。
- 找不到运行时桥接：宿主会回退 Null runtime。
- 输出被丢弃：查看 `/api/state` 的 `wasm` 预算/诊断字段。
- `click-retained-group-clear` 这类分组 retained 样例一次输入会输出多条命令，需要 `runtime_max_commands >= 3`。
- `click-retained-group-alpha` 左键一次会输出 4 条命令（`upsert_group_presentation + glow + ribbon + quad`），所以需要 `runtime_max_commands >= 4`。
- `click-retained-group-clip`、`click-retained-group-layer` 和 `click-retained-group-transform` 左键一次也都会输出 4 条命令，所以同样需要 `runtime_max_commands >= 4`。
- `click-retained-group-mask` 左键一次也会输出 4 条命令（`upsert_group_clip_rect + mask tail + glow + ribbon + quad`），所以同样需要 `runtime_max_commands >= 4`。
- `click-retained-group-local-origin` 左键一次会输出 7 条命令（`upsert_group_local_origin + upsert_group_transform + glow + sprite + particle + ribbon + quad`），所以需要 `runtime_max_commands >= 7`。
- `click-retained-group-material` 左键一次会输出 5 条命令（`glow + sprite + particle + ribbon + quad`），所以需要 `runtime_max_commands >= 5`；中键更新时还会额外输出 1 条 `upsert_group_material`，并同时携带新的可选 style tail、response tail、feedback tail、feedback-mode tail 和 feedback-stack tail。
- `click-retained-group-pass` 左键一次会输出 5 条命令（`glow + sprite + particle + ribbon + quad`），所以需要 `runtime_max_commands >= 5`；中键更新时还会额外输出 1 条 `upsert_group_pass`。
