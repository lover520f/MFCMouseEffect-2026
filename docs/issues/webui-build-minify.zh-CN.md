# WebUI 构建体积优化（开启 minify）

## 背景
Web 设置页使用多入口 Svelte 构建，产物为 `webui/*.svelte.js`。  
此前构建配置显式关闭压缩，导致最终随安装包分发的 `webui` 体积偏大。

## 目标
1. 在不改变现有运行时加载方式（多入口 + IIFE）的前提下，先做低风险体积压缩。
2. 保持现有 `pnpm run build` 与复制流程不变。

## 修改
- 文件：`MFCMouseEffect/WebUIWorkspace/vite.config.js`
- 调整：
1. `build.minify` 从 `false` 改为 `'esbuild'`。
2. 增加 `esbuild.legalComments = 'none'`，减少产物注释体积。

## 结果（实测）
基于 `pnpm run build` 后的 `x64/Release/webui`：

1. 总体积：
   - 优化前：`1,374,806` bytes
   - 优化后：`483,927` bytes
   - 下降：`890,879` bytes（约 `64.8%`）
2. `*.svelte.js` 合计：
   - 优化前：`1,301,171` bytes
   - 优化后：`410,292` bytes
   - 下降：`890,879` bytes（约 `68.5%`）

## 兼容性与风险
1. 仅变更构建输出压缩策略，不改业务逻辑与接口。
2. 发布可用性无影响；调试可读性会下降（压缩后代码不便直接阅读）。
3. 若后续需要进一步瘦身，可在此基础上评估“多入口共享运行时代码”方案。

