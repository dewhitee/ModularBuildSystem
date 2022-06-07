// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSTabTool.h"

#include "MBSTabToolPanel.h"

MBS::FTabTool::FTabTool()
{
}

MBS::FTabTool::~FTabTool()
{
}

void MBS::FTabTool::Initialize()
{
	TabName = "TabTool";
	TabDisplayName = FText::FromString("Tab Tool");
	TooltipText = FText::FromString("Tab Tool Window");
}

TSharedRef<SDockTab> MBS::FTabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STabToolPanel)
		];
	
	return SpawnedTab;
}

void MBS::FTabTool::OnStartupModule()
{
	MBS::FToolBase::OnStartupModule();
	FModularBuildSystemEditorModule::Get().AddMenuExtension(
		FMenuExtensionDelegate::CreateRaw(this, &MBS::FTabTool::MakeMenuEntry), FName("Section_2"));
}

void MBS::FTabTool::OnShutdownModule()
{
	MBS::FToolBase::OnShutdownModule();
}
