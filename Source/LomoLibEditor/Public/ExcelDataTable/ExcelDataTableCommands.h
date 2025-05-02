// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * Excel DataTable 转换器的命令类
 */
class FExcelDataTableCommands : public TCommands<FExcelDataTableCommands>
{
public:
	FExcelDataTableCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	// Commands
	TSharedPtr<FUICommandInfo> BatchImport;
	TSharedPtr<FUICommandInfo> BatchExport;
	TSharedPtr<FUICommandInfo> OpenSettings;
}; 