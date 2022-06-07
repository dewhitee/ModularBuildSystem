// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelEntranceGeneratorProperty.h"
#include "ModularBuildSystem.h"
#include "Solver/MBSTransformSolver.h"
#include "Tunnel/TunnelBuildSystemActor.h"

bool UTunnelEntranceGeneratorProperty::CustomInit(UTunnelBuildSystemGenerator* Generator, ATunnelBuildSystemActor* BS)
{
	check(Generator);
	check(BS);
	if (!EntranceRooftopSolver)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: EntranceRooftopSolver == nullptr"), *GetName());
		return false;
	}
	if (!EntranceWallSolver)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: EntranceWallSolver == nullptr"), *GetName());
		return false;
	}
	
	const bool bInstanced = BS->IsOfInstancedMeshConfigurationType();
	constexpr bool bWorldSpace = false;
	const FVector DefaultSectionSize = FVector(UModularSectionResolution::DefaultSectionSize, 0.f, 0.f);
	//const FTransform& BSTransform = BS->GetActorTransform();

	// Init walls (corners)
	for (const auto& Wall : BS->TunnelWalls)
	{
		const int32 WallMaxInRow = Wall.GetInitializer().GetMaxInRow();
		const int32 WallTotalCount = Wall.GetInitializer().GetTotalCount();

		TArray<FTransform> WallTransforms = BS->GetSectionTransformAt(
			Wall, {0, WallMaxInRow, WallMaxInRow - 1, WallTotalCount - 1}, bInstanced, bWorldSpace);

		WallTransforms[0].AddToTranslation(-DefaultSectionSize);
		WallTransforms[1].MultiplyScale3D(FVector(-1.f, 1.f, 1.f));
		WallTransforms[2].MultiplyScale3D(FVector(-1.f, 1.f, 1.f));
		WallTransforms[3].AddToTranslation(DefaultSectionSize);
		
		BS->InitMultipleModularSections(
			Data.GetRandomMesh(), WallTransforms, Wall.GetId(), true, !bWorldSpace);
	}
	
	/*for (const auto& Wall : BS->TunnelWalls)
	{
		for (int32 i = 0; i < Wall.Initializer.TotalCount; i++)
		{
			FNextTransformArgs EntranceWallArgs = FNextTransformArgs(
				BSTransform, i, Wall, BS->BuildStats, nullptr);

			FTransform NewTransform = EntranceWallSolver->GetNextTransform(EntranceWallArgs);
			if (EntranceWallArgs.bOutShouldBeSkipped)
			{
				continue;
			}
			BS->InitModularSection(
				Data.GetRandomMesh(), NewTransform, Wall.Id, true, false);
		}
	}*/

	// Init roofs
	const FModularLevel& Roof = BS->TunnelRoof;
	const int32 RoofMaxInRow = Roof.GetInitializer().GetMaxInRow();
	const int32 RoofTotalCount = Roof.GetInitializer().GetTotalCount();

	TArray<FTransform> RoofTransforms = BS->GetSectionTransformAt(
		Roof, {0, RoofMaxInRow, RoofMaxInRow - 1, RoofTotalCount - 1}, bInstanced, bWorldSpace);

	RoofTransforms[0].AddToTranslation(-DefaultSectionSize);
	RoofTransforms[1].MultiplyScale3D(FVector(-1.f, 1.f, 1.f));
	RoofTransforms[2].MultiplyScale3D(FVector(-1.f, 1.f, 1.f));
	RoofTransforms[3].AddToTranslation(DefaultSectionSize);
	
	BS->InitMultipleModularSections(
		EntranceRoof.GetRandomMesh(), RoofTransforms, Roof.GetId(), true, !bWorldSpace);

	// Init rooftops
	const FModularLevel& Rooftop = BS->TunnelRooftop;
	const int32 RooftopTotalCount = Rooftop.GetInitializer().GetTotalCount();
	for (int32 i = 0; i < RooftopTotalCount; i++)
	{
		FNextTransformArgs EntranceRooftopArgs = FNextTransformArgs(
			BS->GetActorTransform(), i, Rooftop, BS->BuildStats, nullptr, BS->Spline);
		
		FTransform NewTransform = EntranceRooftopSolver->GetNextTransform(EntranceRooftopArgs);
		if (EntranceRooftopArgs.bOutShouldBeSkipped)
		{
			continue;
		}
		BS->InitModularSection(
			EntranceRooftop.GetRandomMesh(), NewTransform, Rooftop.GetId(), true, false);
	}

	return true;
}
