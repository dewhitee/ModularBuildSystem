// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemEditor.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FMenuTool : public IEditorModuleListener, public TSharedFromThis<FMenuTool>
{
	TSharedPtr<FUICommandList> CommandList;
public:
	FMenuTool();
	virtual ~FMenuTool() override;
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;
	void MakeMenuEntry(FMenuBuilder& MenuBuilder);
	void MakeSubMenu(FMenuBuilder& MenuBuilder);

protected:
	void MapCommands();

	// UI Command functions
	void MenuCommand1();
	void MenuCommand2();
	void MenuCommand3();
};
}
