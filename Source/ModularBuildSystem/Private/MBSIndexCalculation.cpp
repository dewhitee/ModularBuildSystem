// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSIndexCalculation.h"
#include "ModularBuildSystem.h"

void UMBSIndexCalculation::BreakIndexCalculationArgs(const FMBSIndexCalculationArgs& InArgs,
	AModularBuildSystemActor*& BuildSystem, int32& Index, FModularSectionInitializer& Initializer,
	TArray<int32>& OccupiedIndices)
{
	BuildSystem = InArgs.BuildSystem;
	Initializer = *InArgs.Initializer;
	Index = InArgs.AtIndex;
	OccupiedIndices = *InArgs.OccupiedIndices;
}

int32 UMBSIndexCalculation::AdjustIndexIfPossible_Implementation(int32& Index, int32 TotalCount,
	const TArray<int32>& OccupiedIndices)
{
	const int32 InitialIndex = Index;
	for (int32 i = 0; i < MaxAdjustIterationCount; i++)
	{
		if (OccupiedIndices.Contains(Index))
		{
			if (Index + 1 < TotalCount)
			{
				Index++;
			}
			else
			{
				Index = 0;
			}
		}
		else
		{
#if WITH_EDITOR
			if (MaxAdjustIterationCount == 0)
			{
				UE_LOG(LogMBSIndexCalculation, VeryVerbose, TEXT("%s: Index=%d is already valid and should not be adjusted."),
					*GetName(), Index);
			}
			else
			{
				UE_LOG(LogMBSIndexCalculation, Verbose, TEXT("%s: Index=%d (Initial=%d) has been adjusted (It=%d)."),
					*GetName(), Index, InitialIndex, i);
			}
#endif			
			return Index;
		}
	}
	UE_LOG(LogMBSIndexCalculation, Error, TEXT("%s: Index=%d (Initial=%d) can't be adjusted."),
		*GetName(), Index, InitialIndex);
	return Index;
}
