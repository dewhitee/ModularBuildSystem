// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseEntranceGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Entrances")
class MODULARBUILDSYSTEM_API UHouseEntranceGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseEntranceGeneratorProperty()
	{
		Data.DebugPropertyName = "House Entrances";
	}
	
	/**
 	 * @brief Currently not supported
 	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Entrances", meta=(EditCondition="bEnabled"))
	int32 EntranceCount = 1;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Entrances", meta=(EditCondition="bEnabled"))
	bool bEntranceFromFront = true;
};
