// StructToJsonWidget.cpp

#include "Struct2JsonWidget.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Framework/Application/SlateApplication.h"

class FStructFilter : public IClassViewerFilter
{
public:
    virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
        return InClass->IsChildOf(UScriptStruct::StaticClass());
    }

    virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
        return InUnloadedClassData->IsChildOf(UScriptStruct::StaticClass());
    }
};

SStructToJsonWidget::~SStructToJsonWidget()
{
    if (DetailsViewBox.IsValid())
    {
        DetailsViewBox->SetContent(SNullWidget::NullWidget);
    }

    if (StructureDetailsView.IsValid())
    {
        StructureDetailsView->SetStructureData(nullptr);
        StructureDetailsView.Reset();
    }

    // 然后清理 StructData
    if (StructData.IsValid())
    {
        StructData.Reset();
    }

    SelectedStruct = nullptr;
}

void SStructToJsonWidget::Construct(const FArguments& InArgs)
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    
    FStructureDetailsViewArgs StructureViewArgs;
    StructureViewArgs.bShowObjects = true;
    StructureViewArgs.bShowAssets = true;
    
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bHideSelectionTip = false;
    DetailsViewArgs.bLockable = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bShowOptions = true;

    StructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
        DetailsViewArgs,
        StructureViewArgs,
        nullptr);
    
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("选择结构体")))
            .OnClicked(this, &SStructToJsonWidget::OnSelectStructClicked)
        ]
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(5)
        [
            SAssignNew(DetailsViewBox, SBox)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("导出为JSON")))
            .OnClicked(this, &SStructToJsonWidget::OnExportToJsonClicked)
            .IsEnabled(this, &SStructToJsonWidget::IsStructSelected)
        ]
    ];
}

bool SStructToJsonWidget::IsStructSelected() const
{
    return SelectedStruct && StructData.IsValid();
}

FReply SStructToJsonWidget::OnSelectStructClicked()
{
    // 创建选择对话框
    TArray<const UScriptStruct*> OutStructs;

    FString ProjectName = FApp::GetProjectName();
    
    // 收集所有已注册的结构体
    for (TObjectIterator<UScriptStruct> It; It; ++It)
    {
        const UScriptStruct* Struct = *It;

        if (Struct->GetOutermost() == GetTransientPackage())
        {
            continue;
        }
        // PathName 打印结果为: /Script/ProjectName.StructName
        FString PathName = Struct->GetPathName();
        PathName = PathName.Replace(TEXT("/Script/"), TEXT(""));
        if (PathName.StartsWith(ProjectName))
        {
            OutStructs.Add(Struct);
        }
    }
    
    // 获取当前 StructToJsonWidget 所在的窗口
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (!ParentWindow.IsValid())
    {
        ParentWindow = FGlobalTabmanager::Get()->GetRootWindow();
    }
    
    // 显示结构体选择对话框
    TSharedRef<SWindow> PickerWindow = SNew(SWindow)
        .Title(NSLOCTEXT("StructToJson", "PickStruct", "选择一个结构体"))
        .ClientSize(FVector2D(500, 600))
        .SupportsMinimize(false)
        .SupportsMaximize(false);
    
    TSharedPtr<SListView<const UScriptStruct*>> ListView;
    
    PickerWindow->SetContent(
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("Menu.Background"))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SAssignNew(ListView, SListView<const UScriptStruct*>)
                .ListItemsSource(&OutStructs)
                .OnGenerateRow_Lambda([](const UScriptStruct* InStruct, const TSharedRef<STableViewBase>& OwnerTable) {
                    return SNew(STableRow<const UScriptStruct*>, OwnerTable)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(InStruct->GetName()))
                    ];
                })
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(NSLOCTEXT("StructToJson", "Select", "选择"))
                    .OnClicked_Lambda([this, ListView, &PickerWindow]() {
                        if (ListView->GetNumItemsSelected() > 0)
                        {
                            UScriptStruct* SelectedStruct = const_cast<UScriptStruct*>(ListView->GetSelectedItems()[0]);
                            this->SelectedStruct = SelectedStruct; // 直接赋值，不使用 MakeShareable
                            this->StructData = MakeShareable(new FStructOnScope(SelectedStruct));
                            this->StructureDetailsView->SetStructureData(this->StructData);
                            this->DetailsViewBox->SetContent(this->StructureDetailsView->GetWidget().ToSharedRef());
                        }
                        PickerWindow->RequestDestroyWindow();
                        return FReply::Handled();
                    })
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5, 0, 0, 0)
                [
                    SNew(SButton)
                    .Text(NSLOCTEXT("StructToJson", "Cancel", "取消"))
                    .OnClicked_Lambda([&PickerWindow]() {
                        PickerWindow->RequestDestroyWindow();
                        return FReply::Handled();
                    })
                ]
            ]
        ]
    );
    
    FSlateApplication::Get().AddModalWindow(PickerWindow, ParentWindow);
    
    return FReply::Handled();
}

FReply SStructToJsonWidget::OnExportToJsonClicked()
{
    if (!SelectedStruct || !StructData.IsValid())
    {
        return FReply::Handled();
    }

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> SaveFilenames;
        FString DefaultPath = FPaths::ProjectContentDir();
        FString DefaultFile = SelectedStruct->GetName() + TEXT(".txt");

        const bool bFileSelected = DesktopPlatform->SaveFileDialog(
            FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle(),
            TEXT("保存JSON文件"),
            DefaultPath,
            DefaultFile,
            TEXT("JSON文件|*.txt"),
            EFileDialogFlags::None,
            SaveFilenames
        );

        if (bFileSelected && SaveFilenames.Num() > 0)
        {
            SavePath = SaveFilenames[0];
            FString JsonString;
            bool bSuccess = false;
            FString MessageContent;
            
            // 创建临时变量来存储结构体数据的副本
            UScriptStruct* TempStruct = SelectedStruct;
            void* TempMemory = FMemory::Malloc(TempStruct->GetStructureSize());
            TempStruct->InitializeStruct(TempMemory);
            TempStruct->CopyScriptStruct(TempMemory, StructData->GetStructMemory());
            
            {
                TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
                FJsonObjectConverter::UStructToJsonObject(TempStruct, TempMemory, 
                    JsonObject.ToSharedRef(), 0, 0);
                
                TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
                FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
            }
            
            // 清理临时内存
            TempStruct->DestroyStruct(TempMemory);
            FMemory::Free(TempMemory);
            
            if (FFileHelper::SaveStringToFile(JsonString, *SavePath))
            {
                MessageContent = FString::Printf(TEXT("已成功导出JSON到: %s"), *SavePath);
                bSuccess = true;
            }
            else
            {
                MessageContent = TEXT("保存JSON文件失败");
            }
            
            // 显示消息
            if (bSuccess)
            {
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(MessageContent));
            }
            else
            {
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(MessageContent));
            }
        }
    }

    return FReply::Handled();
}