// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSToolBase.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FEditorModeTool : public FToolBase
{
	static TSharedPtr<FSlateStyleSet> StyleSet;
	
public:
	virtual ~FEditorModeTool() override;
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

private:
	void RegisterStyleSet();
	void UnregisterStyleSet();
	void RegisterEditorMode();
	void UnregisterEditorMode();
};
}
