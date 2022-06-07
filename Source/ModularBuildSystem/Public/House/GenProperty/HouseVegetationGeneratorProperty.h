// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseVegetationGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Vegetation")
class MODULARBUILDSYSTEM_API UHouseVegetationGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseVegetationGeneratorProperty()
	{
		Data.DebugPropertyName = "House Vegetation";	
	}
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Vegetation", meta=(EditCondition="bEnabled"))
	FVector VegetationOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Vegetation", meta=(EditCondition="bEnabled"))
	int32 VegetationCount;
};
