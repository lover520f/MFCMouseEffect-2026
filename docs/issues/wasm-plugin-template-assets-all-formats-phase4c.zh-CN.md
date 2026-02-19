# WASM 模板 Phase4c：真实资源包与全格式覆盖

## 背景

此前模板虽然支持 `image_assets` 字段，但样例构建产物不是“开箱即用”：
- 构建时不会把图片资源复制到 `dist/samples/...`；
- 用户导入样例清单后仍可能缺图；
- 各格式覆盖关系不够明确。

## 本次改动

## 1. 模板真实资源包

新增 `examples/wasm-plugin-template/assets`，包含真实下载/生成图片资源。
覆盖宿主支持的全部格式：
- `.png`
- `.jpg`
- `.jpeg`
- `.bmp`
- `.gif`
- `.tif`
- `.tiff`

## 2. 样例预设改为全格式映射

更新：
- `examples/wasm-plugin-template/scripts/sample-presets.mjs`

关键映射：
- `image-burst` 使用 `emoji-1.jpeg`
- `image-lift` 使用 `lift-a.tif` + `lift-b.tiff`
- `button-adaptive` 使用 `btn-middle.tiff`

## 3. 构建脚本自动复制图片资源

更新：
- `examples/wasm-plugin-template/scripts/build-lib.mjs`
- `examples/wasm-plugin-template/scripts/build.mjs`
- `examples/wasm-plugin-template/scripts/build-sample.mjs`
- `examples/wasm-plugin-template/scripts/build-all-samples.mjs`

新增公共能力：
- `copyRelativeFiles(rootDir, outputDir, relativeFiles)`

行为：
- 当清单或样例预设声明 `image_assets` 时，构建结果会自动复制对应 `assets/*` 文件到产物目录。

## 4. 用户文档同步（中英文）

更新：
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`
- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`

## 验证

在模板目录执行：

```bash
pnpm run build:samples
```

期望：
- 全部样例预设构建通过；
- 样例 `plugin.json` 的 `image_assets` 正确；
- 产物目录包含对应 `assets/*` 文件。

## 结果

模板样例现在可直接导入运行，无需手工补图。
