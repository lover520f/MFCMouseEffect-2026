# json.hpp（2.5w 行）治理：依赖边界收口与代码量统计排除

## 1. 背景

`MFCMouseEffect/MouseFx/ThirdParty/json.hpp` 是第三方单头文件，体量约 2.5 万行。  
这部分代码不属于项目自研逻辑，但此前业务代码直接 include 该文件，且统计口径容易把它算进“项目代码量”。

## 2. 判定

判定：`架构治理`（非功能 Bug）。  
目标：

- 让业务层不再直接依赖第三方头路径，形成清晰依赖边界。
- 让代码量统计默认排除第三方与生成目录，反映真实自研规模。

## 3. 实施内容

### 3.1 JSON 依赖边界收口

- 新增：`MFCMouseEffect/MouseFx/Core/Json/JsonFacade.h`
  - 作为唯一 JSON 依赖入口。
  - 内部再 include `MouseFx/ThirdParty/json.hpp`。
- 业务代码改为 include facade，不再直接 include 第三方头：
  - `MouseFx/Core/Config/EffectConfigJsonCodec.h`
  - `MouseFx/Core/Config/EffectConfigJsonCodecEffectsColorHelpers.h`
  - `MouseFx/Core/Control/AppController.cpp`
  - `MouseFx/Core/Control/CommandHandler.cpp`
  - `MouseFx/Core/Control/CommandHandler.ApplySettings.cpp`
  - `MouseFx/Core/Wasm/WasmPluginManifest.cpp`
  - `MouseFx/Server/SettingsSchemaBuilder.cpp`
  - `MouseFx/Server/SettingsStateMapper.cpp`
  - `MouseFx/Server/WebSettingsServer.Routing.cpp`

### 3.2 工程文件调整

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - 增加 `MouseFx/Core/Json/JsonFacade.h`
  - 移除 `MouseFx/ThirdParty/json.hpp` 的项目头文件展示项（仍保留物理文件供 facade 使用）
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
  - 增加 `JsonFacade.h` 过滤器项

### 3.3 代码量统计排除规则

- 新增仓库根：`.clocignore`
  - 默认排除：
    - `MFCMouseEffect/MouseFx/ThirdParty`
    - `MFCMouseEffect/WasmRuntimeBridge/third_party`
    - `MFCMouseEffect/WebUIWorkspace/node_modules`
    - `MFCMouseEffect/WebUI`
    - `x64`
    - `Win32`

### 3.4 低风险“瘦身宏”尝试（JsonFacade）

- 在 `JsonFacade.h` 增加：
  - `JSON_NO_IO=1`
  - `JSON_USE_GLOBAL_UDLS=0`
  - `JSON_DISABLE_ENUM_SERIALIZATION=1`
- 目的：关闭项目运行路径未使用的 JSON 功能分支，降低模板实例化面。

## 4. 验证

- 本次改动不改变 JSON 解析/序列化行为，仅做 include 路径与统计口径治理。
- 业务代码编译入口保持一致（通过 facade 间接使用同一份 `nlohmann::json`）。
- 构建验证：
  - `Release|x64`：通过
  - `Debug|x64`：通过
- 体积观察：
  - `x64/Release/MFCMouseEffect.exe` 仍为 `1,075,200` bytes（与变更前一致）
  - 结论：`json.hpp` 文件本身体积并不会直接等比例进入 EXE；当前瓶颈不在这部分。

## 5. 后续建议

- 后续若替换 JSON 库（如降体积或跨平台收敛），只需在 `JsonFacade.h` 层演进，避免全项目散点替换。
- 若目标是继续压缩 EXE，优先从渲染/功能模块裁剪与链接策略（而非 `json.hpp` 源文件体积）入手。
