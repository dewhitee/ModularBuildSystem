// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseChimneyGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Chimney")
class MODULARBUILDSYSTEM_API UHouseChimneyGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseChimneyGeneratorProperty()
	{
		Data.DebugPropertyName = "House Chimney";
	}
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chimney", meta=(EditCondition="bEnabled"))
	FVector ChimneyOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chimney", meta=(EditCondition="bEnabled"))
	int32 ChimneyIndex;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chimney", meta=(EditCondition="bEnabled"))
	bool bRandomChimneyIndex;
};
