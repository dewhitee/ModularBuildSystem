// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseRoofGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Roof")
class MODULARBUILDSYSTEM_API UHouseRoofGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseRoofGeneratorProperty()
	{
		Data.DebugPropertyName = "House Roof";
	}
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Roof", meta=(EditCondition="bEnabled"))
	FVector RoofOffset;
};
