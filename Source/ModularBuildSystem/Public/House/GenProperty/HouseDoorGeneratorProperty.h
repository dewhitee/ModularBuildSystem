// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseDoorGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Doors")
class MODULARBUILDSYSTEM_API UHouseDoorGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseDoorGeneratorProperty()
	{
		Data.DebugPropertyName = "House Doors";
	}
};
