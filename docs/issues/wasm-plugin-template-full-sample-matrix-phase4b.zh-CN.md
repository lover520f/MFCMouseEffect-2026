# WASM 模板 Phase4b：全量样例矩阵与中英文用户文档

## 背景

用户需要快速看懂 `wasm-example` 目录结构并直接套用完整样例。
此前模板文档存在几个问题：
- 模板根目录只有英文 README；
- 样例 key 列表与实际预设矩阵不完全一致；
- 部分预设 key 缺少源码实现。

## 本次改动

## 1. 样例矩阵补齐

新增缺失样例实现：
- `examples/wasm-plugin-template/assembly/samples/mixed-emoji-celebrate.ts`
- `examples/wasm-plugin-template/assembly/samples/button-adaptive.ts`

当前可构建样例矩阵完整覆盖：
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

## 2. 样例清单 image_assets 输出

更新脚本：
- `examples/wasm-plugin-template/scripts/build-sample.mjs`
- `examples/wasm-plugin-template/scripts/build-all-samples.mjs`
- `examples/wasm-plugin-template/scripts/sample-presets.mjs`

行为：
- 预设里声明了 `imageAssets` 时，生成的 `plugin.json` 会自动写入 `image_assets`。

## 3. 面向用户文档（中英文）

扩展并重写：
- `examples/wasm-plugin-template/README.md`

新增中文文档：
- `examples/wasm-plugin-template/README.zh-CN.md`

两份文档都覆盖：
- 目录结构说明；
- 全量样例矩阵；
- 构建命令；
- 清单字段规则；
- 运行时放置路径。

## 4. 架构/快速开始文档同步

更新：
- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/custom-effects-wasm-route.md`
- `docs/architecture/custom-effects-wasm-route.zh-CN.md`

## 验证

在 `examples/wasm-plugin-template` 目录执行：

```bash
pnpm run build:samples
```

期望：
- 所有样例 key 均可编译通过；
- 每个样例目录都有 `effect.wasm` 与 `plugin.json`；
- 带图片资源预设的 `plugin.json` 包含 `image_assets`。

## 结果

模板已具备面向用户交付条件：
- 样例覆盖完整；
- 中英文文档齐全；
- 快速开始与架构文档一致。
