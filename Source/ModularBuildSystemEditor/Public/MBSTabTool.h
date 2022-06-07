// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSToolBase.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FTabTool : public FToolBase
{
public:
	FTabTool();
	virtual ~FTabTool() override;
	virtual void Initialize() override;
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) override;
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;
};
}
