// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelBuildSystemGenerator.h"
#include "Tunnel/TunnelBuildSystemActor.h"
#include "List/ModularBuildSystemMeshList.h"
#include "Tunnel/TunnelBasementGeneratorProperty.h"
#include "Tunnel/TunnelEntranceGeneratorProperty.h"
#include "Tunnel/TunnelWallGeneratorProperty.h"
#include "Tunnel/TunnelRoofGeneratorProperty.h"
#include "Tunnel/TunnelRooftopGeneratorProperty.h"

UTunnelBuildSystemGenerator::UTunnelBuildSystemGenerator()
{
	
}

FGeneratedModularSections UTunnelBuildSystemGenerator::Generate_Implementation()
{
	Super::Generate_Implementation();

	if (!PreGenerate(BuildSystemPtr))
	{
		return FGeneratedModularSections();
	}

	// Generate
	//FMBSGeneratorPropertyInitArgs EntranceArgs;
	//EntranceArgs.Generator = this;
	Entrance->CustomInit(this, BuildSystemPtr);

	//LogGenerationSummary();
	FinishGeneration();
	
	return FGeneratedModularSections(BuildSystemPtr.Get());
}

bool UTunnelBuildSystemGenerator::SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystemPtr)
{
	BuildSystemPtr = Cast<ATunnelBuildSystemActor>(InBuildSystemPtr.GetObject());
	return BuildSystemPtr != nullptr;
}

TScriptInterface<IModularBuildSystemInterface> UTunnelBuildSystemGenerator::GetBuildSystemPtr() const
{
	return BuildSystemPtr.Get();
}

bool UTunnelBuildSystemGenerator::CheckLists() const
{
	bool bResult = Super::CheckLists();
	CheckList({
		{ TEXT("TunnelBasementMeshList"), TunnelBasement->Data.MeshList, TunnelBasement->bSupported },
		{ TEXT("TunnelWallsMeshList"), TunnelWall->Data.MeshList, TunnelWall->bSupported },
		{ TEXT("TunnelRoofMeshList"), TunnelRoof->Data.MeshList, TunnelRoof->bSupported },
		{ TEXT("TunnelRooftopMeshList"), TunnelRooftop->Data.MeshList, TunnelRooftop->bSupported },
		{ TEXT("EntranceMeshList"), Entrance->Data.MeshList, Entrance->bSupported }
	}, bResult);
	
	return bResult;
}

void UTunnelBuildSystemGenerator::PrepareBuildSystem() const
{
	const int32 Area = Bounds.X * Bounds.Y;
	const int32 Perimeter = (Bounds.X + Bounds.Y) * 2;
	
	const int32 TotalCount = Area;
	const int32 MaxInRow = Bounds.X;
	
	BuildSystemPtr->TunnelBasement
		.SetName(TEXT("Basement"))
		.SetMeshList(TunnelBasement->Data.MeshList)
		.SetResolution(TunnelBasement->Data.Resolution)
		.SetFromMeshList()
		.SetTotalCount(TotalCount)
		.SetMaxInRow(MaxInRow)
		.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		.SetPivotLocation(GetPivotLocation(TunnelBasement->Data, BuildSystemPtr->TunnelBasement));

	BuildSystemPtr->TunnelWalls.SetNum(LevelCount);
	const int32 WallTotalCount = Bounds.X * 2;
	const int32 WallMaxInRow = Bounds.X;
	int32 WallIndex = 0;
	for (auto& Wall : BuildSystemPtr->TunnelWalls)
	{
		Wall.SetName(*FString::Printf(TEXT("TunnelWall_%d"), WallIndex))
		    .SetMeshList(TunnelWall->Data.MeshList)
		    .SetResolution(TunnelWall->Data.Resolution)
		    .SetFromMeshList()
		    .SetTotalCount(WallTotalCount)
		    .SetMaxInRow(WallMaxInRow)
		    .SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		    .SetPivotLocation(GetPivotLocation(TunnelWall->Data, Wall))
		    .SetSolver(TunnelWall->Solver);
		WallIndex++;
	}
	
	const int32 RoofTotalCount = WallTotalCount;
	const int32 RoofMaxInRow = WallMaxInRow;
	BuildSystemPtr->TunnelRoof
		.SetName(TEXT("Roof"))
		.SetMeshList(TunnelRoof->Data.MeshList)
		.SetResolution(TunnelRoof->Data.Resolution)
		.SetFromMeshList()
		.SetTotalCount(RoofTotalCount)
		.SetMaxInRow(RoofMaxInRow)
		.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		.SetPivotLocation(GetPivotLocation(TunnelRoof->Data, BuildSystemPtr->TunnelRoof))
		.SetSolver(TunnelRoof->Solver);

	const int32 RooftopTotalCount = Bounds.Y > 2 ? (Bounds.Y - 2) * WallMaxInRow : 0;
	const int32 RooftopMaxInRow = WallMaxInRow;
	BuildSystemPtr->TunnelRooftop
		.SetName(TEXT("Rooftop"))
		.SetMeshList(TunnelRooftop->Data.MeshList)
		.SetResolution(TunnelRooftop->Data.Resolution)
		.SetFromMeshList()
		.SetTotalCount(RooftopTotalCount)
		.SetMaxInRow(RooftopMaxInRow)
		.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		.SetSolver(TunnelRooftop->Solver);
}

bool UTunnelBuildSystemGenerator::CanGenerate() const
{
	return TunnelBasement && TunnelWall && TunnelRoof && TunnelRooftop && Entrance && Super::CanGenerate();
}
