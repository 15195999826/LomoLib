// StructToJsonWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PropertyEditorModule.h"
#include "IStructureDetailsView.h"

class SStructToJsonWidget : public SCompoundWidget
{
public:
    virtual ~SStructToJsonWidget() override;
    
    SLATE_BEGIN_ARGS(SStructToJsonWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    
private:
    FReply OnSelectStructClicked();
    FReply OnExportToJsonClicked();
    bool IsStructSelected() const;
    TSharedPtr<SBox> DetailsViewBox;
    TSharedPtr<IStructureDetailsView> StructureDetailsView;
    UScriptStruct* SelectedStruct;
    TSharedPtr<FStructOnScope> StructData;
    FString SavePath;
};