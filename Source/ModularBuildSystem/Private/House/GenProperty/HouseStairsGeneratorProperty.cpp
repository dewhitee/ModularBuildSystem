// Fill out your copyright notice in the Description page of Project Settings.


#include "House/GenProperty/HouseStairsGeneratorProperty.h"

#include "Engine/StaticMeshActor.h"
#include "House/HouseBuildSystemActor.h"
#include "ModularBuildSystem.h"

bool UHouseStairsGeneratorProperty::Init_Implementation(FMBSGeneratorPropertyInitArgs& Args)
{
	if (!Args.InIndices || Args.InIndices->IsEmpty())
	{
		return false;
	}

	AHouseBuildSystemActor* HBS = Cast<AHouseBuildSystemActor>(Args.BuildSystem);
	check(HBS);

	UStaticMesh* StairsMesh = Data.GetMesh(0);
	const FModularLevel& WallLevel = HBS->Walls[0];

	if (HBS->IsOfInstancedMeshConfigurationType())
	{
		for (const auto& Transform : *Args.InTransforms)
		{
			UE_LOG(LogGenerator, Verbose, TEXT("%s: Initializing stairs at the %s transform."),
				*GetName(), *Transform.ToHumanReadableString());
			HBS->InitModularSection(StairsMesh, Transform, HBS->Basement.GetId(), true, true);
		}
	}
	else
	{
		for (const int32 EntranceIndex : *Args.InIndices)
		{
			// TODO: implement logic for instanced configuration type. Currently spawn static mesh actors instead, as before.
			if (const FModularSection* EntranceSection = HBS->GetSectionAt(WallLevel, EntranceIndex))
			{
				UE_LOG(LogGenerator, Verbose, TEXT("%s: Initializing stairs at the %s entrance modular section."),
					*GetName(), *EntranceSection->GetStaticMeshActor()->GetName());

				const FTransform& EntranceTransform = EntranceSection->GetStaticMeshActor()->GetActorTransform();
				UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: EntranceTransform = %s"), *GetName(),
					*EntranceTransform.ToHumanReadableString());
					
				HBS->InitModularSection(StairsMesh, EntranceTransform, HBS->Basement.GetId(), true, false);
			}
			else
			{
				UE_LOG(LogGenerator, Error, TEXT("%s: Entrance not found at EntranceIndex = %d! EntranceSection was nullptr."),
					*GetName(), EntranceIndex);
				return false;
			}
		}
	}
	
	return true;
}
