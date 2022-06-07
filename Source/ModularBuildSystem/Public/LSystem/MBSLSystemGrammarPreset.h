// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSLSystem.h"
#include "Engine/DataAsset.h"
#include "MBSLSystemGrammarPreset.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSLSystemGrammarPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Initial state of an L-System.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Axiom;

	/**
	 * Map of user defined rules where Key is a single symbol and Value is a string that defines a rule for it.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FName, FString> Rules;

	/**
	 * Definitions of all symbols available to the L-System.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMBSLSystemAlphabet> Alphabet;
};
