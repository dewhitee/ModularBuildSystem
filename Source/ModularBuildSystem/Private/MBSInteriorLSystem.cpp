// Fill out your copyright notice in the Description page of Project Settings.


#include "Interior/MBSInteriorLSystem.h"

#include "ModularBuildSystemActor.h"
#include "Interior/MBSInterior.h"
#include "Interior/MBSInteriorGenerator.h"

bool UMBSInteriorLSystem::PreRun(FMBSLSystemRunArgs& Args)
{
	Super::PreRun(Args);
	if (!Args.BS)
	{
		return false;
	}

	verifyf((Interior = Args.BS->GetInterior()), TEXT("%s: BuildSystem's Interior object was nullptr; This should never happen."),
		*GetName());

	//LayoutRooms();
	return true;
}

bool UMBSInteriorLSystem::PostRun(FMBSLSystemRunArgs& Args)
{
	return Super::PostRun(Args);
}

void UMBSInteriorLSystem::PostSymbolConsumed(FMBSLSystemNextArgs& Args)
{
	Super::PostSymbolConsumed(Args);
	if (Args.OutSpawnedActor)
	{
		// Register to the room
	}
}

void UMBSInteriorLSystem::Next_Implementation(FMBSLSystemNextArgs& Args)
{
	Super::Next_Implementation(Args);
}

void UMBSInteriorLSystem::LayoutRooms()
{
	
}

void UMBSInteriorLSystem::PlaceWalls()
{
}

void UMBSInteriorLSystem::AddEntrances()
{
}

void UMBSInteriorLSystem::PlaceItems()
{
}
