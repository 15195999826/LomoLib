# 运行时属性编辑器（RapidPropertyEditor）

RapidPropertyEditor是一个运行时的属性编辑器组件，类似于虚幻引擎编辑器中的DetailsView组件，允许在游戏运行时查看和编辑UObject的属性。

## 基本功能

- 显示和编辑UObject的属性
- 支持基本类型：整数、浮点数、布尔值、字符串、名称、文本
- 支持复杂类型：TArray、TMap、自定义结构体(USTRUCT)
- 提供属性修改的回调机制

## 使用方式

### 1. 创建UI蓝图

1. 创建一个UserWidget蓝图
2. 添加一个ScrollBox作为容器
3. 在ScrollBox下添加一个URapidPropertyEditor控件（命名为PropertyEditor）

### 2. 在C++或蓝图中使用

#### C++示例

```cpp
// 获取属性编辑器控件
URapidPropertyEditor* PropertyEditor = YourWidget->GetPropertyEditor();

// 创建或获取要编辑的对象
UYourObject* YourObject = GetObjectToEdit();

// 设置对象到属性编辑器
PropertyEditor->SetObject(YourObject);

// 绑定属性修改事件
PropertyEditor->OnPropertyChanged.AddDynamic(this, &YourClass::HandlePropertyChanged);

// 属性修改处理函数
void YourClass::HandlePropertyChanged(UObject* Object, FName PropertyName, const FProperty* Property)
{
    // 处理属性修改逻辑
}
```

#### 蓝图示例

1. 在事件图表中获取要编辑的对象
2. 调用PropertyEditor的SetObject函数，传入对象
3. 如果需要监听属性变化，可以绑定OnPropertyChanged事件

### 3. 支持的属性类型和注意事项

- 只有带有EditAnywhere或BlueprintReadWrite标记的属性会被显示和编辑
- 基本数据类型(int32, float, bool, FString, FName, FText)
- 数组(TArray)：可以查看但目前不支持添加/删除元素
- 映射(TMap)：可以查看但目前不支持添加/删除键值对
- 结构体(USTRUCT)：支持嵌套结构体的属性编辑

## 高级用法

### 自定义属性显示名称

可以使用元数据为属性提供友好的显示名称：

```cpp
UPROPERTY(EditAnywhere, meta = (DisplayName = "友好名称"))
int32 MyProperty;
```

### 刷新与重置

- 调用`Refresh()`方法可以刷新属性显示
- 调用`ClearObject()`方法可以清空当前编辑的对象

## 技术实现

RapidPropertyEditor使用UE的反射系统来动态发现对象的属性，并为每种属性类型创建相应的编辑控件。它通过以下步骤工作：

1. 使用`TFieldIterator<FProperty>`遍历对象的所有属性
2. 根据属性类型创建相应的编辑控件(SpinBox, CheckBox, EditableTextBox等)
3. 将属性的当前值设置到相应的控件
4. 绑定控件的值变化事件以更新对象属性
5. 通过委托通知属性的变化

## 限制和未来改进

当前版本有以下限制：

1. 不支持在运行时添加/删除数组元素和映射键值对
2. 不支持对象引用类型的属性编辑
3. 不支持自定义属性编辑器
4. 不支持属性分类显示

未来计划添加的功能：

1. 支持数组和映射的添加/删除操作
2. 支持对象引用属性的选择和编辑
3. 添加属性分类和层次结构
4. 支持搜索和筛选功能
5. 改进视觉样式和用户体验

## 示例

项目中包含了一个示例，演示了如何使用RapidPropertyEditor：

- RapidPropertyEditorWidget: 示例UI小部件
- PropertyEditorTestObject: 测试对象，包含各种类型的属性

查看示例代码以了解如何在您的项目中集成和使用RapidPropertyEditor。 