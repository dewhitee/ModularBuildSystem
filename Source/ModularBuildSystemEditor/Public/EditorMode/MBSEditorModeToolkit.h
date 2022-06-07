// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSEditorModeWidget.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FEditorModeToolkit : public FModeToolkit
{
	TSharedPtr<MBS::SEditorModeWidget> MBSEditorModeWidget;
	
public:
	FEditorModeToolkit();
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;
	virtual FText GetToolPaletteDisplayName(FName Palette) const override;
	virtual void BuildToolPalette(FName Palette, FToolBarBuilder& ToolbarBuilder) override;
	virtual FText GetActiveToolDisplayName() const override;
	virtual FText GetActiveToolMessage() const override;
};
}
