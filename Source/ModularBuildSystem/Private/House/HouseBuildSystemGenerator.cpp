// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HouseBuildSystemGenerator.h"

#include "House/GenProperty/HouseWindowGeneratorProperty.h"
#include "House/GenProperty/HouseBasementGeneratorProperty.h"
#include "House/GenProperty/HouseChimneyGeneratorProperty.h"
#include "House/GenProperty/HouseCornerGeneratorProperty.h"
#include "House/GenProperty/HouseDoorGeneratorProperty.h"
#include "House/GenProperty/HouseEntranceGeneratorProperty.h"
#include "House/GenProperty/HouseFloorGeneratorProperty.h"
#include "House/GenProperty/HouseRoofGeneratorProperty.h"
#include "House/GenProperty/HouseRooftopGeneratorProperty.h"
#include "House/GenProperty/HouseStairsGeneratorProperty.h"
#include "House/GenProperty/HouseVegetationGeneratorProperty.h"
#include "House/GenProperty/HouseWallGeneratorProperty.h"
#include "House/HouseBuildSystemActor.h"
#include "MBSFunctionLibrary.h"
#include "MBSGeneratorProperty.h"
#include "MBSIndexCalculation.h"
#include "List/ModularBuildSystemMeshList.h"
#include "List/ModularBuildSystemActorList.h"
#include "ModularBuildSystem.h"
#include "SectionBuilder.h"
#include "Timer.h"
#include "Shape/ModularLevelShape.h"

#include "Engine/StaticMeshActor.h"

UHouseBuildSystemGenerator::UHouseBuildSystemGenerator()
{
	bCanHaveEntrances = true;
	bCanHaveDoors = true;
	bCanHaveStairs = true;
	bCanHaveFloors = false;
	bCanHaveCorners = true;
}

FGeneratedModularSections UHouseBuildSystemGenerator::Generate_Implementation()
{
	TIMER_THIS
	Super::Generate_Implementation();

	//// TODO: Clear MBS actor when Generator changes. Currently overridden PivotLocations of modular levels are not reset - but should be.
	if (!PreGenerate(BuildSystemPtr))
	{
		TIMER_SHOW
		return FGeneratedModularSections();
	}

	// Offset floors
	// TODO: implement for loop for Floors and offset them if
	for (auto& Floor : BuildSystemPtr->Floors)
	{
		BuildSystemPtr->OffsetModularLevel(Floor, FVector(0.f, 0.f, Floor_New->FloorZOffset));
	}

	BuildSystemPtr->OffsetModularLevel(BuildSystemPtr->Roof, /*RoofOffset*/Roof_New->RoofOffset);

	// - Update meshes and/or add new sections
	// Entrances
	TArray<int32> EntranceIndices;
	TArray<FTransform> EntranceTransforms;
	SetEntrances(EntranceIndices, EntranceTransforms);

	// Doors
	SetDoors(EntranceIndices, EntranceTransforms);
	 
	// Stairs
	SetStairs(EntranceIndices, EntranceTransforms);
	
	// Windows
	TArray<int32> WindowIndices;
	FMBSGeneratorPropertyInitArgs WindowsArgs;
	WindowsArgs.Generator = this;
	WindowsArgs.InIndices = &EntranceIndices;
	WindowsArgs.OutIndices = &WindowIndices;
	WindowsArgs.InLevelName = "windows";
	WindowsArgs.BuildSystem = BuildSystemPtr;
	Windows_New->Init(WindowsArgs);

	// TODO: Move to the subclasses?
	SetChimney();
	SetRoofWindows();
	SetInterWallCorners();
	SetVegetation();

	SetFloorHoles(Floor_New->FloorHoleIdTransforms);
	SetFloorHoleDoor(Floor_New->FloorHoleIdTransforms);

	//LogGenerationSummary();
	FinishGeneration();

	TIMER_SHOW
	return FGeneratedModularSections(BuildSystemPtr.Get());
}

bool UHouseBuildSystemGenerator::CheckBounds() const
{
	Super::CheckBounds();

	if (Bounds.X <= 0 || Bounds.Y <= 0)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: Nor Bounds.X neither Bounds.Y can be equal or less than zero."),
			*GetName());
		return false;
	}
	UE_LOG(LogGenerator, Verbose, TEXT("%s: Bounds are valid."), *GetName());
	return true;
}

bool UHouseBuildSystemGenerator::CheckLists() const
{
	bool bResult = Super::CheckLists();

	// New
	// TODO: Deprecate and use CheckProperties only
	// TODO: Check MeshList or ActorList depending on bUseActorList property
	CheckList({
		{ TEXT("BasementMeshList"), Basement_New->Data.MeshList, true },
		{ TEXT("WallMeshList"), Walls_New->Data.MeshList, bCanHaveWalls },
		{ TEXT("EntranceMeshList"), Entrances_New->Data.MeshList, bCanHaveEntrances },
		{ TEXT("DoorActorList"), Doors_New->Data.ActorList, Doors_New->bEnabled },
		{ TEXT("StairsMeshList"), Stairs_New->Data.MeshList, Stairs_New->bEnabled },
		{ TEXT("FloorMeshList"), Floor_New->Data.MeshList, bCanHaveFloors },
		{ TEXT("CornerMeshList"), Corners_New2->Data.MeshList, Corners_New2->bEnabled },
		{ TEXT("RoofMeshList"), Roof_New->Data.MeshList, Roof_New->bSupported },
		{ TEXT("RooftopMeshList"), Rooftop_New->Data.MeshList, bCanHaveRooftop },
		{ TEXT("ChimneyMeshList"), Chimney_New->Data.MeshList, Chimney_New->bEnabled },
		{ TEXT("RoofWindowsMeshList"), RoofWindows.Data.MeshList, RoofWindows.bSupported && !RoofWindows.Data.bUseActorList },
		{ TEXT("RoofWindowsActorList"), RoofWindows.Data.ActorList, RoofWindows.bSupported && RoofWindows.Data.bUseActorList },
		{ TEXT("InterWallCornersMeshList"), InterWallCorners.Data.MeshList, InterWallCorners.bSupported },
		{ TEXT("FloorHolesMeshList"), Floor_New->FloorHoles.MeshList, Floor_New->bSupported }
	}, bResult);
	//
	return bResult;
}

bool UHouseBuildSystemGenerator::CheckResolutions() const
{
	bool bResult = Super::CheckResolutions();
	// TODO: Deprecate and use CheckProperties only
	CheckResolution({
		{ TEXT("BasementResolution"), Basement_New->Data.Resolution, true },
		{ TEXT("WallResolution"), Walls_New->Data.Resolution, bCanHaveWalls },
		{ TEXT("StairsResolution"), Stairs_New->Data.Resolution, Stairs_New->bSupported },
		{ TEXT("FloorResolution"), Floor_New->Data.Resolution, bCanHaveFloors },
		{ TEXT("CornerResolution"), Corners_New2->Data.Resolution, Corners_New2->bEnabled },
		{ TEXT("RoofResolution"), Roof_New->Data.Resolution, Roof_New->bSupported },
		{ TEXT("RooftopResolution"), Rooftop_New->Data.Resolution, bCanHaveRooftop },
		{ TEXT("ChimneyResolution"), Chimney_New->Data.Resolution, Chimney_New->bEnabled },
		{ TEXT("RoofWindowsResolution"), RoofWindows.Data.Resolution, RoofWindows.bSupported },
		{ TEXT("InterWallCornersResolution"), InterWallCorners.Data.Resolution, InterWallCorners.bSupported },
		{ TEXT("FloorHolesResolution"), Floor_New->FloorHoles.Resolution, Floor_New->bCanHaveFloorHoles }
	}, bResult);
	//
	
	return bResult;
}

void UHouseBuildSystemGenerator::PreGenerateClear() const
{
	// TODO: Handle smart execution mode
	UE_LOG(LogGenerator, Log, TEXT("%s: PreGenerateClear"), *GetName());
	if (BuildSystemPtr->GetSpawnConfiguration().ExecutionMode == EMBSExecutionMode::Smart)
	{
		// TODO: Check other properties if they were modified
		// Only reset sections if bounds were updated.
		const FMBSBounds& TransformBounds = BuildSystemPtr->GetTransformBounds();
		
		UE_LOG(LogGenerator, Verbose, TEXT("%s: UHouseBuildSystemGenerator: MBS->GetTransformBounds().WasUpdated() = %d"),
			*GetName(), static_cast<int32>(TransformBounds.WasUpdated()));
		if (TransformBounds.WasUpdated())
		{
			if (CustomLevelShape)
			{
				BuildSystemPtr->ResetBuildSystem();
				return;
			}
			
			// TODO: Reset only those sections that were modified
			// Update: Reset only in case when building bounds are less that they were before.
			if (TransformBounds.IsCurrentZLessThanPrevious())
			{
				UE_LOG(LogGenerator, Verbose, TEXT("%s: TransformBounds.IsCurrentZLessThanPrevious"), *GetName());
				// TODO: Implement and Refactor. High possibility that this don't belong here and should be handled
				// in the HouseBuildSystemActor class instead (before calling Generator->Generate()).
				const int32 CountToRemove = FMath::Abs(TransformBounds.GetDifferenceZ());
				UE_LOG(LogGenerator, Verbose, TEXT("%s: CountToRemove = %d"), *GetName(), CountToRemove);
				for (int32 i = 0; i < CountToRemove; i++)
				{
					BuildSystemPtr->RemoveSectionsOfLevel(BuildSystemPtr->Walls.Last().GetId());
					BuildSystemPtr->Walls.RemoveAt(BuildSystemPtr->Walls.Num() - 1);

					BuildSystemPtr->RemoveSectionsOfLevel(BuildSystemPtr->Corners.Last().GetId());
					BuildSystemPtr->Corners.RemoveAt(BuildSystemPtr->Corners.Num() - 1);
				}
				BuildSystemPtr->ResetBuildSystem(false, true, false, false);
			}
			else if (TransformBounds.IsCurrentZGreaterThanPrevious())
			{
				UE_LOG(LogGenerator, Verbose, TEXT("%s: TransformBounds.IsCurrentZGreaterThanPrevious"), *GetName());
				BuildSystemPtr->ResetBuildSystem(false, true, false, false);
				// Doing nothing in this case. Initialization will be run only for new modular levels
			}
			else
			{
				UE_LOG(LogGenerator, Verbose, TEXT("%s: Current Z is the same as before... Resetting all"), *GetName());
				BuildSystemPtr->ResetBuildSystem();
			}
		}
		else
		{
			// Clear all in case we have a custom level shape for now.
			// TODO: Refactor
			if (CustomLevelShape)
			{
				BuildSystemPtr->ResetSingleLevel(BuildSystemPtr->Basement.GetId());
				BuildSystemPtr->ResetSingleLevel(BuildSystemPtr->Rooftop.GetId());
			}
			
			if (BuildSystemPtr->IsOfInstancedMeshConfigurationType())
			{
				BuildSystemPtr->ResetBuildSystem();
			}
			else
			{
				UE_LOG(LogGenerator, Log, TEXT("%s: Reset only actor sections"), *GetName());
				BuildSystemPtr->ResetBuildSystem(false, true, false, false);
			}
		}
	}
	else
	{
		// Clearing
		UE_LOG(LogGenerator, Log, TEXT("%s: ResetAll"), *GetName());
		BuildSystemPtr->ResetBuildSystem();
	}
}

void UHouseBuildSystemGenerator::PrepareBuildSystem() const
{
	// - Generate levels
	const int32 Area = Bounds.X * Bounds.Y;
	const int32 Perimeter = (Bounds.X + Bounds.Y) * 2;

	int32 BasementTotalCount, BasementMaxInRow;
	if (Basement_New->Data.Resolution->GetSnapMode() == EModularSectionResolutionSnapMode::Wall)
	{
		BasementTotalCount = Perimeter;
		BasementMaxInRow = Bounds.X + 2;
	}
	else
	{
		BasementTotalCount = Area;
		BasementMaxInRow = Bounds.X;
	}
	
	// Set basement
	BuildSystemPtr->Basement
		.SetMeshList(Basement_New->Data.MeshList)
		.SetMeshListIndex(Basement_New->MeshIndex)
		.SetResolution(Basement_New->Data.Resolution)
		.SetTotalCount(BasementTotalCount)
		.SetMaxInRow(BasementMaxInRow)
		.SetFromMeshList()
		.SetShape(CustomLevelShape)
		.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		.SetPivotLocation(GetPivotLocation(Basement_New->Data, BuildSystemPtr->Basement))
		.SetName(TEXT("Basement"));

	// Set walls
	const int32 WallTotalCount = Perimeter;
	const int32 WallMaxInRow = Bounds.X + 2;

	BuildSystemPtr->Walls.SetNum(LevelCount);
	int32 WallIndex = 0;
	for (auto& Wall : BuildSystemPtr->Walls)
	{
		Wall.SetMeshList(Walls_New->Data.MeshList)
		    .SetMeshListIndex(Walls_New->MeshIndex)
		    .SetResolution(Walls_New->Data.Resolution)
		    .SetTotalCount(WallTotalCount)
		    .SetMaxInRow(WallMaxInRow)
		    .SetFromMeshList()
		    .SetShape(CustomLevelShape)
		    .SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
		    .SetPivotLocation(GetPivotLocation(Walls_New->Data, Wall))
		    .SetName(*FString::Printf(TEXT("Wall_%d"), WallIndex));
		WallIndex++;
	}

	// Set floors
	if (Floor_New && Floor_New->bEnabled)
	{
		int32 Num = Floor_New->bFloorForEachLevel ? LevelCount : Floor_New->MaxFloorCount;
		// TODO: Put member property from Attic gen property here
		if (bCanHaveAttic)
		{
			Num++;
		}
		
		BuildSystemPtr->Floors.SetNum(Num);
		int32 FloorIndex = 0;
		for (auto& Floor : BuildSystemPtr->Floors)
		{
			const int32 MeshListIndex = Floor_New->bUniqueFirstFloor && FloorIndex == 0 ? Floor_New->FirstFloorMeshIndex : Floor_New->OtherFloorMeshIndex;
			Floor.SetMeshList(Floor_New->Data.MeshList)
				.SetMeshListIndex(Floor_New->MeshIndex)
				.SetResolution(Floor_New->Data.Resolution)
				.SetTotalCount(Area)
				.SetMaxInRow(Bounds.X)
				.SetMeshListIndex(MeshListIndex)
				.SetFromMeshList()
				.SetShape(CustomLevelShape)
				.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
				.SetPivotLocation(GetPivotLocation(Floor_New->Data, Floor))
				.SetName(*FString::Printf(TEXT("Floor_%d"), FloorIndex))
				.SetAffectsNextZMultiplier(bCanHaveAttic ? !(FloorIndex == Num - 1) : true);
			FloorIndex++;
		}
		// TODO: Check if floors are cleared properly
		// Currently there is a bug where floors are not clearing on reset button of a build system actor
	}
	else
	{
		BuildSystemPtr->Floors.Empty();
	}

	// Set corners
	if (Corners_New2 && Corners_New2->bEnabled)
	{
		BuildSystemPtr->Corners.SetNum(LevelCount + 1);
		int32 CornerIndex = 0;
		for (auto& Corner : BuildSystemPtr->Corners)
		{
			Corner.SetMeshList(Corners_New2->Data.MeshList)
				.SetMeshListIndex(Corners_New2->MeshIndex)
				.SetResolution(Corners_New2->Data.Resolution)
				.SetTotalCount(4)
				.SetMaxInRow(2)
				.SetFromMeshList()
				.SetShape(CustomLevelShape)
				.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
				.SetPivotLocation(GetPivotLocation(Corners_New2->Data, Corner))
				.SetName(*FString::Printf(TEXT("Corner_%d"), CornerIndex));
			CornerIndex++;
		}
	}
	else
	{
		BuildSystemPtr->Corners.Empty();
	}

	// Set roof
	if (Roof_New && Roof_New->bEnabled)
	{
		BuildSystemPtr->Roof
			.SetMeshList(Roof_New->Data.MeshList)
			.SetMeshListIndex(Roof_New->MeshIndex)
			.SetResolution(Roof_New->Data.Resolution)
			.SetTotalCount(WallTotalCount)
			.SetMaxInRow(WallMaxInRow)
			.SetFromMeshList()
			.SetShape(CustomLevelShape)
			.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
			.SetPivotLocation(GetPivotLocation(Roof_New->Data, BuildSystemPtr->Roof))
			.SetName(TEXT("Roof"));
	}

	// Set rooftop
	if (Rooftop_New && Rooftop_New->bEnabled)
	{
		const int32 RooftopTotalCount = (Bounds.X - 1) * (Bounds.Y - 1);
		const int32 RooftopMaxInRow = Bounds.X - 1;

		BuildSystemPtr->Rooftop
			.SetMeshList(Rooftop_New->Data.MeshList)
			.SetMeshListIndex(Rooftop_New->MeshIndex)
			.SetResolution(Rooftop_New->Data.Resolution)
			.SetTotalCount(RooftopTotalCount)
			.SetMaxInRow(RooftopMaxInRow)
			.SetFromMeshList()
			.SetShape(CustomLevelShape)
			.SetInstancedStaticMeshComponent(nullptr, BuildSystemPtr.Get(), true)
			.SetPivotLocation(GetPivotLocation(Rooftop_New->Data, BuildSystemPtr->Rooftop))
			.SetName(TEXT("Rooftop"));
	}
	else // Invalidate rooftop
	{
		BuildSystemPtr->Rooftop.Invalidate();
	}
}

void UHouseBuildSystemGenerator::SetDoors(const TArray<int32>& InEntranceIndices,
	const TArray<FTransform>& InEntranceTransforms) const
{
	if (/*bWithDoors*/!Doors_New || !Doors_New->bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("doors"));
	if (!CheckEntrancesCount(TEXT("doors"), InEntranceIndices))
	{
		return;
	}

	MBS::FSectionBuilder(BuildSystemPtr, &BuildSystemPtr->Walls[0])
		.Actor(Doors_New->Data.GetActorClass(0, Walls_New->Data.Resolution))
		.At(InEntranceTransforms)
		.Spawn(true);

	/*const FModularLevel& WallLevel = BuildSystemPtr->Walls[0];
	const TSubclassOf<AActor> ActorClass = Doors_New->Data.GetActorClass(0, Walls_New->Data.Resolution);
	if (ActorClass)
	{
		for (const FTransform& EntranceTransform : InEntranceTransforms)
		{
			UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: EntranceTransform = %s"), *GetName(), *EntranceTransform.ToHumanReadableString());
			BuildSystemPtr->InitModularSectionActor(EntranceTransform, WallLevel.Id, ActorClass, true, true);
		}
	}*/
}

void UHouseBuildSystemGenerator::SetEntrances(TArray<int32>& OutEntranceIndices,
	TArray<FTransform>& OutEntranceTransforms) const
{
	if (!Entrances_New || !Entrances_New->bEnabled || BuildSystemPtr->Walls.IsEmpty())
	{
		return;
	}
	
	PrintHeader(TEXT("entrances"));
	UE_LOG(LogGenerator, Verbose, TEXT("%s: Setting entrances"), *GetName());

	const FModularLevel& WallLevel = BuildSystemPtr->Walls[0];

	//const int32 CentralElement = Entrances_New->bEntranceFromFront ? 0 : Bounds.X / 2;
	//const int32 CentralRow = Entrances_New->bEntranceFromFront ? Bounds.Y / 2 - 1 : 0;

	const int32 WallsMaxInRow = WallLevel.GetInitializer().GetMaxInRow();
	int32 ElementIndex = WallsMaxInRow + (WallsMaxInRow % 2);

	// TODO: If MBS actor uses Shape that swaps front and back indices - decrement index
	// TODO: Also, add new virtual function for MBS actor or Shape objects that will return if Front is Odd or Even index
	if (CustomLevelShape && CustomLevelShape->HasFrontIndicesInverted() && CustomLevelShape->SkipsSections())
	{
		ElementIndex--;
	}

	//UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: CentralElement=%d, CentralRow=%d, ElementIndex=%d"), *GetName(),
	//	CentralElement, CentralRow, ElementIndex);
	UStaticMesh* EntranceMesh = Entrances_New->Data.MeshList->GetMesh(0, Walls_New->Data.Resolution);

	// Check if index is valid
	const int32 WallSectionCount = BuildSystemPtr->GetSectionsOfLevel(WallLevel).Num();
	for (int32 i = 0; i < WallSectionCount; i++)
	{
		if (!BuildSystemPtr->IsValidSectionIndex(WallLevel, ElementIndex))
		{
			if (ElementIndex >= WallSectionCount - 1)
			{
				ElementIndex = 0;
			}
			else
			{
				ElementIndex++;
			}
			UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: ElementIndex=%d, WallSectionCount=%d"), *GetName(),
				ElementIndex, WallSectionCount);
		}
		else
		{
			break;
		}
	}
	
	MBS::FSectionBuilder Builder(BuildSystemPtr, &WallLevel);
	Builder.Mesh(EntranceMesh)
		.At(ElementIndex)
		.ReplaceIfInstancedOrSet();
	OutEntranceTransforms.Append(Builder.GetTransforms());
	
	/*if (BuildSystemPtr->IsOfInstancedMeshConfigurationType())
	{
		// If configuration is set to instanced - then we need to remove one instance at the index and replace it with single modular section
		FTransform ReplacedInstanceTransform;
		BuildSystemPtr->ReplaceWithNonInstancedSection(EntranceMesh, ElementIndex, WallLevel.Id,
			WallLevel.InstancedStaticMeshComponent, ReplacedInstanceTransform);
		OutEntranceTransforms.Add(ReplacedInstanceTransform);
	}
	else
	{
		BuildSystemPtr->SetMeshAt(BuildSystemPtr->Walls[0], /*CentralElement * #1#ElementIndex, /*CentralRow#1#0, EntranceMesh);
		// TODO: Test new
		OutEntranceTransforms.Add(BuildSystemPtr->GetSectionTransformAt(WallLevel.Id, ElementIndex, false, false));
	}*/

	OutEntranceIndices.Add(ElementIndex);
}

void UHouseBuildSystemGenerator::SetStairs(const TArray<int32>& InEntranceIndices,
	const TArray<FTransform>& InEntranceTransforms) const
{
	if (!Stairs_New || !Stairs_New->bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("stairs"));
	if (!CheckEntrancesCount(TEXT("stairs"), InEntranceIndices))
	{
		return;
	}

	UStaticMesh* StairsMesh = Stairs_New->Data.GetMesh(0);
	//const FModularLevel& WallLevel = BuildSystemPtr->Walls[0];

	for (const auto& Transform : InEntranceTransforms)
	{
		UE_LOG(LogGenerator, Verbose, TEXT("%s: Initializing stairs at the %s transform."), *GetName(), *Transform.ToHumanReadableString());
		BuildSystemPtr->InitModularSection(StairsMesh, Transform, BuildSystemPtr->Basement.GetId(), true, true);
	}
}

void UHouseBuildSystemGenerator::SetChimney() const
{
	if (!Chimney_New || !Chimney_New->bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("chimneys"));
		
	const int32 LevelId = BuildSystemPtr->Roof.GetId();	
	FTransform CalculatedTransform = BuildSystemPtr->GetSectionTransformAtRandom(
		LevelId,
		BuildSystemPtr->IsOfInstancedMeshConfigurationType(),
		false);

	CalculatedTransform.AddToTranslation(Chimney_New->ChimneyOffset);

	UMBSFunctionLibrary::InitSectionFromProperty(BuildSystemPtr, Chimney_New, Chimney_New->bRandomChimneyIndex,
		Chimney_New->ChimneyIndex, CalculatedTransform, LevelId, true, true);
}

void UHouseBuildSystemGenerator::SetVegetation() const
{
	if (!Vegetation_New || !Vegetation_New->bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("vegetation"));
	for (auto& Wall : BuildSystemPtr->Walls)
	{
		TArray<int32> OccupiedIndices;
		for (int32 i = 0; i < Vegetation_New->VegetationCount; i++)
		{
			int32 Index;
			if (Vegetation_New->Data.IndexCalculation)
			{
				Index = Vegetation_New->Data.IndexCalculation->CalculateSingle(
					{ BuildSystemPtr, i, &OccupiedIndices, &Wall.GetInitializer() });
			}
			else
			{
				Index = 0;
			}
			FTransform CalculatedTransform = BuildSystemPtr->GetSectionTransformAt(
				Wall, Index, BuildSystemPtr->IsOfInstancedMeshConfigurationType(), false);

			OccupiedIndices.Add(Index);

			MBS::FSectionBuilder(BuildSystemPtr, &Wall)
				.Mesh(Vegetation_New->Data.GetRandomMesh())
				.At(CalculatedTransform)
				.MoveBy(Vegetation_New->VegetationOffset)
				.RotateYaw(90.f)
				.Spawn(true);
			/*// Adjust location and rotation
			CalculatedTransform.AddToTranslation(Vegetation_New->VegetationOffset);
				
			const FRotator Rotation = FRotator(CalculatedTransform.GetRotation());
			CalculatedTransform.SetRotation(
				static_cast<const FQuat>(FRotator(Rotation.Pitch, Rotation.Yaw + 90.f, Rotation.Roll)));
				
			BuildSystemPtr->InitModularSection(
				Vegetation_New->Data.GetRandomMesh(), CalculatedTransform, Wall.Id, true, true);*/
		}
	}
}

void UHouseBuildSystemGenerator::SetRoofWindows() const
{
	if (!RoofWindows.bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("roof windows"));

	constexpr int32 Count = 1;
	TArray<int32> OccupiedIndices;
	for (int32 i = 0; i < Count; i++)
	{
		int32 Index;
		if (RoofWindows.Data.IndexCalculation)
		{
			Index = RoofWindows.Data.IndexCalculation->CalculateSingle(
				{ BuildSystemPtr, i, &OccupiedIndices, &BuildSystemPtr->Roof.GetInitializer() });
		}
		else
		{
			Index = 0;
		}

		MBS::FSectionBuilder(BuildSystemPtr, &BuildSystemPtr->Roof)
			.Actor(RoofWindows.Data.GetRandomActorClass())
			.AtInstanced(Index, BuildSystemPtr->IsOfInstancedMeshConfigurationType())
			.RotateYaw(-90.f)
			.MoveBy(FVector(-100.f, 0.f, -350.f))
			.Spawn(true);
		
		/*FTransform CalculatedTransform = BuildSystemPtr->GetSectionTransformAt(
			BuildSystemPtr->Roof, Index, BuildSystemPtr->IsOfInstancedMeshConfigurationType(), false);

		const FRotator Rotation = FRotator(CalculatedTransform.GetRotation());
		CalculatedTransform.SetRotation(
			static_cast<const FQuat>(FRotator(Rotation.Pitch, Rotation.Yaw - 90.f, Rotation.Roll)));

		// TODO: Replace literals with parameters
		CalculatedTransform.AddToTranslation(FVector(-100.f, 0.f, -350.f));
			
		BuildSystemPtr->InitModularSectionActor(
			CalculatedTransform,
			BuildSystemPtr->Roof.Id,
			RoofWindows.Data.GetRandomActorClass(),
			true,
			true);*/
	}
}

void UHouseBuildSystemGenerator::SetInterWallCorners() const
{
	// TODO: Find the cause of possible EXCEPTION_ACCESS_VIOLATION here
	if (InterWallCorners.bEnabled)
	{
		PrintHeader(TEXT("inter wall corners"));
		for (auto& Wall : BuildSystemPtr->Walls)
		{
			TArray<FModularSection*> WallSections = BuildSystemPtr->GetSectionsOfLevel(Wall);
			const int32 Num = WallSections.Num();
			for (int32 i = 0; i < Num; i++)
			{
				if (WallSections[i] == nullptr)
				{
					UE_LOG(LogGenerator, Error, TEXT("%s: Error on SetInterWallCorners: WallSection[%d] was nullptr."),
						*GetName(), i);
					return;
				}
			
				if (WallSections[i]->GetStaticMeshActor() == nullptr || !IsValid(WallSections[i]->GetStaticMeshActor()))
				{
					UE_LOG(LogGenerator, Error, TEXT("%s: Error on SetInterWallCorners: WallSection[%d]->StaticMesh was nullptr."),
						*GetName(), i);
					return;
				}
#if WITH_EDITOR
				UE_LOG(LogGenerator, Log, TEXT("%s: Initializing single section at %s actor transform."),
					*GetName(), *WallSections[i]->GetName());
#endif
				FTransform InitTransform = WallSections[i]->GetTransform();
				FVector Offset = InterWallCorners.Params.Offset;
				
				/*FMBSInitSingleSectionArgs Args;
				Args.Level = &Wall;
				Args.Transform = &InitTransform;
				Args.StaticMesh = InterWallCorners.Data.GetRandomMesh();
				Args.bAddToSections = true;
				Args.bWithRelativeTransform = false;
				Args.Offset = &Offset;
				BuildSystemPtr->InitSingleSection(Args);*/
				MBS::FSectionBuilder(BuildSystemPtr, &Wall)
					.Mesh(InterWallCorners.Data.GetRandomMesh())
					.At(InitTransform)
					.Spawn(false);
				
				//BuildSystemPtr->OffsetModularSection(Wall, Num + i - 1, InterWallCorners.Params.Offset);
				BuildSystemPtr->OffsetModularSection(Wall, BuildSystemPtr->GetSectionsOfLevel(Wall).Num() - 1, Offset);
			}
		}
	}
}

void UHouseBuildSystemGenerator::SetFloorHoles(TArray<FMBSLevelIdTransformPair>& OutFloorHoleIdTransforms)
{
	Floor_New->FloorHoleIdTransforms.Empty();
	if (!Floor_New->bWithFloorHoles)
	{
		return;
	}
	
	PrintHeader(TEXT("floor holes"));
	const int32 FloorsNum = BuildSystemPtr->Floors.Num();
	UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: FloorsNum = %d"), *GetName(), FloorsNum);
	if (FloorsNum <= 1)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s: Count of floors need to be larger than 1 (Current=%d). Skipping."),
			*GetName(), FloorsNum);
		return;
	}

	TArray<int32> OccupiedIndices;
	for (int32 i = 1; i < BuildSystemPtr->Floors.Num(); i++)
	{
		const FModularLevel& Floor = BuildSystemPtr->Floors[i];
		UStaticMesh* Mesh = Floor_New->FloorHoles.GetMesh(Floor_New->FloorHoleMeshIndex);
		const int32 SectionIndex = Floor_New->FloorHoles.GetIndex(
			FMBSIndexCalculationArgs(BuildSystemPtr, i, &OccupiedIndices, &Floor.GetInitializer()),
			0);

		FTransform OutTransform;
		MBS::FSectionBuilder(BuildSystemPtr, &Floor).Mesh(Mesh).At(SectionIndex).ReplaceIfInstancedOrSet();
		/*if (BuildSystemPtr->IsOfInstancedMeshConfigurationType())
		{
			BuildSystemPtr->ReplaceWithNonInstancedSection(
				Mesh, SectionIndex, Floor.Id, Floor.InstancedStaticMeshComponent, OutTransform);
		}
		else
		{
			BuildSystemPtr->SetMeshAt(Floor, SectionIndex, Mesh);
			OutTransform = BuildSystemPtr->GetSectionTransformAt(Floor, SectionIndex, false, false);
		}*/

		Floor_New->FloorHoleIdTransforms.Add({ Floor.GetId(), OutTransform });
	}
}

void UHouseBuildSystemGenerator::SetFloorHoleDoor(const TArray<FMBSLevelIdTransformPair>& InFloorHoleIdTransforms) const
{
	if (!Floor_New->FloorHoleDoor.bEnabled)
	{
		return;
	}
	
	PrintHeader(TEXT("floor hole door"));
	for (const auto& Pair : InFloorHoleIdTransforms)
	{
		if (Floor_New->FloorHoleDoor.Data.bUseActorList)
		{
			BuildSystemPtr->InitModularSectionActor(Pair.Transform, Pair.LevelId,
				Floor_New->FloorHoleDoor.Data.GetRandomActorClass(), true, true);
		}
	}
}

void UHouseBuildSystemGenerator::PrintHeader(const FString& InPropertyName) const
{
	UE_LOG(LogGenerator, Verbose, TEXT("%s: === Setting %s ==="), *GetName(), *InPropertyName);
}

bool UHouseBuildSystemGenerator::SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
{
	UE_LOG(LogGenerator, Verbose, TEXT("%s: Build system ptr is set to %s"), *GetName(), *InBuildSystem.GetObject()->GetName());
	BuildSystemPtr = Cast<AHouseBuildSystemActor>(InBuildSystem.GetObject());
	return BuildSystemPtr != nullptr;
}

TScriptInterface<IModularBuildSystemInterface> UHouseBuildSystemGenerator::GetBuildSystemPtr() const
{
	return BuildSystemPtr.Get();
}

void UHouseBuildSystemGenerator::LogGenerationSummary() const
{
	Super::LogGenerationSummary();
	UE_LOG(LogGenerator, Log,
		TEXT("%s: \n\t%d sections generated.\n\t%d actor sections generated.\n\t%d instanced sections generated."),
		*GetName(), BuildSystemPtr->GetStaticSections().Num(), BuildSystemPtr->GetActorSections().Num(), BuildSystemPtr->GetInstancedSections().Num());
	
	UE_LOG(LogGenerator, Verbose, TEXT("%s: Root component name after generation = %s."), *GetName(), *BuildSystemPtr->GetRootComponent()->GetName());
}

bool UHouseBuildSystemGenerator::CheckProperties() const
{
	bool bResult = Super::CheckProperties();
	CheckProperty({
		{ TEXT("Windows"), Windows_New, bCanHaveWindows },
		{ TEXT("Walls"), Walls_New, bCanHaveWalls },
		{ TEXT("Stairs"), Stairs_New, bCanHaveStairs },
		{ TEXT("Chimney"), Chimney_New, bCanHaveChimney },
		{ TEXT("Floor"), Floor_New, bCanHaveFloors },
		{ TEXT("Corners"), Corners_New2, bCanHaveCorners },
		{ TEXT("Entrance"), Entrances_New, bCanHaveEntrances },
		{ TEXT("Doors"), Doors_New, bCanHaveDoors },
		{ TEXT("Vegetation"), Vegetation_New, bCanHaveVegetation },
		{ TEXT("Roof"), Roof_New, bCanHaveRoof },
		{ TEXT("Rooftop"), Rooftop_New, bCanHaveRooftop },
		{ TEXT("Basement"), Basement_New, true }
	}, bResult);
	return bResult;
}

bool UHouseBuildSystemGenerator::CheckEntrancesCount(const FString& InLevelName, const TArray<int32>& InEntranceIndices) const
{
	const int32 CountOfEntrances = InEntranceIndices.Num();
	UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: InEntranceIndices.Num() = %d"), *GetName(), CountOfEntrances);
	if (CountOfEntrances == 0)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: Can't add %s due to entrances not found."), *GetName(), *InLevelName);
		return false;
	}
	return true;
}
