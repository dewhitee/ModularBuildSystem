// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemEditor.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FToolBase : public IEditorModuleListener, public TSharedFromThis<FToolBase>
{
protected:	
	FName TabName;
	FText TabDisplayName;
	FText TooltipText;
	
public:
	FToolBase();
	virtual ~FToolBase() override;

	virtual void Initialize() {}
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) { return SNew(SDockTab); }
	virtual void MakeMenuEntry(FMenuBuilder& MenuBuilder);
	
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;
};
}
