# MFCMouseEffect WASM 插件模板

这是 `MFCMouseEffect` 官方的 WASM v1（AssemblyScript）插件模板。

语言： [English](README.md) | [中文](README.zh-CN.md)

## 模板包含内容

- 稳定 ABI 辅助：点击事件输入解析 + 命令缓冲输出写入。
- 可复用随机/颜色工具。
- 覆盖文本、图片、混合、按键自适应的完整样例矩阵。
- 默认构建 / 单样例构建 / 全样例构建脚本。

## 目录结构

```text
examples/wasm-plugin-template/
  assembly/
    common/
      abi.ts                  # ABI 常量与读写辅助
      random.ts               # 确定性随机辅助
    samples/
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
    index.ts                  # 默认入口（当前导出 text-rise）
  scripts/
    build-lib.mjs             # 构建公共能力
    sample-presets.mjs        # 样例矩阵元数据
    build.mjs                 # 构建默认入口
    build-sample.mjs          # 按 key 构建单样例
    build-all-samples.mjs     # 构建全部样例
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
```

样例产物目录：
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/effect.wat`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*`（自动复制 `image_assets` 引用的图片）

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

`sample-presets.mjs` 是样例元数据唯一来源，包含：
- 样例 key
- 源码入口
- 插件 id/name/version
- 可选 `image_assets`

## 内置资源包（覆盖全部支持格式）

模板 `assets/` 已覆盖并接入宿主支持的全部图片格式：
- `.png`：`smile.png`、`confetti.png`、`crown.png`、`emoji-2.png`、`mix-a.png`、`btn-left.png`
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
  "id": "demo.click.text-rise.v1",
  "name": "Demo Click Text Rise",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm"
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

## 运行时放置路径

将 `effect.wasm` + `plugin.json` 复制到插件目录：
- Debug：`<exe_dir>/plugins/wasm/<plugin_id>/`
- Release：`%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` 必须与 `plugin.json.id` 一致。

样例包部署时请复制整个样例目录（`effect.wasm` + `plugin.json` + `assets/`）。

## ABI 提醒

当前 ABI：
- `api_version = 1`

必须导出：
- `mfx_plugin_get_api_version`
- `mfx_plugin_on_click`
- `mfx_plugin_reset`

二进制布局权威定义：
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## 相关文档

- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/wasm-plugin-compatibility.zh-CN.md`
- `docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`
