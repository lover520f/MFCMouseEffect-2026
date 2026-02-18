# WASM 模板 Phase4：样例预设与构建工具收口

## 背景

Phase4 目标是让用户可直接基于官方模板本地编译插件。
原模板只有单一入口和单一构建命令，不利于：
- 快速对比不同效果风格；
- 按样例隔离排错；
- 对外发布可复用参考实现。

## 本次改动

## 1. 模板代码分层

- 新增公共模块：
  - `examples/wasm-plugin-template/assembly/common/abi.ts`
  - `examples/wasm-plugin-template/assembly/common/random.ts`
- 新增样例预设：
  - `examples/wasm-plugin-template/assembly/samples/text-rise.ts`
  - `examples/wasm-plugin-template/assembly/samples/text-burst.ts`
  - `examples/wasm-plugin-template/assembly/samples/image-pulse.ts`
  - `examples/wasm-plugin-template/assembly/samples/mixed-text-image.ts`
- `assembly/index.ts` 仍作为默认入口，改为转发 `text-rise` 导出。

## 2. 构建脚本重构

- 新增通用构建库：
  - `examples/wasm-plugin-template/scripts/build-lib.mjs`
  - `examples/wasm-plugin-template/scripts/sample-presets.mjs`
- 新增样例构建入口：
  - `examples/wasm-plugin-template/scripts/build-sample.mjs`
  - `examples/wasm-plugin-template/scripts/build-all-samples.mjs`
- 更新：
  - `examples/wasm-plugin-template/scripts/build.mjs`
  - `examples/wasm-plugin-template/package.json`

新增命令：
- `npm run build`
- `npm run build:sample -- --sample <key>`
- `npm run build:samples`

## 3. pnpm 兼容处理

`build-lib.mjs` 兼容两类 AssemblyScript 编译器路径：
- `node_modules/assemblyscript/bin/asc.js`（pnpm 常见）
- `node_modules/assemblyscript/bin/asc`（旧路径）

避免误报“找不到编译器”。

## 4. 清单与文档同步

- 默认清单更新：
  - `examples/wasm-plugin-template/plugin.json`
  - 默认 `id` 改为 `demo.click.text-rise.v1`
- 文档同步：
  - `examples/wasm-plugin-template/README.md`
  - `docs/architecture/wasm-plugin-template-quickstart.md`
  - `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
  - `docs/architecture/wasm-plugin-troubleshooting.md`
  - `docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`

## 验证

在 `examples/wasm-plugin-template` 目录执行：

```bash
node ./scripts/build.mjs
node ./scripts/build-sample.mjs --sample mixed-text-image
node ./scripts/build-all-samples.mjs
```

期望结果：
- 默认产物位于 `dist/`
- 各样例产物位于 `dist/samples/<sample_key>/`
- 无 AssemblyScript 类型转换报错。

## 补充说明

验证中曾出现 `text-burst` 的 `f64 -> f32` 报错。
已通过 `assembly/samples/text-burst.ts` 中显式 `f32` 类型与强转修复。
