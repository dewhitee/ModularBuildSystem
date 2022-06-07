// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseStairsGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Stairs")
class MODULARBUILDSYSTEM_API UHouseStairsGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseStairsGeneratorProperty()
	{
		Data.DebugPropertyName = "House Stairs";
	}
	virtual bool Init_Implementation(FMBSGeneratorPropertyInitArgs& Args) override;
};
