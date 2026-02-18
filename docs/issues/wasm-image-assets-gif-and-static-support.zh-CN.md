# WASM 图片资源支持（GIF + 静态图）落地说明（2026-02）

## 目标
- 让 `spawn_image` 不再只依赖内置 `star/ripple`。
- 支持从插件目录读取图片资源：
  - `png`
  - `jpg/jpeg`
  - `bmp`
  - `gif`（按帧播放）
  - `tif/tiff`

## 配置方式
在 `plugin.json` 增加可选字段：

```json
{
  "id": "demo.click.image-pack.v1",
  "name": "Demo Click Image Pack",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm",
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif",
    "assets/coin.jpg"
  ]
}
```

说明：
- `image_assets` 使用**相对 `plugin.json` 的路径**；
- `imageId` 会按数组下标映射（越界按取模）；
- 若 `image_assets` 不存在/为空/文件无效，会自动回退到内置渲染器。

## 关键实现
1. 清单模型扩展
- `WasmPluginManifest` 增加 `image_assets` 解析和校验。
- 校验点：相对路径、禁止 `..` 上跳、扩展名白名单。

2. 资源目录缓存
- 新增 `WasmPluginImageAssetCatalog`：
  - 按 `manifestPath` + 文件时间缓存解析结果；
  - 运行时按 `imageId` 解析目标资源路径。

3. 文件渲染器
- 新增 `WasmImageFileRenderer`（`IRippleRenderer`）：
  - 加载图片文件并绘制；
  - GIF 读取帧延迟并按时间切帧；
  - 支持透明度与可选 tint。

4. 执行链路接入
- `DispatchRouter -> WasmClickCommandExecutor` 透传 `activeManifestPath`。
- `WasmRenderResourceResolver::CreateImageRendererById` 优先从插件图片资源创建渲染器，失败再回退内置。

## 影响文件
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginManifest.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginManifest.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginImageAssetCatalog.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginImageAssetCatalog.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmImageFileRenderer.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmImageFileRenderer.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmRenderResourceResolver.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmRenderResourceResolver.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

