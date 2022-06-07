// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorMode/MBSEditorModeTool.h"
#include "EditorMode/MBSEditorMode.h"
#include "UnrealEd.h"
#include "EditorModeRegistry.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> MBS::FEditorModeTool::StyleSet = nullptr;

MBS::FEditorModeTool::~FEditorModeTool()
{
}

void MBS::FEditorModeTool::OnStartupModule()
{
	RegisterStyleSet();
	RegisterEditorMode();
}

void MBS::FEditorModeTool::OnShutdownModule()
{
	UnregisterStyleSet();
	UnregisterEditorMode();
}

void MBS::FEditorModeTool::RegisterStyleSet()
{
	const FVector2D Icon20x20(20.f, 20.f);
	const FVector2D Icon40x40(40.f, 40.f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	const FString StyleSetPath = FPaths::ProjectPluginsDir() / TEXT("ModularBuildSystem") / TEXT("Resources");
	StyleSet = MakeShareable(new FSlateStyleSet("MBSEditorModeToolStyle"));
	StyleSet->SetContentRoot(StyleSetPath);
	StyleSet->SetCoreContentRoot(StyleSetPath);

	// todo: Spline editor
	{
		StyleSet->Set("MBSEditorMode", new IMAGE_BRUSH(TEXT("MBS_EdMode_Logo_40x40"), Icon40x40));
		StyleSet->Set("MBSEditorMode.Small", new IMAGE_BRUSH(TEXT("MBS_EdMode_Logo_40x40"), Icon20x20));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void MBS::FEditorModeTool::UnregisterStyleSet()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

void MBS::FEditorModeTool::RegisterEditorMode()
{
	FEditorModeRegistry::Get().RegisterMode<MBS::FEditorMode>(
		MBS::FEditorMode::EM_MBS,
		FText::FromString("MBS Editor Mode"),
		FSlateIcon(StyleSet->GetStyleSetName(), "MBSEditorMode", "MBSEditorMode.Small"),
		true, 500);
}

void MBS::FEditorModeTool::UnregisterEditorMode()
{
	FEditorModeRegistry::Get().UnregisterMode(MBS::FEditorMode::EM_MBS);
}

#undef IMAGE_BRUSH
