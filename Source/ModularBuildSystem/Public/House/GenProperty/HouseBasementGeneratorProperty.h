// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseBasementGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Basement")
class MODULARBUILDSYSTEM_API UHouseBasementGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:	
	UHouseBasementGeneratorProperty()
	{
		Data.DebugPropertyName = "House Basement";
	}
};
