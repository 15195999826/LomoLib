# LomoLib

LomoLib 是一个通用的 Unreal Engine 5 插件库，提供了一系列实用的功能模块和工具，帮助开发者更高效地进行游戏开发。

## 📋 功能模块

### 🎬 程序动画系统 (Program Animation)
- **UProgramAniComponent**: 通用程序动画组件
- **FProgramAnimationData**: 动画数据资产支持
- **ProgramAnimationSettings**: 动画配置和反转动画生成
- **编辑器工具**: 一键刷新动画资产和生成反转动画

### 🚀 预加载系统 (Preload System)
- **UPreloadService**: 预加载服务管理
- **IPreloadProvider**: 预加载提供者接口
- **AGeneralPreloaderActor**: 通用预加载器Actor
- **APreloadActor**: 预加载Actor基类

### ⏳ 等待组管理 (Wait Group Management)
- **UWaitGroupManager**: 等待组管理器
- **UWaitGroupTask**: 等待组任务
- **LomoWaitGroup**: 等待组功能封装

### 🖼️ UI 管理系统 (RapidUI)
- **RapidUIRoot**: 分层Overlay UI管理器，支持BackEnd/Main/FrontEnd层级
- **RapidButtonGroup**: 按钮组管理
- **RapidModalWidget**: 模态对话框系统
- **RapidJsonPanel**: JSON数据面板
- **RapidIntPayloadBtn**: 带整型数据的按钮组件

### 📊 Excel-DataTable 转换工具
- **双向转换**: Excel ↔ DataTable 数据转换
- **批量处理**: 支持批量导入/导出操作
- **类型支持**: 支持所有 UE5 可序列化类型
- **编辑器集成**: 工具菜单集成，便于团队协作

### 🎮 通用游戏组件
- **LomoGeneralPlayerController**: 通用玩家控制器
- **LomoGeneralPlayerPawn**: 通用玩家Pawn
- **LomoPlayerInput**: 玩家输入处理
- **GeneralPoolActor**: 通用对象池Actor
- **SpringArmCameraActor**: 弹簧臂相机Actor

### 🔧 实用工具类
- **LomoLibBlueprintFunctionLibrary**: 蓝图函数库
- **AsyncWaitForLevelStatus**: 异步等待关卡状态
- **RegisterInputActionTask**: 注册输入动作任务
- **EmptyActor**: 空Actor基类
- **GValid**: 有效性验证宏

## 🎯 核心特性

### 程序动画系统
```cpp
// 播放命名动画
ProgramAniComponent->PlayProgramAnimation(
    FName("JumpAnimation"), 
    StartLocation, 
    EndLocation
);

// 反转播放上一个动画
ProgramAniComponent->ReverseLastAnimation();
```

### RapidUI 系统
```cpp
// 分层UI管理
UIRoot->PushUIWithName(ERapidUIOverlayType::Main, MyWidget, FName("InventoryWidget"));

// 通过名称查询Widget
UUserWidget* Widget = UIRoot->FindWidgetByName(FName("InventoryWidget"));
```

### 预加载系统
```cpp
// 注册预加载提供者
PreloadService->RegisterProvider(MyPreloadProvider);

// 执行预加载
PreloadService->StartPreload();
```

### Excel-DataTable 工具
- **编辑器菜单**: Tools → LomoLib → Excel DataTable Converter
- **支持格式**: CSV/Excel 文件格式
- **配置管理**: 项目设置 → LomoLib → Excel DataTable Settings

## 📁 项目结构

```
LomoLib/
├── Source/
│   ├── LomoLib/              # 运行时模块
│   │   ├── Animation/        # 程序动画系统
│   │   ├── Preload/          # 预加载系统
│   │   ├── RapidUI/          # UI管理系统
│   │   └── Settings/         # 配置管理
│   ├── LomoLibEditor/        # 编辑器模块
│   │   ├── ExcelDataTable/   # Excel转换工具
│   │   └── ProgramAnimation/ # 动画编辑器工具
│   └── LomoLibTest/          # 测试模块
└── Config/                   # 配置文件
```

## ⚙️ 系统要求

- **Unreal Engine**: 5.0+
- **平台支持**: Windows, Mac, Linux
- **依赖插件**: Enhanced Input

## 🚀 快速开始

1. 将 LomoLib 插件放置到项目的 `Plugins/` 目录下
2. 重新生成项目文件并编译
3. 在项目设置中启用 LomoLib 插件
4. 在 C++ 或蓝图中开始使用各种功能模块

## 📖 文档

- [Excel-DataTable 转换工具详细说明](Source/LomoLibEditor/project-docs/excel-datatable-converter.md)
- [RapidUI 系统使用指南](Source/LomoLib/project-docs/RapidUIRoot.md)

## 🔄 版本信息

- **当前版本**: 1.0
- **开发者**: lomo
- **状态**: Beta 版本

## 📝 更新日志

### v1.0 (当前版本)
- ✅ 程序动画系统完整实现
- ✅ 预加载系统架构完成
- ✅ RapidUI 分层管理系统
- ✅ Excel-DataTable 双向转换工具
- ✅ 等待组管理功能
- ✅ 通用游戏组件集合

---

💡 **提示**: 本插件专注于提供稳定、高效的通用功能模块，帮助开发者快速构建游戏项目的基础架构。如有问题或建议，欢迎反馈。