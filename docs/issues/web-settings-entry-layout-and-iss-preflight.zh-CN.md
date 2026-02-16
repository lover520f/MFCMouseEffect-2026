# WebUI 入口收拢与安装包预检

## 问题
1. `WebUIWorkspace/src` 根目录存在多个 `*-main.js`，入口脚本和业务模块混在一起，目录观感与维护成本都偏高。
2. WebUI 的 Svelte 编译产物已改为不入库后，如果发布前忘记构建，安装包可能出现“静默漏带前端资源”的风险。

## 方案
### 1) 入口目录收拢
- 将多入口脚本统一迁移到 `MFCMouseEffect/WebUIWorkspace/src/entries/`：
  - `main.js`
  - `general-main.js`
  - `effects-main.js`
  - `text-main.js`
  - `trail-main.js`
  - `input-indicator-main.js`
  - `automation-main.js`
  - `dialog-main.js`
  - `shell-main.js`
- 同步更新 `vite.config.js` 入口解析为表驱动映射，降低重复条件分支。

### 2) 安装包脚本预检
- 更新 `Install/MFCMouseEffect.iss`：
  - 增加 Release 产物预检（`MFCMouseEffect.exe` + WebUI 关键 `.svelte.js` bundle）。
  - 预检失败直接 `#error`，阻止继续打包。
- `webui` 文件复制项去掉 `skipifsourcedoesntexist`，避免静默漏包。

### 3) 构建复制阶段清理陈旧产物
- 更新 `MFCMouseEffect/MFCMouseEffect.vcxproj` 的 x64 `PostBuildEvent`：
  - 复制前先 `rmdir /S /Q "$(OutDir)webui"` 清空目标目录。
  - 再从 `$(ProjectDir)WebUI` 执行 `xcopy`。
- 目的：避免旧版遗留 js 长期滞留在 `x64\\{Debug|Release}\\webui`，被安装包 wildcard 误打入。

## 验证
1. `pnpm run build`（`MFCMouseEffect/WebUIWorkspace`）通过。
2. `MSBuild x64 Release` 通过（若可执行文件占用，先自动 `taskkill` 再重试）。
3. `ISCC Install/MFCMouseEffect.iss` 编译通过，生成：
   - `Install/Output/MFCMouseEffect_1.3.0_Setup_x64.exe`

## 结果
- `src` 根目录不再散落入口脚本，结构更清晰。
- 安装包流程具备“关键 WebUI 产物缺失即失败”的保护，不再默默漏带前端资源。

