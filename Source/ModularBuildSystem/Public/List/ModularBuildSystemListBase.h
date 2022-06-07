// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ModularBuildSystemListBase.generated.h"

class UModularSectionResolution;

/**
 * 
 */
UCLASS(Abstract)
class MODULARBUILDSYSTEM_API UModularBuildSystemListBase : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/**
	 * @param Resolution Resolution of a list element to return length from.
	 * @return Length of a list.
	 */
	UFUNCTION(BlueprintCallable, Category=MBS)
	virtual int32 GetLength(UModularSectionResolution* Resolution) const
	{
		unimplemented();
		return -1;
	};

	UFUNCTION(BlueprintCallable, Category=MBS)
	int32 GetMaxIndex(UModularSectionResolution* Resolution) const;
};
