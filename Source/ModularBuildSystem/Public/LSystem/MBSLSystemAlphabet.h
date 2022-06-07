// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MBSLSystemAlphabet.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSLSystemAlphabet : public UDataAsset
{
	GENERATED_BODY()

public:	
	/**
 	 * Definitions of all symbols available to the L-System.
 	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AdvancedDisplay))
	TMap<FName, FText> SymbolDefinitions;
};
