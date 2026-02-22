# Stage54 - 清理未使用的 Legacy UI Filters

## 判定

`工程噪音`：`MFCMouseEffect.vcxproj.filters` 里残留了 `UI/Settings`、`UI/Frame`、`UI/Panes`、`UI/DocView` 相关 filter 定义，但当前工程没有任何文件再挂到这些 filter 下，影响工程视图可读性。

## 目标

1. 删除无引用的 legacy UI filters。
2. 不改任何编译项、不改运行时行为，仅做工程视图层清理。

## 变更摘要

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

删除以下未使用 filter 定义：

- `头文件\UI\Settings`
- `源文件\UI\Settings`
- `头文件\UI\Frame`
- `源文件\UI\Frame`
- `头文件\UI\Panes`
- `源文件\UI\Panes`
- `头文件\UI\DocView`
- `源文件\UI\DocView`

## 验证

1. 扫描 `MFCMouseEffect.vcxproj.filters`，上述 filter 定义已不存在。
2. 编译项未变更（本次不涉及 `vcxproj` 编译输入修改）。

## 收益

1. VS 工程过滤器更贴近当前真实代码结构，降低误判“还有 UI 编译链”的噪音。
2. 为后续继续平台化迁移提供更清晰的工程视图基线。
