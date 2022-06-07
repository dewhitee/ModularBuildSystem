// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSToolBase.h"

MBS::FToolBase::FToolBase()
{
}

MBS::FToolBase::~FToolBase()
{
}

void MBS::FToolBase::MakeMenuEntry(FMenuBuilder& MenuBuilder)
{
	FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(MenuBuilder, TabName);
}

void MBS::FToolBase::OnStartupModule()
{
	Initialize();
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabName, FOnSpawnTab::CreateRaw(this, &MBS::FToolBase::SpawnTab))
		.SetGroup(FModularBuildSystemEditorModule::Get().GetMenuRoot())
		.SetDisplayName(TabDisplayName)
		.SetTooltipText(TooltipText);
}

void MBS::FToolBase::OnShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}
