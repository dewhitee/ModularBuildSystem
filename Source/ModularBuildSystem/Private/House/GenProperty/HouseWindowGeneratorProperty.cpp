// Fill out your copyright notice in the Description page of Project Settings.


#include "House/GenProperty/HouseWindowGeneratorProperty.h"

#include "MBSIndexCalculation.h"
#include "List/ModularBuildSystemActorList.h"
#include "Engine/StaticMeshActor.h"
#include "House/HouseBuildSystemActor.h"
#include "ModularBuildSystem.h"

bool UHouseWindowGeneratorProperty::Init_Implementation(FMBSGeneratorPropertyInitArgs& Args)
{
	if (!bEnabled)
	{
		return false;
	}

	// generate ids for window sections
	if (WindowCount <= 0)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: WindowCount <= 0. Skipping windows."), *GetName());
		return false;
	}

	if (WindowCountPerLevel <= 0)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: WindowCountPerLevel <= 0. Skipping windows."), *GetName());
		return false;
	}

	if (!CheckEntrancesCount(Args))
	{
		return false;
	}

	Args.LevelIndex = -1;
	//int32 WallLevelIndex = -1;
	//static constexpr int32 MaxWindowCountPerLevel = WindowCountPerLevel;
		
	Args.OutIndices->Reserve(WindowCount);
	for (int32 i = 0; i < WindowCount; i++)
	{
		if (i % WindowCountPerLevel == 0)
		{
			Args.LevelIndex++;
			UE_LOG(LogGenerator, Log, TEXT("%s: i=%d, Args.LevelIndex=%d"), *GetName(), i, Args.LevelIndex);
		}

		Args.AtIndex = i;
		AddNewWindowIndex(Args);
	}

	// check if each id is not occupied by entrance or other unique section
	for (const int32 Index : *Args.InIndices)
	{
		while (Args.OutIndices->Contains(Index))
		{
			UE_LOG(LogGenerator, Error, TEXT("%s: OutWindowIndices.Contains(%d) - incrementing index to fix overlap"),
				*GetName(), Index);
			Args.OutIndices->Remove(Index);
			Args.OutIndices->Add(Index + 1);
		}
	}

	// set mesh to the specified index
	Args.LevelIndex = -1;
	for (int32 i = 0; i < WindowCount; i++)
	{
		if (i % WindowCountPerLevel == 0)
		{
			Args.LevelIndex++;
		}

		Args.AtIndex = i;
		SetSingleWindow(Args);
	}

	return true;
}

bool UHouseWindowGeneratorProperty::IsSet_Implementation() const
{
	return Data.IsSet() && WindowSectionData.IsSet();
}

bool UHouseWindowGeneratorProperty::CheckEntrancesCount(FMBSGeneratorPropertyInitArgs& Args) const
{
	const int32 CountOfEntrances = Args.InIndices->Num();
	UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: InEntranceIndices.Num() = %d"), *GetName(), CountOfEntrances);
	if (CountOfEntrances == 0)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: Can't add %s due to entrances not found."), *GetName(), *Args.InLevelName);
		return false;
	}
	return true;
}

void UHouseWindowGeneratorProperty::AddNewWindowIndex(FMBSGeneratorPropertyInitArgs& Args) const
{
	AHouseBuildSystemActor* HouseBuildSystem = Cast<AHouseBuildSystemActor>(Args.BuildSystem);
	check(HouseBuildSystem);
	
	if (HouseBuildSystem->Walls.IsValidIndex(Args.LevelIndex))
	{
		const FModularSectionInitializer& WallInitializer = HouseBuildSystem->Walls[Args.LevelIndex].GetInitializer();
		const int32 MaxInRow = WallInitializer.GetMaxInRow();
		const int32 TotalCount = WallInitializer.GetTotalCount();
		
		int32 WindowIndex;
		if (Data.IndexCalculation)
		{
			WindowIndex = Data.IndexCalculation->CalculateSingle(
				FMBSIndexCalculationArgs(HouseBuildSystem, Args.AtIndex, Args.OutIndices, &WallInitializer));
			UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: USING IndexCalculation (i=%d) WindowIndex=%d, (Wall) TotalCount=%d"),
				*GetName(), Args.AtIndex, WindowIndex, TotalCount);
		}
		else
		{
			WindowIndex = Args.AtIndex % 2 == 0 ? 0 : TotalCount - MaxInRow + static_cast<bool>(MaxInRow % 2);
			UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: (i=%d) WindowIndex=%d, (Wall) TotalCount=%d"),
				*GetName(), Args.AtIndex, WindowIndex, TotalCount);
			// TODO: 
			constexpr int32 MaxAdjustmentIteration = 10;
			for (int32 i = 0; i < MaxAdjustmentIteration; i++)
			{
				if (Args.OutIndices->Contains(WindowIndex))
				{
					if (WindowIndex + 1 < WallInitializer.GetAdjustedTotalCount())
					{
						WindowIndex++;
					}
					else
					{
						WindowIndex = 0;
					}
				}
				else
				{
					UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: WindowIndex after adjustment = %d"), *GetName(), WindowIndex);
					break;
				}
			}
		}
		
		Args.OutIndices->Add(WindowIndex);
	}
	else
	{
		UE_LOG(LogGenerator, Error, TEXT("%s: Invalid WallLevelIndex (%d). Max possible = %d"),
			*GetName(), Args.LevelIndex, HouseBuildSystem->Walls.Num());
	}
}

void UHouseWindowGeneratorProperty::SetSingleWindow(FMBSGeneratorPropertyInitArgs& Args) const
{
	AHouseBuildSystemActor* HouseBuildSystem = Cast<AHouseBuildSystemActor>(Args.BuildSystem);
	check(HouseBuildSystem);
	
	if (HouseBuildSystem->Walls.IsValidIndex(Args.LevelIndex) && Args.OutIndices->IsValidIndex(Args.AtIndex))
	{
		UStaticMesh* WindowSectionMesh = WindowSectionData.GetMesh(0);
		FModularLevel& WindowLevel = HouseBuildSystem->Walls[Args.LevelIndex];
		WindowLevel.SetUpdated(true);
		const int32 InstanceIndex = (*Args.OutIndices)[Args.AtIndex];

		if (HouseBuildSystem->IsOfInstancedMeshConfigurationType())
		{
			FTransform ReplacedInstanceTransform;
			HouseBuildSystem->ReplaceWithNonInstancedSection(WindowSectionMesh, InstanceIndex, WindowLevel.GetId(),
				WindowLevel.InstancedStaticMeshComponent, ReplacedInstanceTransform);
			if (const TSubclassOf<AActor> ActorClass = Data.GetActorClass(0, Data.Resolution))
			{
				UE_LOG(LogGenerator, Verbose, TEXT("%s: Initializing window actor at the window modular section found at InstanceIndex = %d."), 
					*GetName(), InstanceIndex);

				UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: WindowTransform = %s"), *GetName(),
					*ReplacedInstanceTransform.ToHumanReadableString());
				HouseBuildSystem->InitModularSectionActor(ReplacedInstanceTransform, WindowLevel.GetId(), ActorClass,
					true, true);
			}
		}
		else
		{
			HouseBuildSystem->SetMeshAt(WindowLevel, InstanceIndex, 0, WindowSectionMesh);

			if (const TSubclassOf<AActor> ActorClass = Data.GetActorClass(0, Data.Resolution))
			{
				const FModularSection* WindowSection = HouseBuildSystem->GetSectionAt(WindowLevel, InstanceIndex);

				UE_LOG(LogGenerator, Verbose, TEXT("%s: Initializing window actor at the %s window modular section."), 
					*GetName(), *WindowSection->GetName());

				const FTransform& WindowTransform = WindowSection->GetTransform();
				UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: WindowTransform = %s"), *GetName(),
					*WindowTransform.ToHumanReadableString());
				HouseBuildSystem->InitModularSectionActor(WindowTransform, WindowLevel.GetId(), ActorClass,
					true, false);
			}
		}
	}
	else
	{
		FString OutIndicesStr = "[";
		for (const int32 Index : *Args.OutIndices)
		{
			OutIndicesStr += FString::FromInt(Index) + ",";
		}
		OutIndicesStr += "]";
		UE_LOG(LogGenerator, Error, TEXT("%s: WallLevelIndex was %d while max possible wall (level) index is = %d; Args.AtIndex=%d, Args.OutIndices.Num()=%d, Args.OutIndices=%s"), 
			*GetName(), Args.LevelIndex, HouseBuildSystem->Walls.Num() - 1, Args.AtIndex, Args.OutIndices->Num(), *OutIndicesStr);
	}
}
