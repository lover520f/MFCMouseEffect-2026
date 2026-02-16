# WebUI 生成产物改为不入库

## 背景
- `MFCMouseEffect/WebUIWorkspace` 使用 Svelte 构建生成 `*.svelte.js`。
- 这些文件属于编译产物，不是源代码，持续入库会导致提交噪音和冲突成本上升。

## 变更
1. 更新 `.gitignore`，统一忽略前端构建产物目录：
   - `node_modules/`
   - `dist/`
2. 明确忽略 WebUI 复制产物：
   - `MFCMouseEffect/WebUI/*.svelte.js`
3. 将已被跟踪的 `MFCMouseEffect/WebUI/*.svelte.js` 从 Git 索引移除（仅取消跟踪，不删除本地文件）。

## 影响
- 仓库不再保存 Svelte 编译产物，只保存源码与构建脚本。
- 需要在本地先执行一次构建，确保 `MFCMouseEffect/WebUI` 下有最新 bundle，再运行设置页。

## 本地操作建议
在 `MFCMouseEffect/WebUIWorkspace` 目录执行：

```bash
pnpm install
pnpm run build
```

构建后会把产物复制到 `MFCMouseEffect/WebUI`，供应用运行时加载。

