# WebUIWorkspace：npm 切换到 pnpm

## 目的
- 统一前端包管理器为 `pnpm`，减少安装体积与依赖解析开销。
- 让构建命令与锁文件策略一致，避免 `npm` / `pnpm` 混用导致的锁文件漂移。

## 调整内容
1. `MFCMouseEffect/WebUIWorkspace/package.json`
   - 新增 `packageManager: "pnpm@10.28.2"`。
   - `build` 脚本链由 `npm run ...` 改为 `pnpm run ...`。
2. 锁文件切换
   - 删除 `MFCMouseEffect/WebUIWorkspace/package-lock.json`。
   - 新增 `MFCMouseEffect/WebUIWorkspace/pnpm-lock.yaml`。
3. 文档命令同步
   - Web 设置迁移与相关 issue 文档中的 `npm install` / `npm run build` 已同步改为 `pnpm install` / `pnpm run build`。

## 新命令
在 `MFCMouseEffect/WebUIWorkspace` 目录执行：

```bash
pnpm install
pnpm run build
```

## 验证
- 执行 `pnpm run build` 成功，产物正常输出并复制到 `MFCMouseEffect/WebUI`。
