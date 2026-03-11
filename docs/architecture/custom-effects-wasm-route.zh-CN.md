# 自定义特效（WASM 路线）

## 目标
定义稳定的 WASM 架构契约：
- 插件只负责“逻辑计算”；
- C++ 宿主负责“渲染执行与资源控制”；
- 先保证行为稳定和回退安全，再做视觉优化。

本文件只保留高价值架构信息；历史阶段过程下沉到 issue 文档。

## 范围
- 包含：
  - 点击文字/图片类自定义逻辑，
  - 宿主预算与回退机制，
  - 设置页策略与诊断，
  - 模板与本地编译流程。
- 不包含：
  - 运行时“任意 JS 直转 WASM”，
  - 插件直接控制窗口/交换链，
  - 可视化节点编辑器。

## 数据流
1. 宿主标准化输入事件。
2. `WasmEffectHost` 调用插件 `on_input`。
3. 宿主按帧调用 `on_frame` 获取持续动画输出。
4. 插件返回命令缓冲。
5. 宿主做预算校验和命令校验。
6. 宿主执行命令；异常时走回退路径。

核心原则：WASM 计算，C++ 渲染。

Surface 宿主路由规则：
- 宿主在同一套运行时策略（`enabled/budget`）下维护两条独立 wasm host 通道：
  - `effects` 通道：处理 click/move/scroll/hold/hover 特效插件。
  - `indicator` 通道：处理 `indicator_*` 键鼠指示插件。
- `POST /api/wasm/load-manifest` 必须按 `surface` 路由到目标通道，并基于该通道诊断状态做成功判定。

## ABI v2 契约
插件导出（前两项必选）：

```c
uint32_t mfx_plugin_get_api_version(void);
uint32_t mfx_plugin_on_input(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
uint32_t mfx_plugin_on_frame(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
void mfx_plugin_reset(void); // 推荐实现
```

事件类型遵循宿主标准语义（`click/move/scroll/hold*/hover*`），并新增指示器通道
`indicator_click/indicator_scroll/indicator_key` 供键鼠指示器样式使用。

指示器事件补充说明：
- `indicator_*` 仍以 `EventInputV2` 作为基础输入。
- 宿主会在 `EventInputV2` 后追加 `IndicatorEventTailV1`（4 字节），携带
  `size_px` 与 `duration_ms`。
- 若宿主带有更完整的指示器上下文，还会继续在 `IndicatorEventTailV1` 之后追加
  `IndicatorEventContextTailV2`，提供 `primary_code`、`streak`、
  `modifier_mask` 与 `detail_flags`。其中 `primary_code` 会按事件类型映射为
  按键编号 / 滚轮幅度 / `vkCode`。
- 若需要显示键鼠标签，使用 `text_id = kTextIdEventLabel` 即可取到当前事件标签
  （例如 `L x2`、`W+ x3`、`Cmd+Tab`）。

Manifest 路由提示（可选）：
- `input_kinds` 用于收窄触发 `on_input` 的输入通道。
- `surfaces` 用于声明插件面向的宿主表面（`effects` / `indicator` / `all`）。
- `enable_frame_tick` 用于控制宿主是否周期性驱动 `on_frame`。

指示器路由可观测性（测试/运行时）：
- `POST /api/wasm/test-dispatch-app-indicator-key` 会走真实应用分发路径
  （`DispatchInputIndicatorKey`）并返回最新路由快照。
- 快照同时给出分支布尔位（`route_attempted`、`event_supported`、
  `rendered_by_wasm`、`native_fallback_applied`）和原因码
  （`wasm_rendered`、`fallback_disabled`、`event_not_supported`、
  `plugin_unloaded`、`anchor_unavailable`、`invoke_failed_no_output`）。
- 在出现至少一次 wasm 路由尝试后，`/api/state` 也会在
  `input_indicator_wasm_route_status` 输出同一份最新快照。

## 命令契约（当前）
当前生产路径支持：
- `spawn_text`
- `spawn_image`
- `spawn_image_affine`（前缀保持 `spawn_image` 字段，尾部追加仿射元数据）
- `spawn_pulse`（通用点击脉冲原语：`ripple` / `star`）
- `spawn_polyline`（通用折线/多段线原语，支持可变长度点集）
- `spawn_path_stroke`（通用 path stroke 原语，支持 move/line/quad/cubic/close 节点流）
- `spawn_path_fill`（通用 path fill 原语，复用同一套节点流，并支持 `non_zero/even_odd` 填充规则）
- `spawn_ribbon_strip`（通用中心线 ribbon/trail strip 原语，支持每点宽度）
- `spawn_glow_batch`（通用批量发光粒子原语，面向高密度喷射/爆发类效果）
- `spawn_sprite_batch`（通用批量图片精灵原语，面向高密度彩屑/徽章/贴图喷发）
- `spawn_quad_batch`（通用批量 textured-quad 原语，支持显式宽高与可选 atlas UV 裁剪）
- `upsert_glow_emitter`（跨平台 retained glow emitter 基线能力，用于持续更新同一个 emitter）
- `remove_glow_emitter`（移除指定 retained glow emitter）
- `upsert_sprite_emitter`（跨平台 retained sprite emitter 基线能力，用于持续更新同一个图片/回退 sprite emitter）
- `remove_sprite_emitter`（移除指定 retained sprite emitter）
- `upsert_particle_emitter`（跨平台通用 retained particle emitter 基线能力，当前支持 soft-glow / solid-disc 两种粒子风格）
- `remove_particle_emitter`（移除指定 retained particle emitter）
- `upsert_ribbon_trail`（跨平台 retained ribbon/trail 基线能力，用于维护一条持续存在的中心线带状轨迹）
- `remove_ribbon_trail`（移除指定 retained ribbon trail）
- `upsert_quad_field`（跨平台 retained quad field 基线能力，用于维护一组持续存在的 textured-quad）
- `remove_quad_field`（移除指定 retained quad field）
- `remove_group`（在当前 active manifest 下，按 `group_id` 清空一组 retained 实例）
- `upsert_group_presentation`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的 `alpha_multiplier/visible` 展示语义）
- `upsert_group_clip_rect`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的组级裁剪；当前宿主语义是 `instance_clip ∩ group_clip` 作用到 retained glow/sprite/particle/ribbon/quad，并支持可选 group-mask tail：`rect|round_rect|ellipse`）
- `upsert_group_layer`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的层次语义；当前宿主语义是 `blend override + sort bias` 作用到 retained glow/sprite/particle/ribbon/quad）
- `upsert_group_transform`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的组级变换；16 字节基础头仍然只携带平移 `offset_x_px/offset_y_px`，第一段可选 tail 还能追加 `rotation_rad + uniform_scale`，第二段可选 tail 还能继续追加 `pivot_x_px/pivot_y_px`，第三段可选 tail 还能继续追加 `scale_x/scale_y`；当前宿主解释仍分阶段推进，平移作用到 retained glow/sprite/particle/ribbon/quad，而几何级旋转/缩放现在会作用到同时启用了 `...FLAG_USE_GROUP_LOCAL_ORIGIN` 的 retained glow/sprite/particle/ribbon/quad）
- `upsert_group_local_origin`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的组内局部原点；当前宿主语义是维护一个屏幕坐标原点，并只让显式声明了 `...FLAG_USE_GROUP_LOCAL_ORIGIN` 的 retained glow/sprite/particle/ribbon/quad 实例消费它）
- `upsert_group_material`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的材质语义；当前宿主语义刻意保持收敛，只开放宿主拥有的 `tint override + intensity multiplier`、可选的宿主拥有 style tail：`soft_bloom_like|afterimage_like`、可选的宿主拥有 response tail：`diffusion_amount|persistence_amount`、可选的宿主拥有 feedback tail：`echo_amount|echo_drift_px`、可选的宿主拥有 feedback-mode tail：`directional|tangential|swirl + phase_rad`，以及可选的宿主拥有 feedback-stack tail：`echo_layers + echo_falloff`，并只作用到 retained glow/sprite/particle/ribbon/quad）
- `upsert_group_pass`（在当前 active manifest 下，按 `group_id` 更新一组 retained 实例的受控 pass 语义；当前宿主语义刻意保持收敛，只开放宿主拥有的 `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`、可选的 mode tail：`directional|tangential|swirl + phase_rad`、可选的 stack tail：`echo_layers + echo_falloff`、可选的 pipeline tail：`secondary_pass_kind + secondary_pass_amount + secondary_response_amount`、可选的 blend tail：`secondary_blend_mode(multiply|lerp) + secondary_blend_weight`、可选的 routing tail：`secondary_route_mask(glow|sprite|particle|ribbon|quad)`、可选的 lane-response tail：`secondary_glow|sprite|particle|ribbon|quad_response`、可选的 temporal tail：`phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor`、可选的 temporal-mode tail：`exponential|linear|pulse + temporal_strength`、可选的 tertiary-stage tail：`tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight`、可选的 tertiary-routing tail：`tertiary_route_mask(glow|sprite|particle|ribbon|quad)`、可选的 tertiary lane-response tail：`tertiary_glow|sprite|particle|ribbon|quad_response`、可选的 tertiary temporal tail：`tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor`、可选的 tertiary temporal-mode tail：`tertiary_temporal_mode + tertiary_temporal_strength`，以及可选的 tertiary stack tail：`tertiary_echo_layers + tertiary_echo_falloff`，并只重新作用到 retained glow/sprite/particle/ribbon/quad，不暴露 raw shader/post-process 控制）

通用字段：
- 变换：`x, y, scale, rotation`
- 运动：`vx, vy, ax, ay`
- 样式：`alpha, color`
- 生命周期：`delay_ms, life_ms`
- 资源：`text_id`/`image_id`

`spawn_pulse` 专属字段：
- 几何：`x, y, start_radius_px, end_radius_px, stroke_width_px`
- 样式：`alpha, fill_argb, stroke_argb, glow_argb`
- 生命周期：`delay_ms, life_ms`
- 渲染器选择：`pulse_kind = ripple | star`

`spawn_polyline` 专属字段：
- 几何：`point_count`、`points[]`、`line_width_px`
- 样式：`alpha`、`stroke_argb`、`glow_argb`
- 生命周期：`delay_ms`、`life_ms`
- 标志位：`closed`

`spawn_glow_batch` 专属字段：
- 批头：`item_count`、`flags`、`delay_ms`、`life_ms`
- 单粒子几何：`x`、`y`、`radius_px`
- 单粒子样式：`alpha`、`color_argb`
- 单粒子运动：`vx`、`vy`、`ax`、`ay`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在 item payload 后）：`blend_mode`、`sort_key`、`group_id`

`spawn_path_stroke` 专属字段：
- 路径头：`node_count`、`line_width_px`、`alpha`、`delay_ms`、`life_ms`
- 样式：`stroke_argb`、`glow_argb`
- 描边风格：`line_join`、`line_cap`
- 节点流：`move_to | line_to | quad_to | cubic_to | close`
- 可选渲染尾部（追加在节点 payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前只在 path 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`spawn_path_fill` 专属字段：
- 路径头：`node_count`、`alpha`、`glow_width_px`、`delay_ms`、`life_ms`
- 样式：`fill_argb`、`glow_argb`
- 填充风格：`fill_rule`
- 节点流：`move_to | line_to | quad_to | cubic_to | close`
- 可选渲染尾部（追加在节点 payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前只在 path 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`spawn_ribbon_strip` 专属字段：
- strip 头：`point_count`、`alpha`、`glow_width_px`、`delay_ms`、`life_ms`
- 样式：`fill_argb`、`glow_argb`
- 点流：`x`、`y`、`width_px`
- flags：`closed`
- 可选渲染尾部（追加在 point payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前只在 path 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`spawn_sprite_batch` 专属字段：
- 批头：`item_count`、`flags`、`delay_ms`、`life_ms`
- 单项几何：`x`、`y`、`scale`
- 单项样式：`alpha`、`rotation`、`tint_argb`
- 单项资源：`image_id`
- 单项运动：`vx`、`vy`、`ax`、`ay`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在 item payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前在 macOS sprite/quad 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`spawn_quad_batch` 专属字段：
- 批头：`item_count`、`flags`、`delay_ms`、`life_ms`
- 单项几何：`x`、`y`、`width_px`、`height_px`
- 单项样式：`alpha`、`rotation`、`tint_argb`
- 单项资源：`image_id`
- 单项 atlas 区域：`src_u0`、`src_v0`、`src_u1`、`src_v1`
- 单项运动：`vx`、`vy`、`ax`、`ay`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在 item payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前在 macOS sprite/quad 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`upsert_glow_emitter` 专属字段：
- emitter 身份/锚点：`emitter_id`、`x`、`y`
- 发射：`emission_rate_per_sec`、`direction_rad`、`spread_rad`
- 粒子速度/尺寸：`speed_min`、`speed_max`、`radius_min_px`、`radius_max_px`
- 粒子样式：`alpha_min`、`alpha_max`、`color_argb`
- 生命周期/预算：`acceleration_x`、`acceleration_y`、`emitter_ttl_ms`、`particle_life_ms`、`max_particles`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在固定 payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前在 macOS retained emitter 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`remove_glow_emitter` 专属字段：
- `emitter_id`

`upsert_sprite_emitter` 专属字段：
- emitter 身份/锚点：`emitter_id`、`image_id`、`x`、`y`
- 发射：`emission_rate_per_sec`、`direction_rad`、`spread_rad`
- 粒子速度/尺寸：`speed_min`、`speed_max`、`size_min_px`、`size_max_px`
- 粒子样式：`alpha_min`、`alpha_max`、`tint_argb`、`rotation_min_rad`、`rotation_max_rad`
- 生命周期/预算：`acceleration_x`、`acceleration_y`、`emitter_ttl_ms`、`particle_life_ms`、`max_particles`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在固定 payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前在 macOS retained emitter 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`remove_sprite_emitter` 专属字段：
- `emitter_id`

`upsert_particle_emitter` 专属字段：
- emitter 身份/锚点：`emitter_id`、`x`、`y`
- 发射：`emission_rate_per_sec`、`direction_rad`、`spread_rad`
- 粒子速度/尺寸：`speed_min`、`speed_max`、`radius_min_px`、`radius_max_px`
- 粒子样式：`alpha_min`、`alpha_max`、`color_argb`、`particle_style = soft_glow | solid_disc`
- 生命周期/预算：`acceleration_x`、`acceleration_y`、`emitter_ttl_ms`、`particle_life_ms`、`max_particles`
- 可选 curve tail（追加在固定 payload 后、共享渲染尾部前）：`size_start_scale`、`size_end_scale`、`alpha_start_scale`、`alpha_end_scale`、`color_start_argb`、`color_end_argb`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在固定 payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前在 macOS retained emitter 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`

`remove_particle_emitter` 专属字段：
- `emitter_id`

`upsert_ribbon_trail` 专属字段：
- trail 身份：`trail_id`
- strip 几何/生命周期：`point_count`、`alpha`、`glow_width_px`、`life_ms`
- 样式：`fill_argb`、`glow_argb`
- 点流：`x`、`y`、`width_px`
- flags：`closed`
- 可选渲染尾部（追加在 point payload 后）：`blend_mode`、`sort_key`、`group_id`

`remove_ribbon_trail` 专属字段：
- `trail_id`

`upsert_quad_field` 专属字段：
- field 身份/生命周期：`field_id`、`item_count`、`ttl_ms`
- 单项几何：`x`、`y`、`width_px`、`height_px`
- 单项样式：`alpha`、`rotation`、`tint_argb`
- 单项资源：`image_id`
- 单项 atlas 区域：`src_u0`、`src_v0`、`src_u1`、`src_v1`
- 单项运动：`vx`、`vy`、`ax`、`ay`
- 可选渲染尾部（追加在 item payload 后）：`blend_mode`、`sort_key`、`group_id`
- 可选 `clip_rect` 尾部（当前只在 macOS retained quad 通道解释，追加在渲染尾部之后）：`left_px`、`top_px`、`width_px`、`height_px`
- 旧标志位：`screen_blend`
- 可选渲染尾部（追加在 item payload 后）：`blend_mode`、`sort_key`、`group_id`

`remove_quad_field` 专属字段：
- `field_id`

说明：
- ABI 已升级到 v2，但命令结构体命名仍保留 `*CommandV1`，用于保持二进制布局连续性。
- `spawn_image_affine` 当前在宿主侧主要用于图片平移/缩放/旋转语义映射。
- `spawn_pulse` 复用了宿主现有点击脉冲渲染器，而不是再新增一条特效专用引擎路径；WASM 以后可以自行组合多层延迟脉冲/星形，而不必每次继续改原生路由。
- `spawn_polyline` 是宿主当前第一条“可变长度”绘图命令，目的是让闪电、轨迹、轮廓线、后续拖尾类自定义效果可以直接由 WASM 组织，而不必再新增一条内置效果通道。
- `spawn_path_stroke` 把同样的命令缓冲模式扩展到了 path/vector 描边组合，目的是让丝带、曲线闪电、符文轮廓、闭合描边路径这类效果直接由 WASM 组织，而不必继续增加特化宿主命令。
- `spawn_path_fill` 则把同一套 path/vector 面扩展到了填充图形与镂空图形，目的是让徽章、法阵、blob、带内孔的 ring/seal 这类效果也能直接由 WASM 组织，而不必把图形形状再次固化回宿主枚举。
- `spawn_ribbon_strip` 则把这套公共面推进到“中心线 + 每点宽度”的连续带状几何，目的是让丝带拖尾、流光线、挥舞痕迹、能量束这类效果直接由 WASM 组织，而不需要立刻跳到 retained mesh/material 契约。
- `spawn_glow_batch` 是宿主当前第一条“批量粒子”原语，目的是让喷射、爆裂、余烬、发光尘埃等高密度效果直接由 WASM 组织，而不是继续退回到图片专用批处理。
- `spawn_sprite_batch` 把同样的批处理模式扩展到图片粒子，目的是让彩屑、贴纸、徽章、emoji/图片喷发等高密度精灵效果直接由 WASM 组织，而不是一张图就起一个 overlay。
- `spawn_quad_batch` 则把这条 transient 图片通路从“按 scale 的精灵”推进到“显式宽高 + atlas 裁剪的 quad”，为 atlas 徽章、裁切卡片、拉伸贴片，以及后续 `quad/atlas/ribbon` 路线提供第一个可复用公共基线，而不需要直接开放 raw mesh/shader 契约。
- `upsert_glow_emitter/remove_glow_emitter` 是宿主当前第一组跨平台 retained 生命周期命令，目标是让 WASM 能维护持续存在、可更新、可显式移除的 glow emitter，而不是只能靠瞬时 burst 命令不断回灌。
- `upsert_sprite_emitter/remove_sprite_emitter` 则把 retained 生命周期面扩到了 image-backed sprite；当图片资源缺失时，宿主会降级成 tinted orb-style fallback，而不是直接丢弃命令。
- `upsert_particle_emitter/remove_particle_emitter` 则把 retained 生命周期面第一次收敛到更通用的粒子原语上。v1 刻意只开放 `soft_glow / solid_disc` 两种形状风格，但已经复用了同一套宿主 retained 生命周期、计数器和共享渲染尾部。
- `upsert_ribbon_trail/remove_ribbon_trail` 则把 retained 生命周期面第一次扩展到“非粒子几何”上。v1 刻意只保留宿主拥有的中心线 strip + 每点宽度 + 按 TTL 淡出行为，并继续复用同一套共享渲染尾部。
- `upsert_quad_field/remove_quad_field` 则把 retained 生命周期面扩展到“非粒子 textured 几何”上。v1 刻意只保留宿主拥有的 quad 集群 + 每项 atlas 裁剪 + 共享 TTL 运动行为，并继续复用同一套共享渲染尾部。
- particle emitter 现在还支持一个很小的可选 spawn tail，用来表达 `cone|radial` 发射模式和 `point|disc|ring` 生成形状；其后还可追加 dynamics tail 来表达 `drag/turbulence`，再追加 curve tail 控制 `size/alpha/color over life`。这样固定 ABI 不用再改，但 WASM 已经能开始做更接近原生粒子系统的“生成形状 + 动力学 + 生命周期曲线”组织。
- `spawn_path_stroke`、`spawn_path_fill`、`spawn_ribbon_strip`、`spawn_glow_batch`、`spawn_sprite_batch`、`spawn_quad_batch`、`upsert_glow_emitter`、`upsert_sprite_emitter`、`upsert_particle_emitter`、`upsert_ribbon_trail`、`upsert_quad_field` 现在还支持可选共享渲染尾部：在不破坏 v2 基础布局的前提下追加 `blend_mode/sort_key/group_id`。
- 当前宿主对这三个字段的解释仍然刻意保持最小：`blend_mode=screen|add` 都走 screen-like 合成；`sort_key` 驱动 Windows ripple overlay 排序和 macOS overlay window level；`group_id` 现在已经同时作为 `remove_group`、`upsert_group_presentation`、`upsert_group_clip_rect`、`upsert_group_layer`、`upsert_group_transform` 和 `upsert_group_material` 的 retained 分组键投入使用。
- `spawn_path_stroke`、`spawn_path_fill`、`spawn_ribbon_strip` 现在还支持在共享渲染尾部之后继续追加第二个可选 `clip_rect` 尾部；当前 phase1 的宿主解释也刻意保持很窄，只在 macOS path 通道里把它当作屏幕空间矩形裁剪。
- `spawn_sprite_batch`、`spawn_quad_batch`、`upsert_quad_field` 现在也支持同样的可选 `clip_rect` 尾部；当前 phase2 的宿主解释仍然刻意保持很窄，只在 macOS 的 sprite/quad 瞬时通道和 retained quad field 通道里把它当作屏幕空间矩形裁剪。
- `upsert_glow_emitter`、`upsert_sprite_emitter`、`upsert_particle_emitter` 现在也支持同样的可选 `clip_rect` 尾部；当前 phase3 的宿主解释仍然刻意保持很窄，只在 macOS 的 retained emitter 通道里把它当作屏幕空间矩形裁剪。
- `remove_group` 在 v2 里只作用于 retained 实例。它会清掉当前 manifest 下共享同一个 `group_id` 的 glow/sprite/particle/ribbon/quad retained 实例，不会回收瞬时 `spawn_*` overlay。
- `upsert_group_presentation` 在 v2 里也只作用于 retained 实例。当前宿主解释仍然刻意保持很窄：它只维护每组 retained 的 `alpha_multiplier/visible` 状态，并把这个状态应用到当前和未来的 glow/sprite/particle/ribbon/quad retained 实例，不影响瞬时 `spawn_*` overlay。
- `upsert_group_clip_rect` 现在还支持一个可选 group-mask tail。当前宿主解释仍然刻意保持很窄：retained 通道会先算出最终有效裁剪矩形，再可选地把这一组 retained 实例限制到 `rect|round_rect|ellipse` 这三种 mask 里。Windows 仍保持 compile/runtime parity，但暂时还不会出现可见的 group-mask 效果。
- `upsert_group_layer` 在 v2 里也只作用于 retained 实例。当前宿主解释仍然刻意保持很窄：它只维护每组 retained 的 `blend override/sort bias` 状态，并把这个状态应用到当前和未来的 glow/sprite/particle/ribbon/quad retained 实例，不影响瞬时 `spawn_*` overlay。
- `upsert_group_transform` 在 v2 里也只作用于 retained 实例。它的 16 字节基础头仍然只维护每组 retained 的平移偏移 `offset_x_px/offset_y_px`，但现在允许通过可选 transform tails 继续追加 `rotation_rad + uniform_scale`、`pivot_x_px/pivot_y_px` 和 `scale_x/scale_y` 而不改变 command kind。当前宿主解释依旧刻意保持收敛：平移会作用到当前和未来的 glow/sprite/particle/ribbon/quad retained 实例，而几何级旋转/缩放现在会作用于同时启用了 `group_local_origin` 的 retained glow/sprite/particle/ribbon/quad，不影响瞬时 `spawn_*` overlay。
- `upsert_group_local_origin` 在 v2 里也只作用于 retained 实例。当前宿主解释仍然刻意保持很窄：它只维护每组 retained 的屏幕坐标原点，但只有显式打开 `...FLAG_USE_GROUP_LOCAL_ORIGIN` 的 retained 命令才会把自己的坐标解释为组内局部坐标。这样不会重解释现有 grouped sample 的绝对坐标，却给新的 retained glow/sprite/particle/ribbon/quad 实例提供了第一版 group-local 坐标系。
- `upsert_group_material` 在 v6 里也只作用于 retained 实例。当前宿主解释仍然刻意保持很窄：基础 v1 只维护每组 retained 的 `tint override + intensity multiplier` 状态；可选 v2 style tail 还能追加宿主拥有的 `soft_bloom_like|afterimage_like + style_amount`；可选 v3 response tail 还能继续追加宿主拥有的 `diffusion_amount|persistence_amount`；可选 v4 feedback tail 还能继续追加宿主拥有的 `echo_amount|echo_drift_px`；可选 v5 feedback-mode tail 还能继续追加宿主拥有的 `directional|tangential|swirl + phase_rad`；可选 v6 feedback-stack tail 还能继续追加宿主拥有的 `echo_layers + echo_falloff`。宿主仍然只是通过现有 retained upsert 链重算 glow/sprite/particle/ribbon/quad 的颜色/尺寸/边缘光宽度、life/ttl 与受控 echo-like 漂移等参数，不影响瞬时 `spawn_*` overlay，也不把 raw shader/material 自由直接暴露给 wasm。
- `click-retained-group-clear` 这类多命令样例需要 `runtime_max_commands >= 3`；`click-retained-group-alpha`、`click-retained-group-clip`、`click-retained-group-layer`、`click-retained-group-mask` 和 `click-retained-group-transform` 需要 `runtime_max_commands >= 4`；`click-retained-group-local-origin` 现在需要 `runtime_max_commands >= 7`，因为左键会同时 upsert grouped `glow + sprite + particle + ribbon + quad`。core HTTP 合同回归会先把运行时策略提到足够的命令预算，再验证 grouped upsert/dim/remove 行为。
- 当前 retained 公共面已经包含 glow emitter、sprite emitter、generic particle emitter、ribbon trail 和 quad field 五条基线；更高阶的 material/post-process retained 契约仍留给 v3 后续扩展。

## 预算与回退
默认预算：
- `max_execution_ms <= 1.0`
- `max_commands <= 256`
- `output_buffer_bytes` 受策略约束

回退规则：
- 超时：丢弃本次输出
- 超量：截断命令
- 连续失败：禁用插件路径并回退内置效果

可观测字段：
- `wasm.runtime_backend`
- `wasm.runtime_fallback_reason`

## 当前状态
- 运行时路径、诊断、回退契约已接通。
- 策略字段（`enabled`、`manifest_path`、`fallback_to_builtin_click`、预算）已持久化。
- 模板生态与构建脚本已落在：
  - `examples/wasm-plugin-template`

## 关联文档（权威入口）
- 快速开始：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- 兼容策略：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-compatibility.zh-CN.md`
- 排错手册：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`
- 历史阶段拆分文档已从活跃文档层移除，以降低 token 成本。
- 如需追溯细节，请直接查看本文件及相关 WASM 提交的 git 历史。
