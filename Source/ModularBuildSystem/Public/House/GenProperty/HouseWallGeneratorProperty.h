// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseWallGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Walls")
class MODULARBUILDSYSTEM_API UHouseWallGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseWallGeneratorProperty()
	{
		Data.DebugPropertyName = "House Walls";
	}
	
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Walls", meta=(EditCondition="bSupported"))
	//int32 DefaultWallMeshIndex = 0;
};
