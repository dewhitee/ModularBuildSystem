// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MBSResolutionRecognizer.generated.h"

class UModularSectionResolution;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSResolutionRecognizer : public UObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MBS)
	bool bReturnUndefinedIfNotRecognized = false;
	
public:
	UFUNCTION(BlueprintCallable, Category=MBS)
	UModularSectionResolution* RecognizeFromSelection();
};
