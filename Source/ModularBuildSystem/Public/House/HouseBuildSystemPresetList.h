// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "List/ModularBuildSystemPresetList.h"
#include "HouseBuildSystemPresetList.generated.h"

class UHouseBuildSystemPreset;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UHouseBuildSystemPresetList : public UModularBuildSystemPresetList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<UHouseBuildSystemPreset*> Presets;
		
};
