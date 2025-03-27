# 运行时属性编辑器组件结构

## 目录结构

```
Plugins/LomoLib/Source/LomoLib/
├── Public/
│   └── RapidUI/
│       └── PropertyEditor/
│           ├── RapidPropertyEditor.h               # 主要的属性编辑器组件
│           ├── PropertyEditorTestObject.h          # 测试对象类定义
│           └── RapidPropertyEditorWidget.h         # 示例小部件
│
├── Private/
│   └── RapidUI/
│       └── PropertyEditor/
│           ├── RapidPropertyEditor.cpp             # 属性编辑器实现
│           ├── PropertyEditorTestObject.cpp        # 测试对象实现
│           └── RapidPropertyEditorWidget.cpp       # 示例小部件实现
│
└── project-docs/
    ├── PropertyEditor.md                          # 使用文档
    └── PropertyEditorStructure.md                 # 结构文档(本文件)
```

## 类和组件介绍

### URapidPropertyEditor

主要的属性编辑器控件，继承自UUserWidget，用于显示和编辑UObject的属性。

**主要功能**：
- 设置和获取目标对象
- 遍历并显示对象属性
- 为不同类型的属性创建编辑控件
- 处理属性值的变化
- 通知属性修改事件

**主要方法**：
- `SetObject(UObject* InObject)`: 设置要编辑的对象
- `GetObject()`: 获取当前编辑的对象
- `Refresh()`: 刷新属性显示
- `ClearObject()`: 清空当前对象

**主要事件**：
- `OnPropertyChanged`: 当属性被修改时触发

### UPropertyEditorTestObject

用于测试属性编辑器的示例对象类，包含各种不同类型的属性。

**包含属性类型**：
- 基本类型：int32, float, FString, bool, FName, FText
- 数组：TArray<int32>, TArray<FString>
- 映射：TMap<int32, FString>
- 结构体：FTestStruct (包含嵌套结构体FNestedTestStruct)

### URapidPropertyEditorWidget

示例小部件，展示如何使用RapidPropertyEditor组件。

**主要功能**：
- 创建测试对象
- 设置到属性编辑器
- 处理属性改变事件
- 提供刷新和重置功能

## 使用流程

1. 将RapidPropertyEditor添加到你的UUserWidget中
2. 创建或获取要编辑的UObject
3. 调用RapidPropertyEditor的SetObject方法，传入对象
4. 如果需要响应属性变化，绑定OnPropertyChanged事件
5. 使用Refresh方法在需要时刷新属性显示

## 关键实现细节

1. **对象属性的发现**：使用UE反射系统中的TFieldIterator<FProperty>遍历对象属性
2. **属性类型检测**：使用IsA<T>方法动态检测属性类型
3. **动态控件创建**：使用NewObject<T>方法在运行时创建适合的UI控件
4. **属性值获取和设置**：使用FProperty的方法获取和设置属性值
5. **结构体和容器处理**：使用特殊的辅助类(FScriptArrayHelper, FScriptMapHelper)处理复杂类型

## 扩展和自定义

要扩展RapidPropertyEditor支持新的属性类型或自定义UI，主要需要修改以下方法：

1. `CreatePropertyWidget`: 根据属性类型分派到特定的创建方法
2. `CreateBasicPropertyWidget`: 处理基本类型的属性
3. 添加新的特定类型创建方法，如`CreateCustomTypeWidget`
4. 添加相应的值变化处理方法

## 性能考虑

1. 属性编辑器使用动态创建的UI元素，对于拥有大量属性的对象可能会影响性能
2. 考虑为大型对象实现属性分类和分页显示
3. 可以通过缓存创建的控件来优化频繁刷新的场景

## 蓝图使用示例

要在蓝图中使用RapidPropertyEditor：

1. 创建一个继承自UUserWidget的蓝图
2. 添加一个ScrollBox控件
3. 在ScrollBox下添加一个RapidPropertyEditor控件
4. 在事件图表中，获取你想要编辑的对象并调用SetObject方法 