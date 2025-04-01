# RapidUIRoot使用指南

RapidUIRoot是UI管理系统的核心组件，用于管理所有UI Widget。本文档说明了如何使用RapidUIRoot的各种功能，特别是新增的Widget查询功能。

## 基础架构

RapidUIRoot采用分层的Overlay设计，从下至上依次为：

1. **BackEnd** - 底层UI，通常作为背景或基础界面
2. **Main** - 主要UI层，放置游戏中的主要界面元素
3. **FrontEnd** - 前端UI层，用于放置提示、弹窗等需要显示在最上层的元素

此外，还有一个Hidden区域用于存放暂时不显示的Widget。

## 新增的Widget管理功能

### 添加Widget

添加Widget时，可以使用带名称的方法，便于后续查询和管理：

```cpp
// C++中使用
UIRoot->PushUIWithName(ERapidUIOverlayType::Main, MyWidget, FName("InventoryWidget"));
```

```
// 蓝图中使用
PushUIWithName(Main, InventoryWidget, "InventoryWidget")
```

### 查询Widget

#### 通过名称查询

```cpp
// C++中查询特定名称的Widget
UUserWidget* InventoryWidget = UIRoot->FindWidgetByName(FName("InventoryWidget"));
```

```
// 蓝图中查询
InventoryWidget = FindWidgetByName("InventoryWidget")
```

#### 通过类型查询

```cpp
// C++中查询特定类型的第一个Widget
UInventoryWidget* InvWidget = Cast<UInventoryWidget>(UIRoot->FindWidgetByClass(UInventoryWidget::StaticClass()));
```

```
// 蓝图中查询特定类型的所有Widget
Array of UserWidget = FindAllWidgetsByClass(InventoryWidget Class)
```

### 移除Widget

```cpp
// C++中移除特定名称的Widget
bool Success = UIRoot->RemoveWidgetByName(FName("InventoryWidget"));
```

```
// 蓝图中移除
RemoveWidgetByName("InventoryWidget")
```

### 注册已存在的Widget

对于通过其他方式添加到界面的Widget，可以手动注册到映射表中，以便进行管理：

```cpp
// C++中注册已存在的Widget
UIRoot->RegisterExistingWidget(ExistingWidget, FName("ExistingWidgetName"));
```

```
// 蓝图中注册
RegisterExistingWidget(ExistingWidget, "ExistingWidgetName")
```

## 使用示例

### 场景1：创建并查询主菜单

```cpp
// 创建并添加主菜单
UMainMenuWidget* MainMenu = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuClass);
UIRoot->PushUIWithName(ERapidUIOverlayType::Main, MainMenu, FName("MainMenu"));

// 之后在需要时查询主菜单
UMainMenuWidget* Menu = Cast<UMainMenuWidget>(UIRoot->FindWidgetByName(FName("MainMenu")));
if (Menu)
{
    Menu->UpdateMenuItems();
}
```

### 场景2：查找所有对话框并关闭

```cpp
// 查找所有对话框并关闭
TArray<UUserWidget*> AllDialogs = UIRoot->FindAllWidgetsByClass(UDialogWidget::StaticClass());
for (UUserWidget* Dialog : AllDialogs)
{
    UIRoot->RemoveWidgetByName(/* 需要先获取Dialog对应的名称 */);
}
```

## 最佳实践

1. **命名规范**：为Widget指定有意义、唯一的名称，推荐使用类似`类型_功能_ID`的格式，如`Dialog_Quest_101`
2. **类型管理**：如果需要批量管理特定类型的Widget，使用`FindAllWidgetsByClass`方法
3. **生命周期管理**：当Widget不再需要时，记得使用`RemoveWidgetByName`方法移除，避免内存泄漏
4. **注册现有Widget**：对于使用其他方式添加的Widget，使用`RegisterExistingWidget`方法注册到系统中

## 注意事项

1. 通过`FindWidgetByName`查询不存在的Widget将返回nullptr，使用前请进行空检查
2. `RemoveWidgetByName`会同时从界面和管理系统中移除Widget
3. 当多次添加同名Widget时，新的Widget将覆盖旧的映射关系 