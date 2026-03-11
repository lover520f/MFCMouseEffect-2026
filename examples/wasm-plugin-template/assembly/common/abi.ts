export const API_VERSION: u32 = 2;

export const EVENT_INPUT_BYTES: u32 = 28;
export const FRAME_INPUT_BYTES: u32 = 24;
export const INDICATOR_EVENT_TAIL_BYTES: u32 = 4;
export const INDICATOR_EVENT_CONTEXT_TAIL_BYTES: u32 = 8;
export const EVENT_TEXT_ID_INPUT_LABEL: u32 = 0xFFFFFFFF;

export const COMMAND_KIND_SPAWN_TEXT: u16 = 1;
export const COMMAND_KIND_SPAWN_IMAGE: u16 = 2;
export const COMMAND_KIND_SPAWN_IMAGE_AFFINE: u16 = 3;
export const COMMAND_KIND_SPAWN_PULSE: u16 = 4;
export const COMMAND_KIND_SPAWN_POLYLINE: u16 = 5;
export const COMMAND_KIND_SPAWN_GLOW_BATCH: u16 = 6;
export const COMMAND_KIND_SPAWN_SPRITE_BATCH: u16 = 7;
export const COMMAND_KIND_UPSERT_GLOW_EMITTER: u16 = 8;
export const COMMAND_KIND_REMOVE_GLOW_EMITTER: u16 = 9;
export const COMMAND_KIND_SPAWN_PATH_STROKE: u16 = 10;
export const COMMAND_KIND_SPAWN_PATH_FILL: u16 = 11;
export const COMMAND_KIND_UPSERT_SPRITE_EMITTER: u16 = 12;
export const COMMAND_KIND_REMOVE_SPRITE_EMITTER: u16 = 13;
export const COMMAND_KIND_UPSERT_PARTICLE_EMITTER: u16 = 14;
export const COMMAND_KIND_REMOVE_PARTICLE_EMITTER: u16 = 15;
export const COMMAND_KIND_SPAWN_QUAD_BATCH: u16 = 16;
export const COMMAND_KIND_SPAWN_RIBBON_STRIP: u16 = 17;
export const COMMAND_KIND_UPSERT_RIBBON_TRAIL: u16 = 18;
export const COMMAND_KIND_REMOVE_RIBBON_TRAIL: u16 = 19;
export const COMMAND_KIND_UPSERT_QUAD_FIELD: u16 = 20;
export const COMMAND_KIND_REMOVE_QUAD_FIELD: u16 = 21;
export const COMMAND_KIND_REMOVE_GROUP: u16 = 22;
export const COMMAND_KIND_UPSERT_GROUP_PRESENTATION: u16 = 23;
export const COMMAND_KIND_UPSERT_GROUP_CLIP_RECT: u16 = 24;
export const COMMAND_KIND_UPSERT_GROUP_LAYER: u16 = 25;
export const COMMAND_KIND_UPSERT_GROUP_TRANSFORM: u16 = 26;
export const COMMAND_KIND_UPSERT_GROUP_LOCAL_ORIGIN: u16 = 27;
export const COMMAND_KIND_UPSERT_GROUP_MATERIAL: u16 = 28;
export const COMMAND_KIND_UPSERT_GROUP_PASS: u16 = 29;

export const SPAWN_TEXT_COMMAND_BYTES: u32 = 56;
export const SPAWN_IMAGE_COMMAND_BYTES: u32 = 56;
export const SPAWN_IMAGE_AFFINE_COMMAND_BYTES: u32 = 88;
export const SPAWN_PULSE_COMMAND_BYTES: u32 = 52;
export const SPAWN_POLYLINE_HEADER_BYTES: u32 = 32;
export const POLYLINE_POINT_BYTES: u32 = 8;
export const SPAWN_PATH_STROKE_HEADER_BYTES: u32 = 32;
export const SPAWN_PATH_FILL_HEADER_BYTES: u32 = 32;
export const PATH_STROKE_NODE_BYTES: u32 = 28;
export const SPAWN_GLOW_BATCH_HEADER_BYTES: u32 = 16;
export const GLOW_BATCH_ITEM_BYTES: u32 = 36;
export const SPAWN_SPRITE_BATCH_HEADER_BYTES: u32 = 16;
export const SPRITE_BATCH_ITEM_BYTES: u32 = 44;
export const SPAWN_QUAD_BATCH_HEADER_BYTES: u32 = 16;
export const QUAD_BATCH_ITEM_BYTES: u32 = 64;
export const SPAWN_RIBBON_STRIP_HEADER_BYTES: u32 = 32;
export const RIBBON_STRIP_POINT_BYTES: u32 = 12;
export const UPSERT_RIBBON_TRAIL_HEADER_BYTES: u32 = 32;
export const REMOVE_RIBBON_TRAIL_COMMAND_BYTES: u32 = 8;
export const UPSERT_QUAD_FIELD_HEADER_BYTES: u32 = 16;
export const REMOVE_QUAD_FIELD_COMMAND_BYTES: u32 = 8;
export const REMOVE_GROUP_COMMAND_BYTES: u32 = 8;
export const UPSERT_GROUP_PRESENTATION_COMMAND_BYTES: u32 = 16;
export const UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES: u32 = 28;
export const GROUP_CLIP_MASK_TAIL_BYTES: u32 = 8;
export const UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES_WITH_MASK_TAIL: u32 =
  UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES + GROUP_CLIP_MASK_TAIL_BYTES;
export const UPSERT_GROUP_LAYER_COMMAND_BYTES: u32 = 16;
export const UPSERT_GROUP_TRANSFORM_COMMAND_BYTES: u32 = 16;
export const GROUP_TRANSFORM_TAIL_BYTES: u32 = 8;
export const GROUP_TRANSFORM_PIVOT_TAIL_BYTES: u32 = 8;
export const GROUP_TRANSFORM_SCALE2D_TAIL_BYTES: u32 = 8;
export const UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL: u32 =
  UPSERT_GROUP_TRANSFORM_COMMAND_BYTES + GROUP_TRANSFORM_TAIL_BYTES;
export const UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_AND_PIVOT: u32 =
  UPSERT_GROUP_TRANSFORM_COMMAND_BYTES + GROUP_TRANSFORM_TAIL_BYTES + GROUP_TRANSFORM_PIVOT_TAIL_BYTES;
export const UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_PIVOT_AND_SCALE2D: u32 =
  UPSERT_GROUP_TRANSFORM_COMMAND_BYTES +
  GROUP_TRANSFORM_TAIL_BYTES +
  GROUP_TRANSFORM_PIVOT_TAIL_BYTES +
  GROUP_TRANSFORM_SCALE2D_TAIL_BYTES;
export const UPSERT_GROUP_LOCAL_ORIGIN_COMMAND_BYTES: u32 = 16;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES: u32 = 20;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_TAIL: u32 = 28;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_AND_RESPONSE_TAILS: u32 = 36;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_RESPONSE_AND_FEEDBACK_TAILS: u32 = 44;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_ALL_TAILS: u32 = 52;
export const UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_FULL_TAILS: u32 = 60;
export const UPSERT_GROUP_PASS_COMMAND_BYTES: u32 = 20;
export const GROUP_PASS_MODE_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_STACK_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_PIPELINE_TAIL_BYTES: u32 = 12;
export const GROUP_PASS_BLEND_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_ROUTING_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_LANE_RESPONSE_TAIL_BYTES: u32 = 20;
export const GROUP_PASS_TEMPORAL_TAIL_BYTES: u32 = 12;
export const GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_TERTIARY_TAIL_BYTES: u32 = 16;
export const GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_TERTIARY_LANE_RESPONSE_TAIL_BYTES: u32 = 20;
export const GROUP_PASS_TERTIARY_TEMPORAL_TAIL_BYTES: u32 = 12;
export const GROUP_PASS_TERTIARY_TEMPORAL_MODE_TAIL_BYTES: u32 = 8;
export const GROUP_PASS_TERTIARY_STACK_TAIL_BYTES: u32 = 8;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_MODE_TAIL: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES + GROUP_PASS_MODE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_MODE_AND_STACK_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES + GROUP_PASS_MODE_TAIL_BYTES + GROUP_PASS_STACK_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_ALL_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_FULL_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_ROUTING_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_LANE_RESPONSE_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TEMPORAL_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TEMPORAL_MODE_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_ROUTING_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES +
  GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_LANE_RESPONSE_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES +
  GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES +
  GROUP_PASS_TERTIARY_LANE_RESPONSE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TEMPORAL_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES +
  GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES +
  GROUP_PASS_TERTIARY_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TEMPORAL_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TEMPORAL_MODE_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES +
  GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES +
  GROUP_PASS_TERTIARY_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TEMPORAL_MODE_TAIL_BYTES;
export const UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_STACK_TAILS: u32 =
  UPSERT_GROUP_PASS_COMMAND_BYTES +
  GROUP_PASS_MODE_TAIL_BYTES +
  GROUP_PASS_STACK_TAIL_BYTES +
  GROUP_PASS_PIPELINE_TAIL_BYTES +
  GROUP_PASS_BLEND_TAIL_BYTES +
  GROUP_PASS_ROUTING_TAIL_BYTES +
  GROUP_PASS_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TAIL_BYTES +
  GROUP_PASS_TERTIARY_ROUTING_TAIL_BYTES +
  GROUP_PASS_TERTIARY_LANE_RESPONSE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TEMPORAL_TAIL_BYTES +
  GROUP_PASS_TERTIARY_TEMPORAL_MODE_TAIL_BYTES +
  GROUP_PASS_TERTIARY_STACK_TAIL_BYTES;
export const UPSERT_GLOW_EMITTER_COMMAND_BYTES: u32 = 76;
export const REMOVE_GLOW_EMITTER_COMMAND_BYTES: u32 = 8;
export const UPSERT_SPRITE_EMITTER_COMMAND_BYTES: u32 = 88;
export const REMOVE_SPRITE_EMITTER_COMMAND_BYTES: u32 = 8;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES: u32 = 76;
export const REMOVE_PARTICLE_EMITTER_COMMAND_BYTES: u32 = 8;
export const PARTICLE_EMITTER_SPAWN_TAIL_BYTES: u32 = 16;
export const PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES: u32 = 16;
export const PARTICLE_EMITTER_LIFE_TAIL_BYTES: u32 = 24;
export const COMMAND_RENDER_SEMANTICS_TAIL_BYTES: u32 = 12;
export const COMMAND_CLIP_RECT_TAIL_BYTES: u32 = 16;
export const UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS: u32 =
  UPSERT_GLOW_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP: u32 =
  UPSERT_GLOW_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
export const UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS: u32 =
  UPSERT_SPRITE_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP: u32 =
  UPSERT_SPRITE_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_TAIL: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_TAIL_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_DYNAMICS_TAIL: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_DYNAMICS_TAIL_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_LIFE_TAIL: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_LIFE_TAIL_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_LIFE_TAILS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_LIFE_TAILS_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_DYNAMICS_TAILS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_DYNAMICS_TAILS_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS_AND_CLIP: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
export const UPSERT_RIBBON_TRAIL_HEADER_BYTES_WITH_SEMANTICS: u32 =
  UPSERT_RIBBON_TRAIL_HEADER_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
export const UPSERT_RIBBON_TRAIL_HEADER_BYTES_WITH_SEMANTICS_AND_CLIP: u32 =
  UPSERT_RIBBON_TRAIL_HEADER_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
export const UPSERT_QUAD_FIELD_HEADER_BYTES_WITH_SEMANTICS: u32 =
  UPSERT_QUAD_FIELD_HEADER_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;

export const PULSE_KIND_RIPPLE: u32 = 0;
export const PULSE_KIND_STAR: u32 = 1;
export const SPAWN_POLYLINE_FLAG_CLOSED: u32 = 0x0001;
export const SPAWN_GLOW_BATCH_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const SPAWN_SPRITE_BATCH_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const SPAWN_QUAD_BATCH_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const SPAWN_RIBBON_STRIP_FLAG_CLOSED: u32 = 0x0001;
export const UPSERT_RIBBON_TRAIL_FLAG_CLOSED: u32 = 0x0001;
export const UPSERT_RIBBON_TRAIL_FLAG_USE_GROUP_LOCAL_ORIGIN: u32 = 0x0002;
export const UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const UPSERT_QUAD_FIELD_FLAG_USE_GROUP_LOCAL_ORIGIN: u32 = 0x0002;
export const UPSERT_GROUP_PRESENTATION_FLAG_VISIBLE: u32 = 0x0001;
export const UPSERT_GROUP_CLIP_RECT_FLAG_ENABLED: u32 = 0x0001;
export const UPSERT_GROUP_LAYER_FLAG_BLEND_OVERRIDE_ENABLED: u32 = 0x0001;
export const UPSERT_GROUP_MATERIAL_FLAG_TINT_ENABLED: u32 = 0x0001;
export const GROUP_MATERIAL_STYLE_NONE: u32 = 0;
export const GROUP_MATERIAL_STYLE_SOFT_BLOOM_LIKE: u32 = 1;
export const GROUP_MATERIAL_STYLE_AFTERIMAGE_LIKE: u32 = 2;
export const GROUP_MATERIAL_FEEDBACK_MODE_DIRECTIONAL: u32 = 0;
export const GROUP_MATERIAL_FEEDBACK_MODE_TANGENTIAL: u32 = 1;
export const GROUP_MATERIAL_FEEDBACK_MODE_SWIRL: u32 = 2;
export const GROUP_PASS_KIND_NONE: u32 = 0;
export const GROUP_PASS_KIND_SOFT_BLOOM_LIKE: u32 = 1;
export const GROUP_PASS_KIND_AFTERIMAGE_LIKE: u32 = 2;
export const GROUP_PASS_KIND_ECHO_LIKE: u32 = 3;
export const GROUP_PASS_MODE_DIRECTIONAL: u32 = 0;
export const GROUP_PASS_MODE_TANGENTIAL: u32 = 1;
export const GROUP_PASS_MODE_SWIRL: u32 = 2;
export const GROUP_PASS_BLEND_MODE_MULTIPLY: u32 = 0;
export const GROUP_PASS_BLEND_MODE_LERP: u32 = 1;
export const GROUP_PASS_TEMPORAL_MODE_EXPONENTIAL: u32 = 0;
export const GROUP_PASS_TEMPORAL_MODE_LINEAR: u32 = 1;
export const GROUP_PASS_TEMPORAL_MODE_PULSE: u32 = 2;
export const GROUP_PASS_ROUTE_GLOW: u32 = 0x01;
export const GROUP_PASS_ROUTE_SPRITE: u32 = 0x02;
export const GROUP_PASS_ROUTE_PARTICLE: u32 = 0x04;
export const GROUP_PASS_ROUTE_RIBBON: u32 = 0x08;
export const GROUP_PASS_ROUTE_QUAD: u32 = 0x10;
export const GROUP_PASS_ROUTE_ALL: u32 = 0x1f;
export const GROUP_CLIP_MASK_SHAPE_RECT: u32 = 0;
export const GROUP_CLIP_MASK_SHAPE_ROUND_RECT: u32 = 1;
export const GROUP_CLIP_MASK_SHAPE_ELLIPSE: u32 = 2;
export const UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const UPSERT_GLOW_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN: u32 = 0x0002;
export const UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND: u32 = 0x0001;
export const UPSERT_SPRITE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN: u32 = 0x0002;
export const PARTICLE_EMITTER_STYLE_SOFT_GLOW: u32 = 0;
export const PARTICLE_EMITTER_STYLE_SOLID_DISC: u32 = 1;
export const PARTICLE_EMITTER_EMISSION_MODE_CONE: u32 = 0;
export const PARTICLE_EMITTER_EMISSION_MODE_RADIAL: u32 = 1;
export const PARTICLE_EMITTER_SPAWN_SHAPE_POINT: u32 = 0;
export const PARTICLE_EMITTER_SPAWN_SHAPE_DISC: u32 = 1;
export const PARTICLE_EMITTER_SPAWN_SHAPE_RING: u32 = 2;
export const UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND: u32 = 0x01;
export const UPSERT_PARTICLE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN: u32 = 0x02;
export const PATH_STROKE_NODE_MOVE_TO: u32 = 0;
export const PATH_STROKE_NODE_LINE_TO: u32 = 1;
export const PATH_STROKE_NODE_QUAD_TO: u32 = 2;
export const PATH_STROKE_NODE_CUBIC_TO: u32 = 3;
export const PATH_STROKE_NODE_CLOSE: u32 = 4;
export const PATH_STROKE_LINE_JOIN_MITER: u32 = 0;
export const PATH_STROKE_LINE_JOIN_ROUND: u32 = 1;
export const PATH_STROKE_LINE_JOIN_BEVEL: u32 = 2;
export const PATH_STROKE_LINE_CAP_BUTT: u32 = 0;
export const PATH_STROKE_LINE_CAP_ROUND: u32 = 1;
export const PATH_STROKE_LINE_CAP_SQUARE: u32 = 2;
export const PATH_FILL_RULE_NON_ZERO: u32 = 0;
export const PATH_FILL_RULE_EVEN_ODD: u32 = 1;
export const BLEND_MODE_NORMAL: u32 = 0;
export const BLEND_MODE_SCREEN: u32 = 1;
export const BLEND_MODE_ADD: u32 = 2;

export const BUTTON_LEFT: u8 = 1;
export const BUTTON_RIGHT: u8 = 2;
export const BUTTON_MIDDLE: u8 = 3;

export const EVENT_KIND_CLICK: u8 = 1;
export const EVENT_KIND_MOVE: u8 = 2;
export const EVENT_KIND_SCROLL: u8 = 3;
export const EVENT_KIND_HOLD_START: u8 = 4;
export const EVENT_KIND_HOLD_UPDATE: u8 = 5;
export const EVENT_KIND_HOLD_END: u8 = 6;
export const EVENT_KIND_HOVER_START: u8 = 7;
export const EVENT_KIND_HOVER_END: u8 = 8;
export const EVENT_KIND_INDICATOR_CLICK: u8 = 9;
export const EVENT_KIND_INDICATOR_SCROLL: u8 = 10;
export const EVENT_KIND_INDICATOR_KEY: u8 = 11;

export const EVENT_FLAG_SCROLL_HORIZONTAL: u8 = 0x01;
export const INDICATOR_MODIFIER_CTRL: u8 = 0x01;
export const INDICATOR_MODIFIER_SHIFT: u8 = 0x02;
export const INDICATOR_MODIFIER_ALT: u8 = 0x04;
export const INDICATOR_MODIFIER_META: u8 = 0x08;
export const INDICATOR_DETAIL_FLAG_KEY_SYSTEM: u8 = 0x01;

export function canHandleEvent(inputLen: u32, outputCap: u32, minOutputBytes: u32): bool {
  return inputLen >= EVENT_INPUT_BYTES && outputCap >= minOutputBytes;
}

export function hasIndicatorTail(inputLen: u32): bool {
  return inputLen >= (EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES);
}

export function hasIndicatorContextTail(inputLen: u32): bool {
  return inputLen >= (EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES + INDICATOR_EVENT_CONTEXT_TAIL_BYTES);
}

export function canHandleFrameInput(inputLen: u32, outputCap: u32, minOutputBytes: u32): bool {
  return inputLen >= FRAME_INPUT_BYTES && outputCap >= minOutputBytes;
}

export function canHandleClickEvent(
  inputPtr: usize,
  inputLen: u32,
  outputCap: u32,
  minOutputBytes: u32,
): bool {
  return canHandleEvent(inputLen, outputCap, minOutputBytes)
    && readEventKind(inputPtr) == EVENT_KIND_CLICK;
}

export function readEventX(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 0);
}

export function readEventY(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 4);
}

export function readEventDelta(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 8);
}

export function readEventHoldMs(inputPtr: usize): u32 {
  return load<u32>(inputPtr + 12);
}

export function readEventKind(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 16);
}

export function readEventButton(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 17);
}

export function readEventFlags(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 18);
}

export function readEventTickMs(inputPtr: usize): u64 {
  return load<u64>(inputPtr + 20);
}

export function readIndicatorSizePx(inputPtr: usize): u16 {
  return load<u16>(inputPtr + EVENT_INPUT_BYTES + 0);
}

export function readIndicatorDurationMs(inputPtr: usize): u16 {
  return load<u16>(inputPtr + EVENT_INPUT_BYTES + 2);
}

export function readIndicatorPrimaryCode(inputPtr: usize): u32 {
  return load<u32>(inputPtr + EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES + 0);
}

export function readIndicatorStreak(inputPtr: usize): u16 {
  return load<u16>(inputPtr + EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES + 4);
}

export function readIndicatorModifierMask(inputPtr: usize): u8 {
  return load<u8>(inputPtr + EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES + 6);
}

export function readIndicatorDetailFlags(inputPtr: usize): u8 {
  return load<u8>(inputPtr + EVENT_INPUT_BYTES + INDICATOR_EVENT_TAIL_BYTES + 7);
}

export function readFrameCursorX(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 0);
}

export function readFrameCursorY(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 4);
}

export function readFrameDeltaMs(inputPtr: usize): u32 {
  return load<u32>(inputPtr + 8);
}

export function readFramePointerValid(inputPtr: usize): bool {
  return load<u8>(inputPtr + 12) != 0;
}

export function readFrameHoldActive(inputPtr: usize): bool {
  return load<u8>(inputPtr + 13) != 0;
}

export function readFrameTickMs(inputPtr: usize): u64 {
  return load<u64>(inputPtr + 16);
}

export function writeSpawnText(
  outputPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  colorRgba: u32,
  delayMs: u32,
  lifeMs: u32,
  textId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_TEXT);
  store<u16>(outputPtr + 2, <u16>SPAWN_TEXT_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, ax);
  store<f32>(outputPtr + 24, ay);
  store<f32>(outputPtr + 28, scale);
  store<f32>(outputPtr + 32, rotation);
  store<f32>(outputPtr + 36, alpha);
  store<u32>(outputPtr + 40, colorRgba);
  store<u32>(outputPtr + 44, delayMs);
  store<u32>(outputPtr + 48, lifeMs);
  store<u32>(outputPtr + 52, textId);
}

export function writeSpawnImage(
  outputPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  tintRgba: u32,
  delayMs: u32,
  lifeMs: u32,
  imageId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_IMAGE);
  store<u16>(outputPtr + 2, <u16>SPAWN_IMAGE_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, ax);
  store<f32>(outputPtr + 24, ay);
  store<f32>(outputPtr + 28, scale);
  store<f32>(outputPtr + 32, rotation);
  store<f32>(outputPtr + 36, alpha);
  store<u32>(outputPtr + 40, tintRgba);
  store<u32>(outputPtr + 44, delayMs);
  store<u32>(outputPtr + 48, lifeMs);
  store<u32>(outputPtr + 52, imageId);
}

export function writeSpawnImageAffine(
  outputPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  tintRgba: u32,
  delayMs: u32,
  lifeMs: u32,
  imageId: u32,
  affineM11: f32,
  affineM12: f32,
  affineM21: f32,
  affineM22: f32,
  affineDx: f32,
  affineDy: f32,
  affineAnchorMode: u32,
  affineEnabled: bool,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_IMAGE_AFFINE);
  store<u16>(outputPtr + 2, <u16>SPAWN_IMAGE_AFFINE_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, ax);
  store<f32>(outputPtr + 24, ay);
  store<f32>(outputPtr + 28, scale);
  store<f32>(outputPtr + 32, rotation);
  store<f32>(outputPtr + 36, alpha);
  store<u32>(outputPtr + 40, tintRgba);
  store<u32>(outputPtr + 44, delayMs);
  store<u32>(outputPtr + 48, lifeMs);
  store<u32>(outputPtr + 52, imageId);
  store<f32>(outputPtr + 56, affineM11);
  store<f32>(outputPtr + 60, affineM12);
  store<f32>(outputPtr + 64, affineM21);
  store<f32>(outputPtr + 68, affineM22);
  store<f32>(outputPtr + 72, affineDx);
  store<f32>(outputPtr + 76, affineDy);
  store<u32>(outputPtr + 80, affineAnchorMode);
  store<u32>(outputPtr + 84, affineEnabled ? 1 : 0);
}

export function writeSpawnPulse(
  outputPtr: usize,
  x: f32,
  y: f32,
  startRadiusPx: f32,
  endRadiusPx: f32,
  strokeWidthPx: f32,
  alpha: f32,
  fillArgb: u32,
  strokeArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  pulseKind: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_PULSE);
  store<u16>(outputPtr + 2, <u16>SPAWN_PULSE_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, startRadiusPx);
  store<f32>(outputPtr + 16, endRadiusPx);
  store<f32>(outputPtr + 20, strokeWidthPx);
  store<f32>(outputPtr + 24, alpha);
  store<u32>(outputPtr + 28, fillArgb);
  store<u32>(outputPtr + 32, strokeArgb);
  store<u32>(outputPtr + 36, glowArgb);
  store<u32>(outputPtr + 40, delayMs);
  store<u32>(outputPtr + 44, lifeMs);
  store<u32>(outputPtr + 48, pulseKind);
}

export function spawnPolylineCommandBytes(pointCount: u32): u32 {
  return SPAWN_POLYLINE_HEADER_BYTES + pointCount * POLYLINE_POINT_BYTES;
}

export function spawnPathStrokeCommandBytes(nodeCount: u32): u32 {
  return SPAWN_PATH_STROKE_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES;
}

export function spawnPathFillCommandBytes(nodeCount: u32): u32 {
  return SPAWN_PATH_FILL_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES;
}

export function spawnRibbonStripCommandBytes(pointCount: u32): u32 {
  return SPAWN_RIBBON_STRIP_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES;
}

export function upsertRibbonTrailCommandBytes(pointCount: u32): u32 {
  return UPSERT_RIBBON_TRAIL_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES;
}

export function upsertQuadFieldCommandBytes(itemCount: u32): u32 {
  return UPSERT_QUAD_FIELD_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES;
}

export function spawnPathStrokeCommandBytesWithSemantics(nodeCount: u32): u32 {
  return spawnPathStrokeCommandBytes(nodeCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function spawnPathStrokeCommandBytesWithSemanticsAndClip(nodeCount: u32): u32 {
  return spawnPathStrokeCommandBytes(nodeCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function spawnPathFillCommandBytesWithSemantics(nodeCount: u32): u32 {
  return spawnPathFillCommandBytes(nodeCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function spawnPathFillCommandBytesWithSemanticsAndClip(nodeCount: u32): u32 {
  return spawnPathFillCommandBytes(nodeCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function spawnRibbonStripCommandBytesWithSemantics(pointCount: u32): u32 {
  return spawnRibbonStripCommandBytes(pointCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function spawnRibbonStripCommandBytesWithSemanticsAndClip(pointCount: u32): u32 {
  return spawnRibbonStripCommandBytes(pointCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function upsertRibbonTrailCommandBytesWithSemantics(pointCount: u32): u32 {
  return upsertRibbonTrailCommandBytes(pointCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function upsertRibbonTrailCommandBytesWithSemanticsAndClip(pointCount: u32): u32 {
  return upsertRibbonTrailCommandBytes(pointCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function upsertQuadFieldCommandBytesWithSemantics(itemCount: u32): u32 {
  return upsertQuadFieldCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function upsertQuadFieldCommandBytesWithSemanticsAndClip(itemCount: u32): u32 {
  return upsertQuadFieldCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function writeSpawnPolylineHeader(
  outputPtr: usize,
  pointCount: u32,
  lineWidthPx: f32,
  alpha: f32,
  strokeArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_POLYLINE);
  store<u16>(outputPtr + 2, <u16>spawnPolylineCommandBytes(pointCount));
  store<f32>(outputPtr + 4, lineWidthPx);
  store<f32>(outputPtr + 8, alpha);
  store<u32>(outputPtr + 12, strokeArgb);
  store<u32>(outputPtr + 16, glowArgb);
  store<u32>(outputPtr + 20, delayMs);
  store<u32>(outputPtr + 24, lifeMs);
  store<u16>(outputPtr + 28, <u16>pointCount);
  store<u16>(outputPtr + 30, <u16>flags);
}

export function writeSpawnPolylinePoint(
  outputPtr: usize,
  pointIndex: u32,
  x: f32,
  y: f32,
): void {
  const pointOffset = outputPtr + <usize>SPAWN_POLYLINE_HEADER_BYTES + <usize>(pointIndex * POLYLINE_POINT_BYTES);
  store<f32>(pointOffset + 0, x);
  store<f32>(pointOffset + 4, y);
}

export function writeSpawnPathStrokeHeader(
  outputPtr: usize,
  nodeCount: u32,
  lineWidthPx: f32,
  alpha: f32,
  strokeArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  lineJoin: u32,
  lineCap: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_PATH_STROKE);
  store<u16>(outputPtr + 2, <u16>spawnPathStrokeCommandBytes(nodeCount));
  store<f32>(outputPtr + 4, lineWidthPx);
  store<f32>(outputPtr + 8, alpha);
  store<u32>(outputPtr + 12, strokeArgb);
  store<u32>(outputPtr + 16, glowArgb);
  store<u32>(outputPtr + 20, delayMs);
  store<u32>(outputPtr + 24, lifeMs);
  store<u16>(outputPtr + 28, <u16>nodeCount);
  store<u8>(outputPtr + 30, <u8>lineJoin);
  store<u8>(outputPtr + 31, <u8>lineCap);
}

export function writeSpawnPathStrokeHeaderWithSemantics(
  outputPtr: usize,
  nodeCount: u32,
  lineWidthPx: f32,
  alpha: f32,
  strokeArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  lineJoin: u32,
  lineCap: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnPathStrokeHeader(
    outputPtr,
    nodeCount,
    lineWidthPx,
    alpha,
    strokeArgb,
    glowArgb,
    delayMs,
    lifeMs,
    lineJoin,
    lineCap,
  );
  store<u16>(outputPtr + 2, <u16>spawnPathStrokeCommandBytesWithSemantics(nodeCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_PATH_STROKE_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnPathStrokeHeaderWithSemanticsAndClip(
  outputPtr: usize,
  nodeCount: u32,
  lineWidthPx: f32,
  alpha: f32,
  strokeArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  lineJoin: u32,
  lineCap: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeSpawnPathStrokeHeaderWithSemantics(
    outputPtr,
    nodeCount,
    lineWidthPx,
    alpha,
    strokeArgb,
    glowArgb,
    delayMs,
    lifeMs,
    lineJoin,
    lineCap,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>spawnPathStrokeCommandBytesWithSemanticsAndClip(nodeCount));
  writeCommandClipRectTail(
    outputPtr,
    SPAWN_PATH_STROKE_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeSpawnPathFillHeader(
  outputPtr: usize,
  nodeCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  fillRule: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_PATH_FILL);
  store<u16>(outputPtr + 2, <u16>spawnPathFillCommandBytes(nodeCount));
  store<f32>(outputPtr + 4, alpha);
  store<f32>(outputPtr + 8, glowWidthPx);
  store<u32>(outputPtr + 12, fillArgb);
  store<u32>(outputPtr + 16, glowArgb);
  store<u32>(outputPtr + 20, delayMs);
  store<u32>(outputPtr + 24, lifeMs);
  store<u16>(outputPtr + 28, <u16>nodeCount);
  store<u8>(outputPtr + 30, <u8>fillRule);
  store<u8>(outputPtr + 31, 0);
}

export function writeSpawnPathFillHeaderWithSemantics(
  outputPtr: usize,
  nodeCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  fillRule: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnPathFillHeader(
    outputPtr,
    nodeCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    fillRule,
  );
  store<u16>(outputPtr + 2, <u16>spawnPathFillCommandBytesWithSemantics(nodeCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_PATH_FILL_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnPathFillHeaderWithSemanticsAndClip(
  outputPtr: usize,
  nodeCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  fillRule: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeSpawnPathFillHeaderWithSemantics(
    outputPtr,
    nodeCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    fillRule,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>spawnPathFillCommandBytesWithSemanticsAndClip(nodeCount));
  writeCommandClipRectTail(
    outputPtr,
    SPAWN_PATH_FILL_HEADER_BYTES + nodeCount * PATH_STROKE_NODE_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeSpawnRibbonStripHeader(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_RIBBON_STRIP);
  store<u16>(outputPtr + 2, <u16>spawnRibbonStripCommandBytes(pointCount));
  store<f32>(outputPtr + 4, alpha);
  store<f32>(outputPtr + 8, glowWidthPx);
  store<u32>(outputPtr + 12, fillArgb);
  store<u32>(outputPtr + 16, glowArgb);
  store<u32>(outputPtr + 20, delayMs);
  store<u32>(outputPtr + 24, lifeMs);
  store<u16>(outputPtr + 28, <u16>pointCount);
  store<u16>(outputPtr + 30, <u16>flags);
}

export function writeSpawnRibbonStripHeaderWithSemantics(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnRibbonStripHeader(
    outputPtr,
    pointCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>spawnRibbonStripCommandBytesWithSemantics(pointCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_RIBBON_STRIP_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnRibbonStripHeaderWithSemanticsAndClip(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeSpawnRibbonStripHeaderWithSemantics(
    outputPtr,
    pointCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>spawnRibbonStripCommandBytesWithSemanticsAndClip(pointCount));
  writeCommandClipRectTail(
    outputPtr,
    SPAWN_RIBBON_STRIP_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeSpawnRibbonStripPoint(
  outputPtr: usize,
  pointIndex: u32,
  x: f32,
  y: f32,
  widthPx: f32,
): void {
  const pointOffset = outputPtr + <usize>SPAWN_RIBBON_STRIP_HEADER_BYTES + <usize>(pointIndex * RIBBON_STRIP_POINT_BYTES);
  store<f32>(pointOffset + 0, x);
  store<f32>(pointOffset + 4, y);
  store<f32>(pointOffset + 8, widthPx);
}

export function writeUpsertRibbonTrailHeader(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  ttlMs: u32,
  trailId: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_RIBBON_TRAIL);
  store<u16>(outputPtr + 2, <u16>upsertRibbonTrailCommandBytes(pointCount));
  store<f32>(outputPtr + 4, alpha);
  store<f32>(outputPtr + 8, glowWidthPx);
  store<u32>(outputPtr + 12, fillArgb);
  store<u32>(outputPtr + 16, glowArgb);
  store<u32>(outputPtr + 20, ttlMs);
  store<u32>(outputPtr + 24, trailId);
  store<u16>(outputPtr + 28, <u16>pointCount);
  store<u16>(outputPtr + 30, <u16>flags);
}

export function writeUpsertRibbonTrailHeaderWithSemantics(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  ttlMs: u32,
  trailId: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertRibbonTrailHeader(
    outputPtr,
    pointCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    ttlMs,
    trailId,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>upsertRibbonTrailCommandBytesWithSemantics(pointCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_RIBBON_TRAIL_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertRibbonTrailHeaderWithSemanticsAndClip(
  outputPtr: usize,
  pointCount: u32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  ttlMs: u32,
  trailId: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeUpsertRibbonTrailHeaderWithSemantics(
    outputPtr,
    pointCount,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    ttlMs,
    trailId,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>upsertRibbonTrailCommandBytesWithSemanticsAndClip(pointCount));
  writeCommandClipRectTail(
    outputPtr,
    UPSERT_RIBBON_TRAIL_HEADER_BYTES + pointCount * RIBBON_STRIP_POINT_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeUpsertRibbonTrailPoint(
  outputPtr: usize,
  pointIndex: u32,
  x: f32,
  y: f32,
  widthPx: f32,
): void {
  const pointOffset = outputPtr + <usize>UPSERT_RIBBON_TRAIL_HEADER_BYTES + <usize>(pointIndex * RIBBON_STRIP_POINT_BYTES);
  store<f32>(pointOffset + 0, x);
  store<f32>(pointOffset + 4, y);
  store<f32>(pointOffset + 8, widthPx);
}

export function writeRemoveRibbonTrail(
  outputPtr: usize,
  trailId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_RIBBON_TRAIL);
  store<u16>(outputPtr + 2, <u16>REMOVE_RIBBON_TRAIL_COMMAND_BYTES);
  store<u32>(outputPtr + 4, trailId);
}

function writePathStrokeNodeBase(outputPtr: usize, nodeIndex: u32, opcode: u32): usize {
  const nodeOffset = outputPtr + <usize>SPAWN_PATH_STROKE_HEADER_BYTES + <usize>(nodeIndex * PATH_STROKE_NODE_BYTES);
  store<u8>(nodeOffset + 0, <u8>opcode);
  store<u8>(nodeOffset + 1, 0);
  store<u16>(nodeOffset + 2, 0);
  store<f32>(nodeOffset + 4, 0.0);
  store<f32>(nodeOffset + 8, 0.0);
  store<f32>(nodeOffset + 12, 0.0);
  store<f32>(nodeOffset + 16, 0.0);
  store<f32>(nodeOffset + 20, 0.0);
  store<f32>(nodeOffset + 24, 0.0);
  return nodeOffset;
}

export function writePathStrokeNodeMoveTo(
  outputPtr: usize,
  nodeIndex: u32,
  x: f32,
  y: f32,
): void {
  const nodeOffset = writePathStrokeNodeBase(outputPtr, nodeIndex, PATH_STROKE_NODE_MOVE_TO);
  store<f32>(nodeOffset + 4, x);
  store<f32>(nodeOffset + 8, y);
}

export function writePathStrokeNodeLineTo(
  outputPtr: usize,
  nodeIndex: u32,
  x: f32,
  y: f32,
): void {
  const nodeOffset = writePathStrokeNodeBase(outputPtr, nodeIndex, PATH_STROKE_NODE_LINE_TO);
  store<f32>(nodeOffset + 4, x);
  store<f32>(nodeOffset + 8, y);
}

export function writePathStrokeNodeQuadTo(
  outputPtr: usize,
  nodeIndex: u32,
  controlX: f32,
  controlY: f32,
  x: f32,
  y: f32,
): void {
  const nodeOffset = writePathStrokeNodeBase(outputPtr, nodeIndex, PATH_STROKE_NODE_QUAD_TO);
  store<f32>(nodeOffset + 4, controlX);
  store<f32>(nodeOffset + 8, controlY);
  store<f32>(nodeOffset + 12, x);
  store<f32>(nodeOffset + 16, y);
}

export function writePathStrokeNodeCubicTo(
  outputPtr: usize,
  nodeIndex: u32,
  control1X: f32,
  control1Y: f32,
  control2X: f32,
  control2Y: f32,
  x: f32,
  y: f32,
): void {
  const nodeOffset = writePathStrokeNodeBase(outputPtr, nodeIndex, PATH_STROKE_NODE_CUBIC_TO);
  store<f32>(nodeOffset + 4, control1X);
  store<f32>(nodeOffset + 8, control1Y);
  store<f32>(nodeOffset + 12, control2X);
  store<f32>(nodeOffset + 16, control2Y);
  store<f32>(nodeOffset + 20, x);
  store<f32>(nodeOffset + 24, y);
}

export function writePathStrokeNodeClose(
  outputPtr: usize,
  nodeIndex: u32,
): void {
  writePathStrokeNodeBase(outputPtr, nodeIndex, PATH_STROKE_NODE_CLOSE);
}

export function spawnGlowBatchCommandBytes(itemCount: u32): u32 {
  return SPAWN_GLOW_BATCH_HEADER_BYTES + itemCount * GLOW_BATCH_ITEM_BYTES;
}

export function spawnGlowBatchCommandBytesWithSemantics(itemCount: u32): u32 {
  return spawnGlowBatchCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function writeCommandRenderSemanticsTail(
  outputPtr: usize,
  offset: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  const tailPtr = outputPtr + <usize>offset;
  store<u8>(tailPtr + 0, <u8>blendMode);
  store<u8>(tailPtr + 1, 0);
  store<u16>(tailPtr + 2, 0);
  store<i32>(tailPtr + 4, sortKey);
  store<u32>(tailPtr + 8, groupId);
}

export function writeCommandClipRectTail(
  outputPtr: usize,
  offset: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  const tailPtr = outputPtr + <usize>offset;
  store<f32>(tailPtr + 0, clipLeftPx);
  store<f32>(tailPtr + 4, clipTopPx);
  store<f32>(tailPtr + 8, clipWidthPx);
  store<f32>(tailPtr + 12, clipHeightPx);
}

export function writeSpawnGlowBatchHeader(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_GLOW_BATCH);
  store<u16>(outputPtr + 2, <u16>spawnGlowBatchCommandBytes(itemCount));
  store<u32>(outputPtr + 4, delayMs);
  store<u32>(outputPtr + 8, lifeMs);
  store<u16>(outputPtr + 12, <u16>itemCount);
  store<u16>(outputPtr + 14, <u16>flags);
}

export function writeSpawnGlowBatchHeaderWithSemantics(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnGlowBatchHeader(outputPtr, itemCount, delayMs, lifeMs, flags);
  store<u16>(outputPtr + 2, <u16>spawnGlowBatchCommandBytesWithSemantics(itemCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_GLOW_BATCH_HEADER_BYTES + itemCount * GLOW_BATCH_ITEM_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnGlowBatchItem(
  outputPtr: usize,
  itemIndex: u32,
  x: f32,
  y: f32,
  radiusPx: f32,
  alpha: f32,
  colorArgb: u32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
): void {
  const itemOffset = outputPtr + <usize>SPAWN_GLOW_BATCH_HEADER_BYTES + <usize>(itemIndex * GLOW_BATCH_ITEM_BYTES);
  store<f32>(itemOffset + 0, x);
  store<f32>(itemOffset + 4, y);
  store<f32>(itemOffset + 8, radiusPx);
  store<f32>(itemOffset + 12, alpha);
  store<u32>(itemOffset + 16, colorArgb);
  store<f32>(itemOffset + 20, vx);
  store<f32>(itemOffset + 24, vy);
  store<f32>(itemOffset + 28, ax);
  store<f32>(itemOffset + 32, ay);
}

export function spawnSpriteBatchCommandBytes(itemCount: u32): u32 {
  return SPAWN_SPRITE_BATCH_HEADER_BYTES + itemCount * SPRITE_BATCH_ITEM_BYTES;
}

export function spawnSpriteBatchCommandBytesWithSemantics(itemCount: u32): u32 {
  return spawnSpriteBatchCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function spawnSpriteBatchCommandBytesWithSemanticsAndClip(itemCount: u32): u32 {
  return spawnSpriteBatchCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function writeSpawnSpriteBatchHeader(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_SPRITE_BATCH);
  store<u16>(outputPtr + 2, <u16>spawnSpriteBatchCommandBytes(itemCount));
  store<u32>(outputPtr + 4, delayMs);
  store<u32>(outputPtr + 8, lifeMs);
  store<u16>(outputPtr + 12, <u16>itemCount);
  store<u16>(outputPtr + 14, <u16>flags);
}

export function writeSpawnSpriteBatchHeaderWithSemantics(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnSpriteBatchHeader(outputPtr, itemCount, delayMs, lifeMs, flags);
  store<u16>(outputPtr + 2, <u16>spawnSpriteBatchCommandBytesWithSemantics(itemCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_SPRITE_BATCH_HEADER_BYTES + itemCount * SPRITE_BATCH_ITEM_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnSpriteBatchHeaderWithSemanticsAndClip(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeSpawnSpriteBatchHeaderWithSemantics(
    outputPtr,
    itemCount,
    delayMs,
    lifeMs,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>spawnSpriteBatchCommandBytesWithSemanticsAndClip(itemCount));
  writeCommandClipRectTail(
    outputPtr,
    SPAWN_SPRITE_BATCH_HEADER_BYTES + itemCount * SPRITE_BATCH_ITEM_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeSpawnSpriteBatchItem(
  outputPtr: usize,
  itemIndex: u32,
  x: f32,
  y: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  tintRgba: u32,
  imageId: u32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
): void {
  const itemOffset = outputPtr + <usize>SPAWN_SPRITE_BATCH_HEADER_BYTES + <usize>(itemIndex * SPRITE_BATCH_ITEM_BYTES);
  store<f32>(itemOffset + 0, x);
  store<f32>(itemOffset + 4, y);
  store<f32>(itemOffset + 8, scale);
  store<f32>(itemOffset + 12, rotation);
  store<f32>(itemOffset + 16, alpha);
  store<u32>(itemOffset + 20, tintRgba);
  store<u32>(itemOffset + 24, imageId);
  store<f32>(itemOffset + 28, vx);
  store<f32>(itemOffset + 32, vy);
  store<f32>(itemOffset + 36, ax);
  store<f32>(itemOffset + 40, ay);
}

export function spawnQuadBatchCommandBytes(itemCount: u32): u32 {
  return SPAWN_QUAD_BATCH_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES;
}

export function spawnQuadBatchCommandBytesWithSemantics(itemCount: u32): u32 {
  return spawnQuadBatchCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES;
}

export function spawnQuadBatchCommandBytesWithSemanticsAndClip(itemCount: u32): u32 {
  return spawnQuadBatchCommandBytes(itemCount) + COMMAND_RENDER_SEMANTICS_TAIL_BYTES + COMMAND_CLIP_RECT_TAIL_BYTES;
}

export function writeSpawnQuadBatchHeader(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_QUAD_BATCH);
  store<u16>(outputPtr + 2, <u16>spawnQuadBatchCommandBytes(itemCount));
  store<u32>(outputPtr + 4, delayMs);
  store<u32>(outputPtr + 8, lifeMs);
  store<u16>(outputPtr + 12, <u16>itemCount);
  store<u16>(outputPtr + 14, <u16>flags);
}

export function writeSpawnQuadBatchHeaderWithSemantics(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnQuadBatchHeader(outputPtr, itemCount, delayMs, lifeMs, flags);
  store<u16>(outputPtr + 2, <u16>spawnQuadBatchCommandBytesWithSemantics(itemCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    SPAWN_QUAD_BATCH_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeSpawnQuadBatchHeaderWithSemanticsAndClip(
  outputPtr: usize,
  itemCount: u32,
  delayMs: u32,
  lifeMs: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeSpawnQuadBatchHeaderWithSemantics(
    outputPtr,
    itemCount,
    delayMs,
    lifeMs,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>spawnQuadBatchCommandBytesWithSemanticsAndClip(itemCount));
  writeCommandClipRectTail(
    outputPtr,
    SPAWN_QUAD_BATCH_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeSpawnQuadBatchItem(
  outputPtr: usize,
  itemIndex: u32,
  x: f32,
  y: f32,
  widthPx: f32,
  heightPx: f32,
  alpha: f32,
  rotation: f32,
  tintRgba: u32,
  imageId: u32,
  srcU0: f32,
  srcV0: f32,
  srcU1: f32,
  srcV1: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
): void {
  const itemOffset = outputPtr + <usize>SPAWN_QUAD_BATCH_HEADER_BYTES + <usize>(itemIndex * QUAD_BATCH_ITEM_BYTES);
  store<f32>(itemOffset + 0, x);
  store<f32>(itemOffset + 4, y);
  store<f32>(itemOffset + 8, widthPx);
  store<f32>(itemOffset + 12, heightPx);
  store<f32>(itemOffset + 16, alpha);
  store<f32>(itemOffset + 20, rotation);
  store<u32>(itemOffset + 24, tintRgba);
  store<u32>(itemOffset + 28, imageId);
  store<f32>(itemOffset + 32, srcU0);
  store<f32>(itemOffset + 36, srcV0);
  store<f32>(itemOffset + 40, srcU1);
  store<f32>(itemOffset + 44, srcV1);
  store<f32>(itemOffset + 48, vx);
  store<f32>(itemOffset + 52, vy);
  store<f32>(itemOffset + 56, ax);
  store<f32>(itemOffset + 60, ay);
}

export function writeUpsertQuadFieldHeader(
  outputPtr: usize,
  itemCount: u32,
  ttlMs: u32,
  fieldId: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_QUAD_FIELD);
  store<u16>(outputPtr + 2, <u16>upsertQuadFieldCommandBytes(itemCount));
  store<u32>(outputPtr + 4, ttlMs);
  store<u32>(outputPtr + 8, fieldId);
  store<u16>(outputPtr + 12, <u16>itemCount);
  store<u16>(outputPtr + 14, <u16>flags);
}

export function writeUpsertQuadFieldHeaderWithSemantics(
  outputPtr: usize,
  itemCount: u32,
  ttlMs: u32,
  fieldId: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertQuadFieldHeader(outputPtr, itemCount, ttlMs, fieldId, flags);
  store<u16>(outputPtr + 2, <u16>upsertQuadFieldCommandBytesWithSemantics(itemCount));
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_QUAD_FIELD_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertQuadFieldHeaderWithSemanticsAndClip(
  outputPtr: usize,
  itemCount: u32,
  ttlMs: u32,
  fieldId: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeUpsertQuadFieldHeaderWithSemantics(
    outputPtr,
    itemCount,
    ttlMs,
    fieldId,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>upsertQuadFieldCommandBytesWithSemanticsAndClip(itemCount));
  writeCommandClipRectTail(
    outputPtr,
    UPSERT_QUAD_FIELD_HEADER_BYTES + itemCount * QUAD_BATCH_ITEM_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeUpsertQuadFieldItem(
  outputPtr: usize,
  itemIndex: u32,
  x: f32,
  y: f32,
  widthPx: f32,
  heightPx: f32,
  alpha: f32,
  rotation: f32,
  tintRgba: u32,
  imageId: u32,
  srcU0: f32,
  srcV0: f32,
  srcU1: f32,
  srcV1: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
): void {
  const itemOffset = outputPtr + <usize>UPSERT_QUAD_FIELD_HEADER_BYTES + <usize>(itemIndex * QUAD_BATCH_ITEM_BYTES);
  store<f32>(itemOffset + 0, x);
  store<f32>(itemOffset + 4, y);
  store<f32>(itemOffset + 8, widthPx);
  store<f32>(itemOffset + 12, heightPx);
  store<f32>(itemOffset + 16, alpha);
  store<f32>(itemOffset + 20, rotation);
  store<u32>(itemOffset + 24, tintRgba);
  store<u32>(itemOffset + 28, imageId);
  store<f32>(itemOffset + 32, srcU0);
  store<f32>(itemOffset + 36, srcV0);
  store<f32>(itemOffset + 40, srcU1);
  store<f32>(itemOffset + 44, srcV1);
  store<f32>(itemOffset + 48, vx);
  store<f32>(itemOffset + 52, vy);
  store<f32>(itemOffset + 56, ax);
  store<f32>(itemOffset + 60, ay);
}

export function writeRemoveQuadField(
  outputPtr: usize,
  fieldId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_QUAD_FIELD);
  store<u16>(outputPtr + 2, <u16>REMOVE_QUAD_FIELD_COMMAND_BYTES);
  store<u32>(outputPtr + 4, fieldId);
}

export function writeRemoveGroup(
  outputPtr: usize,
  groupId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_GROUP);
  store<u16>(outputPtr + 2, <u16>REMOVE_GROUP_COMMAND_BYTES);
  store<u32>(outputPtr + 4, groupId);
}

export function writeUpsertGroupPresentation(
  outputPtr: usize,
  alphaMultiplier: f32,
  groupId: u32,
  visible: bool,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_PRESENTATION);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PRESENTATION_COMMAND_BYTES);
  store<f32>(outputPtr + 4, alphaMultiplier);
  store<u32>(outputPtr + 8, groupId);
  store<u16>(
    outputPtr + 12,
    <u16>(visible ? UPSERT_GROUP_PRESENTATION_FLAG_VISIBLE : 0),
  );
  store<u16>(outputPtr + 14, 0);
}

export function writeUpsertGroupClipRect(
  outputPtr: usize,
  leftPx: f32,
  topPx: f32,
  widthPx: f32,
  heightPx: f32,
  groupId: u32,
  enabled: bool = true,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_CLIP_RECT);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES);
  store<f32>(outputPtr + 4, leftPx);
  store<f32>(outputPtr + 8, topPx);
  store<f32>(outputPtr + 12, widthPx);
  store<f32>(outputPtr + 16, heightPx);
  store<u32>(outputPtr + 20, groupId);
  store<u16>(
    outputPtr + 24,
    <u16>(enabled ? UPSERT_GROUP_CLIP_RECT_FLAG_ENABLED : 0),
  );
  store<u16>(outputPtr + 26, 0);
}

export function writeUpsertGroupClipRectWithMaskTail(
  outputPtr: usize,
  leftPx: f32,
  topPx: f32,
  widthPx: f32,
  heightPx: f32,
  groupId: u32,
  maskShapeKind: u32,
  cornerRadiusPx: f32,
  enabled: bool = true,
): void {
  writeUpsertGroupClipRect(
    outputPtr,
    leftPx,
    topPx,
    widthPx,
    heightPx,
    groupId,
    enabled,
  );
  store<u8>(outputPtr + <usize>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES + 0, <u8>maskShapeKind);
  store<u8>(outputPtr + <usize>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES + 1, 0);
  store<u16>(outputPtr + <usize>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES + 2, 0);
  store<f32>(outputPtr + <usize>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES + 4, cornerRadiusPx);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES_WITH_MASK_TAIL);
}

export function writeUpsertGroupLayer(
  outputPtr: usize,
  groupId: u32,
  hasBlendOverride: bool,
  blendMode: u32,
  sortBias: i32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_LAYER);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_LAYER_COMMAND_BYTES);
  store<u32>(outputPtr + 4, groupId);
  store<u16>(
    outputPtr + 8,
    <u16>(hasBlendOverride ? UPSERT_GROUP_LAYER_FLAG_BLEND_OVERRIDE_ENABLED : 0),
  );
  store<u8>(outputPtr + 10, <u8>blendMode);
  store<u8>(outputPtr + 11, 0);
  store<i32>(outputPtr + 12, sortBias);
}

export function writeUpsertGroupTransform(
  outputPtr: usize,
  groupId: u32,
  offsetXPx: f32,
  offsetYPx: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_TRANSFORM);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_TRANSFORM_COMMAND_BYTES);
  store<f32>(outputPtr + 4, offsetXPx);
  store<f32>(outputPtr + 8, offsetYPx);
  store<u32>(outputPtr + 12, groupId);
}

export function writeUpsertGroupTransformWithTail(
  outputPtr: usize,
  groupId: u32,
  offsetXPx: f32,
  offsetYPx: f32,
  rotationRad: f32,
  uniformScale: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_TRANSFORM);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL);
  store<f32>(outputPtr + 4, offsetXPx);
  store<f32>(outputPtr + 8, offsetYPx);
  store<u32>(outputPtr + 12, groupId);
  store<f32>(outputPtr + 16, rotationRad);
  store<f32>(outputPtr + 20, uniformScale);
}

export function writeUpsertGroupTransformWithTailAndPivot(
  outputPtr: usize,
  groupId: u32,
  offsetXPx: f32,
  offsetYPx: f32,
  rotationRad: f32,
  uniformScale: f32,
  pivotXPx: f32,
  pivotYPx: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_TRANSFORM);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_AND_PIVOT);
  store<f32>(outputPtr + 4, offsetXPx);
  store<f32>(outputPtr + 8, offsetYPx);
  store<u32>(outputPtr + 12, groupId);
  store<f32>(outputPtr + 16, rotationRad);
  store<f32>(outputPtr + 20, uniformScale);
  store<f32>(outputPtr + 24, pivotXPx);
  store<f32>(outputPtr + 28, pivotYPx);
}

export function writeUpsertGroupTransformWithTailPivotAndScale2D(
  outputPtr: usize,
  groupId: u32,
  offsetXPx: f32,
  offsetYPx: f32,
  rotationRad: f32,
  uniformScale: f32,
  pivotXPx: f32,
  pivotYPx: f32,
  scaleX: f32,
  scaleY: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_TRANSFORM);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_PIVOT_AND_SCALE2D);
  store<f32>(outputPtr + 4, offsetXPx);
  store<f32>(outputPtr + 8, offsetYPx);
  store<u32>(outputPtr + 12, groupId);
  store<f32>(outputPtr + 16, rotationRad);
  store<f32>(outputPtr + 20, uniformScale);
  store<f32>(outputPtr + 24, pivotXPx);
  store<f32>(outputPtr + 28, pivotYPx);
  store<f32>(outputPtr + 32, scaleX);
  store<f32>(outputPtr + 36, scaleY);
}

export function writeUpsertGroupLocalOrigin(
  outputPtr: usize,
  groupId: u32,
  originXPx: f32,
  originYPx: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_LOCAL_ORIGIN);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_LOCAL_ORIGIN_COMMAND_BYTES);
  store<f32>(outputPtr + 4, originXPx);
  store<f32>(outputPtr + 8, originYPx);
  store<u32>(outputPtr + 12, groupId);
}

export function writeUpsertGroupMaterial(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool = true,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_MATERIAL);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES);
  store<u32>(outputPtr + 4, groupId);
  store<u32>(outputPtr + 8, tintArgb);
  store<f32>(outputPtr + 12, intensityMultiplier);
  store<u16>(
    outputPtr + 16,
    <u16>(hasTintOverride ? UPSERT_GROUP_MATERIAL_FLAG_TINT_ENABLED : 0),
  );
  store<u16>(outputPtr + 18, 0);
}

export function writeUpsertGroupMaterialWithStyle(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool,
  styleKind: u32,
  styleAmount: f32,
): void {
  writeUpsertGroupMaterial(outputPtr, groupId, tintArgb, intensityMultiplier, hasTintOverride);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_TAIL);
  store<u8>(outputPtr + 20, <u8>styleKind);
  store<u8>(outputPtr + 21, 0);
  store<u16>(outputPtr + 22, 0);
  store<f32>(outputPtr + 24, styleAmount);
}

export function writeUpsertGroupMaterialWithStyleAndResponse(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool,
  styleKind: u32,
  styleAmount: f32,
  diffusionAmount: f32,
  persistenceAmount: f32,
): void {
  writeUpsertGroupMaterialWithStyle(
    outputPtr,
    groupId,
    tintArgb,
    intensityMultiplier,
    hasTintOverride,
    styleKind,
    styleAmount,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_AND_RESPONSE_TAILS);
  store<f32>(outputPtr + 28, diffusionAmount);
  store<f32>(outputPtr + 32, persistenceAmount);
}

export function writeUpsertGroupMaterialWithStyleResponseAndFeedback(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool,
  styleKind: u32,
  styleAmount: f32,
  diffusionAmount: f32,
  persistenceAmount: f32,
  echoAmount: f32,
  echoDriftPx: f32,
): void {
  writeUpsertGroupMaterialWithStyleAndResponse(
    outputPtr,
    groupId,
    tintArgb,
    intensityMultiplier,
    hasTintOverride,
    styleKind,
    styleAmount,
    diffusionAmount,
    persistenceAmount,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_STYLE_RESPONSE_AND_FEEDBACK_TAILS);
  store<f32>(outputPtr + 36, echoAmount);
  store<f32>(outputPtr + 40, echoDriftPx);
}

export function writeUpsertGroupMaterialWithAllTails(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool,
  styleKind: u32,
  styleAmount: f32,
  diffusionAmount: f32,
  persistenceAmount: f32,
  echoAmount: f32,
  echoDriftPx: f32,
  feedbackMode: u32,
  feedbackPhaseRad: f32,
): void {
  writeUpsertGroupMaterialWithStyleResponseAndFeedback(
    outputPtr,
    groupId,
    tintArgb,
    intensityMultiplier,
    hasTintOverride,
    styleKind,
    styleAmount,
    diffusionAmount,
    persistenceAmount,
    echoAmount,
    echoDriftPx,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_ALL_TAILS);
  store<u8>(outputPtr + 44, <u8>feedbackMode);
  store<u8>(outputPtr + 45, 0);
  store<u16>(outputPtr + 46, 0);
  store<f32>(outputPtr + 48, feedbackPhaseRad);
}

export function writeUpsertGroupMaterialWithFullTails(
  outputPtr: usize,
  groupId: u32,
  tintArgb: u32,
  intensityMultiplier: f32,
  hasTintOverride: bool,
  styleKind: u32,
  styleAmount: f32,
  diffusionAmount: f32,
  persistenceAmount: f32,
  echoAmount: f32,
  echoDriftPx: f32,
  feedbackMode: u32,
  feedbackPhaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
): void {
  writeUpsertGroupMaterialWithAllTails(
    outputPtr,
    groupId,
    tintArgb,
    intensityMultiplier,
    hasTintOverride,
    styleKind,
    styleAmount,
    diffusionAmount,
    persistenceAmount,
    echoAmount,
    echoDriftPx,
    feedbackMode,
    feedbackPhaseRad,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_MATERIAL_COMMAND_BYTES_WITH_FULL_TAILS);
  store<u8>(outputPtr + 52, <u8>feedbackLayerCount);
  store<u8>(outputPtr + 53, 0);
  store<u16>(outputPtr + 54, 0);
  store<f32>(outputPtr + 56, feedbackLayerFalloff);
}

export function writeUpsertGroupPass(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GROUP_PASS);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES);
  store<u32>(outputPtr + 4, groupId);
  store<u8>(outputPtr + 8, <u8>passKind);
  store<u8>(outputPtr + 9, 0);
  store<u16>(outputPtr + 10, 0);
  store<f32>(outputPtr + 12, passAmount);
  store<f32>(outputPtr + 16, responseAmount);
}

export function writeUpsertGroupPassWithMode(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
): void {
  writeUpsertGroupPass(outputPtr, groupId, passKind, passAmount, responseAmount);
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_MODE_TAIL);
  store<u8>(outputPtr + 20, <u8>passMode);
  store<u8>(outputPtr + 21, 0);
  store<u16>(outputPtr + 22, 0);
  store<f32>(outputPtr + 24, phaseRad);
}

export function writeUpsertGroupPassWithModeAndStack(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
): void {
  writeUpsertGroupPassWithMode(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_MODE_AND_STACK_TAILS);
  store<u8>(outputPtr + 28, <u8>feedbackLayerCount);
  store<u8>(outputPtr + 29, 0);
  store<u16>(outputPtr + 30, 0);
  store<f32>(outputPtr + 32, feedbackLayerFalloff);
}

export function writeUpsertGroupPassWithAllTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
): void {
  writeUpsertGroupPassWithModeAndStack(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_ALL_TAILS);
  store<u8>(outputPtr + 36, <u8>secondaryPassKind);
  store<u8>(outputPtr + 37, 0);
  store<u16>(outputPtr + 38, 0);
  store<f32>(outputPtr + 40, secondaryPassAmount);
  store<f32>(outputPtr + 44, secondaryResponseAmount);
}

export function writeUpsertGroupPassWithFullTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
): void {
  writeUpsertGroupPassWithAllTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_FULL_TAILS);
  store<u8>(outputPtr + 48, <u8>secondaryBlendMode);
  store<u8>(outputPtr + 49, 0);
  store<u16>(outputPtr + 50, 0);
  store<f32>(outputPtr + 52, secondaryBlendWeight);
}

export function writeUpsertGroupPassWithRoutingTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
): void {
  writeUpsertGroupPassWithFullTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_ROUTING_TAILS);
  store<u8>(outputPtr + 56, <u8>secondaryRouteMask);
  store<u8>(outputPtr + 57, 0);
  store<u16>(outputPtr + 58, 0);
  store<u32>(outputPtr + 60, 0);
}

export function writeUpsertGroupPassWithLaneResponseTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
): void {
  writeUpsertGroupPassWithRoutingTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_LANE_RESPONSE_TAILS);
  store<f32>(outputPtr + 64, glowResponse);
  store<f32>(outputPtr + 68, spriteResponse);
  store<f32>(outputPtr + 72, particleResponse);
  store<f32>(outputPtr + 76, ribbonResponse);
  store<f32>(outputPtr + 80, quadResponse);
}

export function writeUpsertGroupPassWithTemporalTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
): void {
  writeUpsertGroupPassWithLaneResponseTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TEMPORAL_TAILS);
  store<f32>(outputPtr + 84, phaseRateRadPerSec);
  store<f32>(outputPtr + 88, secondaryDecayPerSec);
  store<f32>(outputPtr + 92, secondaryDecayFloor);
}

export function writeUpsertGroupPassWithTemporalModeTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
): void {
  writeUpsertGroupPassWithTemporalTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TEMPORAL_MODE_TAILS);
  store<u8>(outputPtr + 96, <u8>temporalMode);
  store<u8>(outputPtr + 97, 0);
  store<u16>(outputPtr + 98, 0);
  store<f32>(outputPtr + 100, temporalStrength);
}

export function writeUpsertGroupPassWithTertiaryTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
): void {
  writeUpsertGroupPassWithTemporalModeTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TAILS);
  store<u8>(outputPtr + 104, <u8>tertiaryPassKind);
  store<u8>(outputPtr + 105, <u8>tertiaryBlendMode);
  store<u16>(outputPtr + 106, 0);
  store<f32>(outputPtr + 108, tertiaryPassAmount);
  store<f32>(outputPtr + 112, tertiaryResponseAmount);
  store<f32>(outputPtr + 116, tertiaryBlendWeight);
}

export function writeUpsertGroupPassWithTertiaryRoutingTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
  tertiaryRouteMask: u32,
): void {
  writeUpsertGroupPassWithTertiaryTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
    tertiaryPassKind,
    tertiaryPassAmount,
    tertiaryResponseAmount,
    tertiaryBlendMode,
    tertiaryBlendWeight,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_ROUTING_TAILS);
  store<u8>(outputPtr + 120, <u8>tertiaryRouteMask);
  store<u8>(outputPtr + 121, 0);
  store<u16>(outputPtr + 122, 0);
  store<u32>(outputPtr + 124, 0);
}

export function writeUpsertGroupPassWithTertiaryLaneResponseTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
  tertiaryRouteMask: u32,
  tertiaryGlowResponse: f32,
  tertiarySpriteResponse: f32,
  tertiaryParticleResponse: f32,
  tertiaryRibbonResponse: f32,
  tertiaryQuadResponse: f32,
): void {
  writeUpsertGroupPassWithTertiaryRoutingTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
    tertiaryPassKind,
    tertiaryPassAmount,
    tertiaryResponseAmount,
    tertiaryBlendMode,
    tertiaryBlendWeight,
    tertiaryRouteMask,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_LANE_RESPONSE_TAILS);
  store<f32>(outputPtr + 128, tertiaryGlowResponse);
  store<f32>(outputPtr + 132, tertiarySpriteResponse);
  store<f32>(outputPtr + 136, tertiaryParticleResponse);
  store<f32>(outputPtr + 140, tertiaryRibbonResponse);
  store<f32>(outputPtr + 144, tertiaryQuadResponse);
}

export function writeUpsertGroupPassWithTertiaryTemporalTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
  tertiaryRouteMask: u32,
  tertiaryGlowResponse: f32,
  tertiarySpriteResponse: f32,
  tertiaryParticleResponse: f32,
  tertiaryRibbonResponse: f32,
  tertiaryQuadResponse: f32,
  tertiaryPhaseRateRadPerSec: f32,
  tertiaryDecayPerSec: f32,
  tertiaryDecayFloor: f32,
): void {
  writeUpsertGroupPassWithTertiaryLaneResponseTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
    tertiaryPassKind,
    tertiaryPassAmount,
    tertiaryResponseAmount,
    tertiaryBlendMode,
    tertiaryBlendWeight,
    tertiaryRouteMask,
    tertiaryGlowResponse,
    tertiarySpriteResponse,
    tertiaryParticleResponse,
    tertiaryRibbonResponse,
    tertiaryQuadResponse,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TEMPORAL_TAILS);
  store<f32>(outputPtr + 148, tertiaryPhaseRateRadPerSec);
  store<f32>(outputPtr + 152, tertiaryDecayPerSec);
  store<f32>(outputPtr + 156, tertiaryDecayFloor);
}

export function writeUpsertGroupPassWithTertiaryTemporalModeTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
  tertiaryRouteMask: u32,
  tertiaryGlowResponse: f32,
  tertiarySpriteResponse: f32,
  tertiaryParticleResponse: f32,
  tertiaryRibbonResponse: f32,
  tertiaryQuadResponse: f32,
  tertiaryPhaseRateRadPerSec: f32,
  tertiaryDecayPerSec: f32,
  tertiaryDecayFloor: f32,
  tertiaryTemporalMode: u32,
  tertiaryTemporalStrength: f32,
): void {
  writeUpsertGroupPassWithTertiaryTemporalTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
    tertiaryPassKind,
    tertiaryPassAmount,
    tertiaryResponseAmount,
    tertiaryBlendMode,
    tertiaryBlendWeight,
    tertiaryRouteMask,
    tertiaryGlowResponse,
    tertiarySpriteResponse,
    tertiaryParticleResponse,
    tertiaryRibbonResponse,
    tertiaryQuadResponse,
    tertiaryPhaseRateRadPerSec,
    tertiaryDecayPerSec,
    tertiaryDecayFloor,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_TEMPORAL_MODE_TAILS);
  store<u8>(outputPtr + 160, <u8>tertiaryTemporalMode);
  store<u8>(outputPtr + 161, 0);
  store<u16>(outputPtr + 162, 0);
  store<f32>(outputPtr + 164, tertiaryTemporalStrength);
}

export function writeUpsertGroupPassWithTertiaryStackTails(
  outputPtr: usize,
  groupId: u32,
  passKind: u32,
  passAmount: f32,
  responseAmount: f32,
  passMode: u32,
  phaseRad: f32,
  feedbackLayerCount: u32,
  feedbackLayerFalloff: f32,
  secondaryPassKind: u32,
  secondaryPassAmount: f32,
  secondaryResponseAmount: f32,
  secondaryBlendMode: u32,
  secondaryBlendWeight: f32,
  secondaryRouteMask: u32,
  glowResponse: f32,
  spriteResponse: f32,
  particleResponse: f32,
  ribbonResponse: f32,
  quadResponse: f32,
  phaseRateRadPerSec: f32,
  secondaryDecayPerSec: f32,
  secondaryDecayFloor: f32,
  temporalMode: u32,
  temporalStrength: f32,
  tertiaryPassKind: u32,
  tertiaryPassAmount: f32,
  tertiaryResponseAmount: f32,
  tertiaryBlendMode: u32,
  tertiaryBlendWeight: f32,
  tertiaryRouteMask: u32,
  tertiaryGlowResponse: f32,
  tertiarySpriteResponse: f32,
  tertiaryParticleResponse: f32,
  tertiaryRibbonResponse: f32,
  tertiaryQuadResponse: f32,
  tertiaryPhaseRateRadPerSec: f32,
  tertiaryDecayPerSec: f32,
  tertiaryDecayFloor: f32,
  tertiaryTemporalMode: u32,
  tertiaryTemporalStrength: f32,
  tertiaryFeedbackLayerCount: u32,
  tertiaryFeedbackLayerFalloff: f32,
): void {
  writeUpsertGroupPassWithTertiaryTemporalModeTails(
    outputPtr,
    groupId,
    passKind,
    passAmount,
    responseAmount,
    passMode,
    phaseRad,
    feedbackLayerCount,
    feedbackLayerFalloff,
    secondaryPassKind,
    secondaryPassAmount,
    secondaryResponseAmount,
    secondaryBlendMode,
    secondaryBlendWeight,
    secondaryRouteMask,
    glowResponse,
    spriteResponse,
    particleResponse,
    ribbonResponse,
    quadResponse,
    phaseRateRadPerSec,
    secondaryDecayPerSec,
    secondaryDecayFloor,
    temporalMode,
    temporalStrength,
    tertiaryPassKind,
    tertiaryPassAmount,
    tertiaryResponseAmount,
    tertiaryBlendMode,
    tertiaryBlendWeight,
    tertiaryRouteMask,
    tertiaryGlowResponse,
    tertiarySpriteResponse,
    tertiaryParticleResponse,
    tertiaryRibbonResponse,
    tertiaryQuadResponse,
    tertiaryPhaseRateRadPerSec,
    tertiaryDecayPerSec,
    tertiaryDecayFloor,
    tertiaryTemporalMode,
    tertiaryTemporalStrength,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_STACK_TAILS);
  store<u8>(outputPtr + 168, <u8>tertiaryFeedbackLayerCount);
  store<u8>(outputPtr + 169, 0);
  store<u16>(outputPtr + 170, 0);
  store<f32>(outputPtr + 172, tertiaryFeedbackLayerFalloff);
}

export function writeUpsertGlowEmitter(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_GLOW_EMITTER);
  store<u16>(outputPtr + 2, <u16>UPSERT_GLOW_EMITTER_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, emissionRatePerSec);
  store<f32>(outputPtr + 16, directionRad);
  store<f32>(outputPtr + 20, spreadRad);
  store<f32>(outputPtr + 24, speedMin);
  store<f32>(outputPtr + 28, speedMax);
  store<f32>(outputPtr + 32, radiusMinPx);
  store<f32>(outputPtr + 36, radiusMaxPx);
  store<f32>(outputPtr + 40, alphaMin);
  store<f32>(outputPtr + 44, alphaMax);
  store<u32>(outputPtr + 48, colorArgb);
  store<f32>(outputPtr + 52, accelerationX);
  store<f32>(outputPtr + 56, accelerationY);
  store<u32>(outputPtr + 60, emitterId);
  store<u32>(outputPtr + 64, emitterTtlMs);
  store<u32>(outputPtr + 68, particleLifeMs);
  store<u16>(outputPtr + 72, <u16>maxParticles);
  store<u16>(outputPtr + 74, <u16>flags);
}

export function writeUpsertGlowEmitterWithSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertGlowEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_GLOW_EMITTER_COMMAND_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertGlowEmitterWithSemanticsAndClip(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeUpsertGlowEmitterWithSemantics(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP);
  writeCommandClipRectTail(
    outputPtr,
    UPSERT_GLOW_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeRemoveGlowEmitter(
  outputPtr: usize,
  emitterId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_GLOW_EMITTER);
  store<u16>(outputPtr + 2, <u16>REMOVE_GLOW_EMITTER_COMMAND_BYTES);
  store<u32>(outputPtr + 4, emitterId);
}

export function writeUpsertSpriteEmitter(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  sizeMinPx: f32,
  sizeMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  tintArgb: u32,
  rotationMinRad: f32,
  rotationMaxRad: f32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  imageId: u32,
  maxParticles: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_SPRITE_EMITTER);
  store<u16>(outputPtr + 2, <u16>UPSERT_SPRITE_EMITTER_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, emissionRatePerSec);
  store<f32>(outputPtr + 16, directionRad);
  store<f32>(outputPtr + 20, spreadRad);
  store<f32>(outputPtr + 24, speedMin);
  store<f32>(outputPtr + 28, speedMax);
  store<f32>(outputPtr + 32, sizeMinPx);
  store<f32>(outputPtr + 36, sizeMaxPx);
  store<f32>(outputPtr + 40, alphaMin);
  store<f32>(outputPtr + 44, alphaMax);
  store<u32>(outputPtr + 48, tintArgb);
  store<f32>(outputPtr + 52, rotationMinRad);
  store<f32>(outputPtr + 56, rotationMaxRad);
  store<f32>(outputPtr + 60, accelerationX);
  store<f32>(outputPtr + 64, accelerationY);
  store<u32>(outputPtr + 68, emitterId);
  store<u32>(outputPtr + 72, emitterTtlMs);
  store<u32>(outputPtr + 76, particleLifeMs);
  store<u32>(outputPtr + 80, imageId);
  store<u16>(outputPtr + 84, <u16>maxParticles);
  store<u16>(outputPtr + 86, <u16>flags);
}

export function writeUpsertSpriteEmitterWithSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  sizeMinPx: f32,
  sizeMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  tintArgb: u32,
  rotationMinRad: f32,
  rotationMaxRad: f32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  imageId: u32,
  maxParticles: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertSpriteEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    sizeMinPx,
    sizeMaxPx,
    alphaMin,
    alphaMax,
    tintArgb,
    rotationMinRad,
    rotationMaxRad,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    imageId,
    maxParticles,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_SPRITE_EMITTER_COMMAND_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertSpriteEmitterWithSemanticsAndClip(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  sizeMinPx: f32,
  sizeMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  tintArgb: u32,
  rotationMinRad: f32,
  rotationMaxRad: f32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  imageId: u32,
  maxParticles: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeUpsertSpriteEmitterWithSemantics(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    sizeMinPx,
    sizeMaxPx,
    alphaMin,
    alphaMax,
    tintArgb,
    rotationMinRad,
    rotationMaxRad,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    imageId,
    maxParticles,
    flags,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP);
  writeCommandClipRectTail(
    outputPtr,
    UPSERT_SPRITE_EMITTER_COMMAND_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeRemoveSpriteEmitter(
  outputPtr: usize,
  emitterId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_SPRITE_EMITTER);
  store<u16>(outputPtr + 2, <u16>REMOVE_SPRITE_EMITTER_COMMAND_BYTES);
  store<u32>(outputPtr + 4, emitterId);
}

export function writeUpsertParticleEmitter(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_UPSERT_PARTICLE_EMITTER);
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, emissionRatePerSec);
  store<f32>(outputPtr + 16, directionRad);
  store<f32>(outputPtr + 20, spreadRad);
  store<f32>(outputPtr + 24, speedMin);
  store<f32>(outputPtr + 28, speedMax);
  store<f32>(outputPtr + 32, radiusMinPx);
  store<f32>(outputPtr + 36, radiusMaxPx);
  store<f32>(outputPtr + 40, alphaMin);
  store<f32>(outputPtr + 44, alphaMax);
  store<u32>(outputPtr + 48, colorArgb);
  store<f32>(outputPtr + 52, accelerationX);
  store<f32>(outputPtr + 56, accelerationY);
  store<u32>(outputPtr + 60, emitterId);
  store<u32>(outputPtr + 64, emitterTtlMs);
  store<u32>(outputPtr + 68, particleLifeMs);
  store<u16>(outputPtr + 72, <u16>maxParticles);
  store<u8>(outputPtr + 74, <u8>particleStyle);
  store<u8>(outputPtr + 75, <u8>flags);
}

export function writeParticleEmitterLifeTail(
  outputPtr: usize,
  tailOffset: u32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
): void {
  const offset = outputPtr + <usize>tailOffset;
  store<f32>(offset + 0, sizeStartScale);
  store<f32>(offset + 4, sizeEndScale);
  store<f32>(offset + 8, alphaStartScale);
  store<f32>(offset + 12, alphaEndScale);
  store<u32>(offset + 16, colorStartArgb);
  store<u32>(offset + 20, colorEndArgb);
}

export function writeParticleEmitterSpawnTail(
  outputPtr: usize,
  tailOffset: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
): void {
  const offset = outputPtr + <usize>tailOffset;
  store<u8>(offset + 0, <u8>emissionMode);
  store<u8>(offset + 1, <u8>spawnShape);
  store<u16>(offset + 2, 0);
  store<f32>(offset + 4, spawnRadiusX);
  store<f32>(offset + 8, spawnRadiusY);
  store<f32>(offset + 12, spawnInnerRatio);
}

export function writeParticleEmitterDynamicsTail(
  outputPtr: usize,
  tailOffset: u32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
): void {
  const offset = outputPtr + <usize>tailOffset;
  store<f32>(offset + 0, dragPerSecond);
  store<f32>(offset + 4, turbulenceAccel);
  store<f32>(offset + 8, turbulenceFrequencyHz);
  store<f32>(offset + 12, turbulencePhaseJitter);
}

export function writeUpsertParticleEmitterWithSpawnTail(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
): void {
  writeUpsertParticleEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_TAIL);
  writeParticleEmitterSpawnTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
  );
}

export function writeUpsertParticleEmitterWithDynamicsTail(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
): void {
  writeUpsertParticleEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_DYNAMICS_TAIL);
  writeParticleEmitterDynamicsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
  );
}

export function writeUpsertParticleEmitterWithLifeTail(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
): void {
  writeUpsertParticleEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_LIFE_TAIL);
  writeParticleEmitterLifeTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
}

export function writeUpsertParticleEmitterWithSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitter(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithSpawnTailAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithSpawnTail(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_TAIL_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithDynamicsTailAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithDynamicsTail(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_DYNAMICS_TAIL_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithLifeTailAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithLifeTail(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_LIFE_TAIL_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithSpawnAndLifeTails(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
): void {
  writeUpsertParticleEmitterWithSpawnTail(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_LIFE_TAILS);
  writeParticleEmitterLifeTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
}

export function writeUpsertParticleEmitterWithSpawnAndDynamicsTails(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
): void {
  writeUpsertParticleEmitterWithSpawnTail(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_DYNAMICS_TAILS);
  writeParticleEmitterDynamicsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
  );
}

export function writeUpsertParticleEmitterWithSpawnAndDynamicsTailsAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithSpawnAndDynamicsTails(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_DYNAMICS_TAILS_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithSpawnAndLifeTailsAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithSpawnAndLifeTails(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_AND_LIFE_TAILS_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTails(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
): void {
  writeUpsertParticleEmitterWithSpawnAndDynamicsTails(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS);
  writeParticleEmitterLifeTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
}

export function writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTails(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS);
  writeCommandRenderSemanticsTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES,
    blendMode,
    sortKey,
    groupId,
  );
}

export function writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(
  outputPtr: usize,
  x: f32,
  y: f32,
  emissionRatePerSec: f32,
  directionRad: f32,
  spreadRad: f32,
  speedMin: f32,
  speedMax: f32,
  radiusMinPx: f32,
  radiusMaxPx: f32,
  alphaMin: f32,
  alphaMax: f32,
  colorArgb: u32,
  accelerationX: f32,
  accelerationY: f32,
  emitterId: u32,
  emitterTtlMs: u32,
  particleLifeMs: u32,
  maxParticles: u32,
  particleStyle: u32,
  flags: u32,
  emissionMode: u32,
  spawnShape: u32,
  spawnRadiusX: f32,
  spawnRadiusY: f32,
  spawnInnerRatio: f32,
  dragPerSecond: f32,
  turbulenceAccel: f32,
  turbulenceFrequencyHz: f32,
  turbulencePhaseJitter: f32,
  sizeStartScale: f32,
  sizeEndScale: f32,
  alphaStartScale: f32,
  alphaEndScale: f32,
  colorStartArgb: u32,
  colorEndArgb: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
  clipLeftPx: f32,
  clipTopPx: f32,
  clipWidthPx: f32,
  clipHeightPx: f32,
): void {
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(
    outputPtr,
    x,
    y,
    emissionRatePerSec,
    directionRad,
    spreadRad,
    speedMin,
    speedMax,
    radiusMinPx,
    radiusMaxPx,
    alphaMin,
    alphaMax,
    colorArgb,
    accelerationX,
    accelerationY,
    emitterId,
    emitterTtlMs,
    particleLifeMs,
    maxParticles,
    particleStyle,
    flags,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
    blendMode,
    sortKey,
    groupId,
  );
  store<u16>(outputPtr + 2, <u16>UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS_AND_CLIP);
  writeCommandClipRectTail(
    outputPtr,
    UPSERT_PARTICLE_EMITTER_COMMAND_BYTES + PARTICLE_EMITTER_SPAWN_TAIL_BYTES + PARTICLE_EMITTER_DYNAMICS_TAIL_BYTES + PARTICLE_EMITTER_LIFE_TAIL_BYTES + COMMAND_RENDER_SEMANTICS_TAIL_BYTES,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );
}

export function writeRemoveParticleEmitter(
  outputPtr: usize,
  emitterId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_REMOVE_PARTICLE_EMITTER);
  store<u16>(outputPtr + 2, <u16>REMOVE_PARTICLE_EMITTER_COMMAND_BYTES);
  store<u32>(outputPtr + 4, emitterId);
}
