// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseCornerGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Corners")
class MODULARBUILDSYSTEM_API UHouseCornerGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseCornerGeneratorProperty()
	{
		Data.DebugPropertyName = "House Corners";
	}
};
