// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LSystem/MBSLSystem.h"
#include "MBSInteriorLSystem.generated.h"

class UMBSInterior;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSInteriorLSystem : public UMBSLSystem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMBSInterior> Interior;
	
protected:
	virtual bool PreRun(FMBSLSystemRunArgs& Args) override;
	virtual bool PostRun(FMBSLSystemRunArgs& Args) override;

	virtual void PostSymbolConsumed(FMBSLSystemNextArgs& Args) override;
	
	virtual void Next_Implementation(FMBSLSystemNextArgs& Args) override;
	
	virtual void LayoutRooms();
	virtual void PlaceWalls();
	virtual void AddEntrances();
	virtual void PlaceItems();
};
