// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseRooftopGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Rooftop")
class MODULARBUILDSYSTEM_API UHouseRooftopGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseRooftopGeneratorProperty()
	{
		Data.DebugPropertyName = "House Rooftop";
	}
};
