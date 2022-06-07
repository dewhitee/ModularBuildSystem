// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HouseBuildSystemPreset.h"

#include "ModularBuildSystem.h"
#include "House/HouseBuildSystemGenerator.h"
#include "House/HouseBuildSystemActor.h"

void UHouseBuildSystemPreset::SaveProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
{
	UE_LOG(LogMBS, Log, TEXT("%s: Saving properties"), *GetName());
	if (const AHouseBuildSystemActor* BS = Cast<AHouseBuildSystemActor>(InBuildSystem.GetObject()))
	{
		Generator	= DuplicateObject<UHouseBuildSystemGenerator>(BS->Generator, this);
		Basement	= BS->Basement;
		Floors		= BS->Floors;
		Walls		= BS->Walls;
		Corners		= BS->Corners;
		Roof		= BS->Roof;
		Rooftop		= BS->Rooftop;
	}
}

void UHouseBuildSystemPreset::LoadProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
{
	UE_LOG(LogMBS, Log, TEXT("%s: Loading properties"), *GetName());
	if (AHouseBuildSystemActor* BS = Cast<AHouseBuildSystemActor>(InBuildSystem.GetObject()))
	{
		BS->Generator	= DuplicateObject<UHouseBuildSystemGenerator>(Generator, this);
		BS->Basement	= Basement;
		BS->Floors		= Floors;
		BS->Walls		= Walls;
		BS->Corners		= Corners;
		BS->Roof		= Roof;
		BS->Rooftop		= Rooftop;
	}
}
