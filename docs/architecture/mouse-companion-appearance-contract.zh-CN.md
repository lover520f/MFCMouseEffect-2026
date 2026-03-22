# Mouse Companion Appearance Contract (P2)

## 目标
- 支持“修改模型外观”能力：材质纹理覆写 + 配件节点显隐。
- 保持运行时高性能：模型加载后建立索引，运行期按结构化参数应用。

## 配置文件
- 默认路径：
  - `Assets/Pet3D/source/pet-appearance.json`
  - `MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json`

## JSON 契约
顶层支持两种模式：

1. 直接默认配置：
```json
{
  "default": {
    "skinVariantId": "default",
    "enabledAccessoryIds": [],
    "textureOverridePaths": []
  }
}
```

2. 预设模式：
```json
{
  "activePreset": "casual",
  "presets": [
    { "id": "casual", "skinVariantId": "casual", "enabledAccessoryIds": [], "textureOverridePaths": [] }
  ]
}
```

当前 Windows persona 验收默认推荐直接切 `activePreset`，例如：
- `default`
- `cream-moon`
- `night-leaf`
- `strawberry-ribbon-bow`

为避免 Win receive-only 同步工作区在验收时手改主配置，仓库也提供三份只读组合验收文件：
- `MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-cream-moon.json`
- `MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-night-leaf.json`
- `MFCMouseEffect/Assets/Pet3D/source/pet-appearance.combo-strawberry-ribbon-bow.json`

建议用途：
- mac 主开发源继续维护主 `pet-appearance.json`
- Win 手测/脚本验收优先切 `appearance_profile_path` 到这些只读文件，而不是在 receive-only 仓库里修改 `activePreset`

加载优先级：
- 若存在有效 `activePreset + presets`，优先加载活动预设
- 否则回退到 `default`

当前 Windows 运行时诊断会同步暴露：
- `appearance_requested_preset_id`
- `appearance_resolved_preset_id`
- `appearance_skin_variant_id`
- `appearance_accessory_ids`
- `appearance_accessory_family`
- `appearance_combo_preset`
- `appearance_plugin_id`
- `appearance_plugin_kind`
- `appearance_plugin_source`

用途：
- 便于在 Win 机器上区分“配置里请求了哪个 preset”与“最终实际落到了哪个 preset”
- 便于 real preview / render-proof / `/api/state` 直接确认 `skin + accessory + persona` 是否和当前 `pet-appearance.json` 一致
- 便于确认当前 `appearance/persona` 语义究竟来自哪个 renderer plugin provider；Phase1.5 当前默认应为 builtin native provider，后续接 wasm 时应沿用这组字段而不是重新发明另一套诊断键

字段说明：
- `skinVariantId`：皮肤变体标识（当前保留并透传到渲染桥，便于后续规则扩展）。
- `enabledAccessoryIds`：配件节点 ID 列表（按节点名匹配）。
- `textureOverridePaths`：纹理覆写列表，格式：
  - `MaterialName=/abs/path/to/texture.png`
  - 或 `MaterialName:/abs/path/to/texture.png`

## 运行时流程
1. `LoadPetAppearanceProfileFromJsonFile(...)` 读取并解析配置。
2. `PetCompanionRuntime::ApplyAppearance(...)` 保存当前外观覆写并下发到模型运行时。
3. macOS 桥接：
   - `mfx_macos_mouse_companion_apply_appearance_v1(...)`
   - 配件：按 `enabledAccessoryIds` 控制节点显隐。
   - 材质：按 `textureOverridePaths` 应用纹理；未覆写项回退到模型原始 diffuse。

## 关键性能策略
- 材质索引与骨架索引均在模型加载后构建缓存，不在每帧全量查找。
- 外观应用为“事件触发”（配置变更/模型重载时），不是每帧刷新。

## 回归检查
- 编译：
  - `cmake --build build-macos --target mfx_entry_runtime_common -j 6`
- 行为：
  1. 缺失 `pet-appearance.json` 时安全降级（不影响动作与渲染）。
  2. 配置存在时可成功应用（无崩溃、无线程告警）。
  3. 材质覆写路径无效时回退默认材质，不导致异常。
  4. 切换 `activePreset` 后，Windows `renderer_runtime_*` / `real_renderer_preview.*` 诊断中的 preset / skin / accessory / persona 字段与实际配置一致。
