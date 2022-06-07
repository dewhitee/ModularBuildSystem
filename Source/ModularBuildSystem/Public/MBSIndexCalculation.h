// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "UObject/NoExportTypes.h"
#include "MBSIndexCalculation.generated.h"

class AModularBuildSystemActor;

USTRUCT(BlueprintType)
struct FMBSIndexCalculationArgs
{
	GENERATED_BODY()

	FMBSIndexCalculationArgs() {};
	FMBSIndexCalculationArgs(AModularBuildSystemActor* BuildSystem, int32 AtIndex, TArray<int32>* OccupiedIndices,
		const FModularSectionInitializer* Initializer)
		: BuildSystem(BuildSystem)
		, AtIndex(AtIndex)
		, OccupiedIndices(OccupiedIndices)
		, Initializer(Initializer) {};
	
	TObjectPtr<AModularBuildSystemActor> BuildSystem = nullptr;
	int32 AtIndex = 0;
	TArray<int32>* OccupiedIndices = nullptr;
	const FModularSectionInitializer* Initializer = nullptr;
};

/**
 * Base class for all objects handling custom index calculation in MBS plugin.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class MODULARBUILDSYSTEM_API UMBSIndexCalculation : public UObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MBS)
	int32 MaxAdjustIterationCount = 10;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=MBS)
	int32 CalculateSingle(const FMBSIndexCalculationArgs& Args);
	virtual int32 CalculateSingle_Implementation(const FMBSIndexCalculationArgs& Args)
	{
		unimplemented();
		return 0;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(NativeBreakFunc))
	static void BreakIndexCalculationArgs(const FMBSIndexCalculationArgs& InArgs, AModularBuildSystemActor*& BuildSystem,
		int32& Index, FModularSectionInitializer& Initializer, TArray<int32>& OccupiedIndices);

protected:	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=MBS)
	int32 AdjustIndexIfPossible(UPARAM(ref) int32& Index, int32 TotalCount, const TArray<int32>& OccupiedIndices);
	virtual int32 AdjustIndexIfPossible_Implementation(UPARAM(ref) int32& Index, int32 TotalCount,
		const TArray<int32>& OccupiedIndices);
};
