// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorMode/MBSEditorModeToolkit.h"

#include "EditorMode/MBSEditorMode.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "MBSEditorMode"

namespace
{
	static const FName MBSName = TEXT("MBS");
	const TArray<FName> MBSPaletteNames = { MBSName };
}

MBS::FEditorModeToolkit::FEditorModeToolkit()
{
	//SAssignNew(MBSEditorModeWidget, SMBSEditorModeWidget);
}

void MBS::FEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	SAssignNew(MBSEditorModeWidget, MBS::SEditorModeWidget);
	FModeToolkit::Init(InitToolkitHost);
}

FName MBS::FEditorModeToolkit::GetToolkitFName() const
{
	return FName("MBSEditorMode");
}

FText MBS::FEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "MBS");
	//return NSLOCTEXT("BuilderModeToolkit", "DisplayName", "Builder");
}

FEdMode* MBS::FEditorModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(MBS::FEditorMode::EM_MBS);
}

TSharedPtr<SWidget> MBS::FEditorModeToolkit::GetInlineContent() const
{
	return MBSEditorModeWidget;
}

void MBS::FEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames = MBSPaletteNames;
}

FText MBS::FEditorModeToolkit::GetToolPaletteDisplayName(FName Palette) const
{
	return Palette == MBSName ? LOCTEXT("MBS", "MBS") : FText();
}

void MBS::FEditorModeToolkit::BuildToolPalette(FName Palette, FToolBarBuilder& ToolbarBuilder)
{
	if (Palette == MBSName)
	{
		MBSEditorModeWidget->CustomizeToolBarPalette(ToolbarBuilder);
	}
}

FText MBS::FEditorModeToolkit::GetActiveToolDisplayName() const
{
	return MBSEditorModeWidget->GetActiveToolName();
}

FText MBS::FEditorModeToolkit::GetActiveToolMessage() const
{
	return MBSEditorModeWidget->GetActiveToolMessage();
}

#undef LOCTEXT_NAMESPACE
