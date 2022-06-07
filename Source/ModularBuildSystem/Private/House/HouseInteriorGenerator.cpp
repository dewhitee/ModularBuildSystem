// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HouseInteriorGenerator.h"

#include "MBSFunctionLibrary.h"
#include "Treemap/MBSTreemap.h"
#include "LSystem/MBSLSystem.h"
#include "House/HouseBuildSystemActor.h"
#include "Interior/MBSInteriorPropList.h"
#include "ModularBuildSystem.h"
#include "List/ModularBuildSystemMeshList.h"
#include "List/ModularBuildSystemActorList.h"
#include "ModularSectionResolution.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "House/HouseBuildSystemGenerator.h"
#include "House/GenProperty/HouseDoorGeneratorProperty.h"
#include "House/GenProperty/HouseEntranceGeneratorProperty.h"
#include "House/GenProperty/HouseFloorGeneratorProperty.h"
#include "House/GenProperty/HouseWallGeneratorProperty.h"
#include "Treemap/MBSTreemapPartition.h"

FGeneratedInterior UHouseInteriorGenerator::Generate_Implementation()
{
	FGeneratedInterior OutGenerated = Super::Generate_Implementation();

	check(BuildSystemPtr);

	if (!CanGenerate())
	{
		UE_LOG(LogMBSInterior, Error, TEXT("%s: Condition failed on interior generation."), *GetName());
		return {};
	}

	// Preparing interior data
	Prepare();

	// Get basement transform
	const FTransform& BuildSystemTransform = BuildSystemPtr->GetActorTransform();

	// Set num of out generated levels
	const int32 FloorCount = bUseBasementAsFloor ? BuildSystemPtr->Floors.Num() + 1 : BuildSystemPtr->Floors.Num();
	OutGenerated.InteriorLevels.SetNum(FloorCount);
	UE_LOG(LogMBSInterior, Log, TEXT("%s: Iterating over floors: FloorCount=%d"), *GetName(), FloorCount);
	
	// Create prop on each floor
	for (int32 i = 0; i < FloorCount; i++)
	{
		FModularLevel CurrentLevel;
		FTransform FloorTransform;
		if (i == 0 && bUseBasementAsFloor)
		{
			CurrentLevel = BuildSystemPtr->Basement;
			FloorTransform = CurrentLevel.GetWorldTransform(BuildSystemTransform);
			FloorTransform.AddToTranslation(FVector(0.f, 0.f, CurrentLevel.GetResolutionVec().Z));
			UE_LOG(LogMBSInterior, VeryVerbose, TEXT("%s: FloorTransform=%s"), *GetName(), *FloorTransform.ToString());
		}
		else
		{
			CurrentLevel = BuildSystemPtr->Floors[bUseBasementAsFloor ? i - 1 : i];
			FloorTransform = CurrentLevel.GetWorldTransform(BuildSystemTransform);
		}

		CreateRooms(i, FloorTransform, OutGenerated);

		// TODO: Fill created rooms here / Or fill them later with all actors that are currently spawning below this line
		FillRooms(CurrentLevel, i, OutGenerated);
		
		// Get list of props to spawn

		// Add stairs before spawning other props, this way stairs will not be respawned in case of blocking collision
		// with other interior actors.
		AddStairs(i, OutGenerated);
		AddLadders(i, CurrentLevel, OutGenerated);
		AddFurnace(i, CurrentLevel, OutGenerated, FurnaceTransform);
		AddFurnaceChimney(i, CurrentLevel, OutGenerated, FurnaceTransform);
	}
	
	return OutGenerated;
}

void UHouseInteriorGenerator::Update_Implementation()
{
	Super::Update_Implementation();
}

void UHouseInteriorGenerator::ApplyPresets_Implementation()
{
	Super::ApplyPresets_Implementation();
	
	if (InnerWallsLSystem)
	{
		InnerWallsLSystem->SetGrammarFromPreset();
	}
	
	if (ItemsLSystem)
	{
		ItemsLSystem->SetGrammarFromPreset();
	}
}

bool UHouseInteriorGenerator::CheckBounds() const
{
	bool bResult = Super::CheckBounds();
	return bResult;
}

bool UHouseInteriorGenerator::CheckLists() const
{
	bool bResult = Super::CheckLists();
	CheckList({
		{ TEXT("Floors"), Floors.PropList, true },
		{ TEXT("Stairs"), Stairs.MeshList, true },
		{ TEXT("Ladders"), Ladders.Data.MeshList, Ladders.bSupported },
		{ TEXT("Furnace"), Furnace.Data.ActorList, Furnace.bSupported },
		{ TEXT("FurnaceChimney"), FurnaceChimney.Data.ActorList, FurnaceChimney.bSupported }
	}, bResult);
	
	return bResult;
}

bool UHouseInteriorGenerator::CheckResolutions() const
{
	bool bResult = Super::CheckResolutions();
	return bResult;
}

bool UHouseInteriorGenerator::CanGenerate() const
{
	return CheckLists() && CheckBounds() && CheckResolutions();
}

FTransform UHouseInteriorGenerator::CalculateNewTransform(const FMBSRoom& InRoom) const
{
	//FBox Box = BuildSystemPtr->GetModularLevelInteriorBox(&InLevel);
	FBox Box = InRoom.Bounds;

	// TODO: Scale box to get interior box (exclude walls)
	// Currently pass nullptr and use default wall resolution. Later we will need to pass the modular section resolution
	// of a wall level to this method alongside InLevel (current level upon which we are calculating new interior position)
	AdjustBoxToInteriorAllowedArea(Box, nullptr);
	
	FTransform NewTransform(FMath::RandPointInBox(Box));
	NewTransform.SetLocation(FVector(
		NewTransform.GetLocation().X,
		NewTransform.GetLocation().Y,
		BuildSystemPtr->GetActorLocation().Z + UModularSectionResolution::DefaultSectionSize));
	
	return NewTransform;
}

void UHouseInteriorGenerator::Prepare()
{
	// We check everything in CheckLists method, so this must be true.
	check(Floors.PropList);
	PrepareInteriorData(Floors);
}

void UHouseInteriorGenerator::CreateRooms(int32 InFloorIndex, const FTransform& InFloorTransform, FGeneratedInterior& OutGenerated)
{
	// TODO: Implement

	// Use LSystem if we have it
	if (InnerWallsTreemap)
	{
		// Get BS bounds box
		FBox BSBounds = BuildSystemPtr->GetBoundsBox(false);
		BSBounds.Min.Z = InFloorTransform.GetLocation().Z;
		BSBounds.Max.Z = InFloorTransform.GetLocation().Z;
		UE_LOG(LogMBSInterior, VeryVerbose, TEXT("%s: BSBounds=%s"), *GetName(), *BSBounds.ToString());

		// Adjust box if treemap uses Ratio as node size measure
		if (InnerWallsTreemap->GetMeasure() == EMBSTreemapSizeValueMeasure::Ratio)
		{
			// Adjusting bounds box to exclude outer walls of a building
			AdjustBoxToInteriorAllowedArea(BSBounds, 1.f, nullptr);
		}

		const FIntVector Bounds = BuildSystemPtr->GetTransformBounds().GetBounds();
		InnerWallsTreemap->SetSizeClampBounds(FVector(Bounds));
		InnerWallsTreemap->SetScaleCoefficients(BuildSystemPtr->GetStretchManager().GetScaleCoefficientsSwappedXY());
		
		TArray<FMBSTreemapPartition> Partitions =
			InnerWallsTreemap->CreatePartitions(BSBounds, InFloorTransform, InnerWallsTreemap->GetTree());

		// Adjust partitions depending on a building shape
		// TODO: Test
		InnerWallsTreemap->AdjustPartitionsByShape(Partitions, BuildSystemPtr->Generator->CustomLevelShape,
			FIntPoint(Bounds.X, Bounds.Y), InFloorTransform);
			
		auto Validator = MBS::FGenPropertyValidator({
			{InnerWalls, TEXT("InnerWalls")},
			{Entrances, TEXT("Entrances")},
			{Doors, TEXT("Doors")}
		}, TEXT("object is required but not set. Aborting room creation."));
	
		if (!Validator.Validate())
		{
			return;
		}
		
		for (const auto& Partition : Partitions)
		{
			FMBSRoom Room(Partition.Name, Partition.Bounds);

			// TODO: Spawn sections on the bounds
			// TODO: Implement
			if (BuildSystemPtr->Walls.IsValidIndex(InFloorIndex))
			{
				CreateSingleRoom(Room, BuildSystemPtr->Walls[InFloorIndex], InFloorIndex, BSBounds);
			}
			else
			{
				UE_LOG(LogMBSInterior, Error, TEXT("%s: BuildSystemPtr->Walls.IsValidIndex(%d) returned false"),
					*GetName(), InFloorIndex);
			}

			// Add room to current interior level room array
			OutGenerated.InteriorLevels[InFloorIndex].Rooms.Add(Room);
			UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Add partition %s"), *GetName(), *Room.ToString());
		}
		UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Rooms.Num()=%d"), *GetName(), OutGenerated.InteriorLevels[InFloorIndex].Rooms.Num());
	}
	else if (InnerWallsLSystem && InnerWalls)
	{
		// Placing inner walls that will layout rooms
		FMBSLSystemRunArgs RunArgs;
		RunArgs.BS = BuildSystemPtr;

		// TODO: Remove ClassToSpawn and keep only ClassesToSpawn?
		RunArgs.ClassToSpawn = AStaticMeshActor::StaticClass();
		RunArgs.ClassesToSpawn.Add(AStaticMeshActor::StaticClass());
		RunArgs.ClassesToSpawn.Add(Doors ? Doors->Data.GetRandomActorClass() : nullptr);

		// TODO: Remove MeshToSet and keep only MeshesToSet?
		RunArgs.MeshToSet = InnerWalls->Data.GetMesh(InnerWalls->MeshIndex);
		RunArgs.MeshesToSet.Add(InnerWalls->Data.GetMesh(InnerWalls->MeshIndex));
		RunArgs.MeshesToSet.Add(Entrances ? Entrances->Data.GetRandomMesh() : nullptr);
		
		RunArgs.LevelIndex = InFloorIndex + 1;
		InnerWallsLSystem->Run(RunArgs);

		// TODO: Get name of a room somehow
		const FName RoomName = *FString::Printf(TEXT("TEST_ROOM_%d"), InFloorIndex);
		FMBSRoom NewRoom = FMBSRoom(RoomName);
		NewRoom.BoundWallSections.Append(RunArgs.SpawnedActors);

		UE_LOG(LogMBSInterior, Verbose, TEXT("%s: NewRoom.BoundsWallSections.Num() = %d"), *GetName(), NewRoom.BoundWallSections.Num());
		OutGenerated.InteriorLevels[InFloorIndex].Rooms.Add(NewRoom);
	}
	// Create just a single room otherwise
	else
	{
		OutGenerated.InteriorLevels[InFloorIndex].Rooms.Add(FMBSRoom(TEXT("TEST_ROOM_SINGLE")));
	}
}

void UHouseInteriorGenerator::FillRooms(const FModularLevel& InLevel, int32 InFloorIndex, FGeneratedInterior& OutGenerated)
{
	// For each room
	FInteriorLevel& InteriorLevel = OutGenerated.InteriorLevels[InFloorIndex];
	for (const auto& Room : InteriorLevel.Rooms)
	{
		// Spawn static mesh props
		for (auto& StaticMesh : Floors.StaticMeshes)
		{
			const int32 MaxCount = FMath::RandRange(StaticMesh.Value.GetLowerBoundValue(), StaticMesh.Value.GetUpperBoundValue());
			for (int32 PropIndex = 0; PropIndex < MaxCount; PropIndex++)
			{
				AStaticMeshActor* NewStaticMesh = GetWorld()->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(), CalculateNewTransform(Room));
				NewStaticMesh->GetStaticMeshComponent()->SetStaticMesh(StaticMesh.Key);

				// Component should have overlap events enabled for transform adjusting using box trace.
				NewStaticMesh->GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
				AddNewInteriorActorChecked(NewStaticMesh, InteriorLevel, Room, InteriorLevel.StaticMeshActors, true);
			}
		}

		// Spawn skeletal mesh props
		for (auto& SkeletalMesh : Floors.SkeletalMeshes)
		{
			const int32 MaxCount = FMath::RandRange(SkeletalMesh.Value.GetLowerBoundValue(), SkeletalMesh.Value.GetUpperBoundValue());
			for (int32 PropIndex = 0; PropIndex < MaxCount; PropIndex++)
			{
				ASkeletalMeshActor* NewSkeletalMesh = GetWorld()->SpawnActor<ASkeletalMeshActor>(
					ASkeletalMeshActor::StaticClass(), CalculateNewTransform(Room));
				NewSkeletalMesh->GetSkeletalMeshComponent()->SetSkeletalMesh(SkeletalMesh.Key);
				
				// Component should have overlap events enabled for transform adjusting using box trace.
				NewSkeletalMesh->GetSkeletalMeshComponent()->SetGenerateOverlapEvents(true);
				AddNewInteriorActorChecked(NewSkeletalMesh, InteriorLevel, Room, InteriorLevel.SkeletalMeshActors, true);
			}
		}

		// Spawn actor props
		for (auto& Actor : Floors.Actors)
		{
			const int32 MaxCount = FMath::RandRange(Actor.Value.GetLowerBoundValue(), Actor.Value.GetUpperBoundValue());
			for (int32 PropIndex = 0; PropIndex < MaxCount; PropIndex++)
			{
				AActor* NewActor = GetWorld()->SpawnActor<AActor>(Actor.Key, CalculateNewTransform(Room));
				AddNewInteriorActorChecked(NewActor, InteriorLevel, Room, InteriorLevel.Actors, true);
			}
		}
	}
}

void UHouseInteriorGenerator::AddStairs(int32 InFloorIndex, FGeneratedInterior& OutGenerated) const
{
	if (!bWithStairs)
	{
		UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Skipping stairs"), *GetName());
		return;
	}	

	//const int32 Num = OutGenerated.InteriorLevels.Num();
	UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Adding stairs. OutGenerated.InteriorLevels.Num() = %d"),
		*GetName(), OutGenerated.InteriorLevels.Num());
	//for (int32 i = 0; i < Num; i++)
	//{
	UStaticMesh* StairsMesh = Stairs.GetRandomMesh();
	FInteriorLevel& InteriorLevel = OutGenerated.InteriorLevels[InFloorIndex];
	
	FModularLevel* CurrentLevel;
	verify((CurrentLevel = GetCurrentModularLevel(InFloorIndex)) != nullptr);

	// TODO: Add calculation class that will calculate those indices with custom shape in mind.
	///const float IndexX = -2.f;//BuildSystemPtr->GetShapeAdjustedMaxSectionIndexX(CurrentLevel->CustomShape, 0);
	///const float IndexY = -1.f;//BuildSystemPtr->GetShapeAdjustedMaxSectionIndexY(CurrentLevel->CustomShape, 0);
	const float IndexX = BuildSystemPtr->GetBuildStats().GetMinSectionIndexX(CurrentLevel->GetShape(), 0) - 2.f;
	const float IndexY = BuildSystemPtr->GetBuildStats().GetMinSectionIndexY(CurrentLevel->GetShape(), 0) - 1.f;
	
	AStaticMeshActor* NewStaticMesh = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		Stairs.Resolution->GetTransformShifted(
			BuildSystemPtr->GetActorTransform(),
			FVector(IndexX, IndexY, InFloorIndex + 1.5f),
			false));

	UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Adding stairs [%s] at %s transform."),
		*GetName(), *NewStaticMesh->GetName(), *NewStaticMesh->GetTransform().ToHumanReadableString());
	NewStaticMesh->GetStaticMeshComponent()->SetStaticMesh(StairsMesh);

	for (const auto& Room : InteriorLevel.Rooms)
	{
		AddNewInteriorActorChecked(NewStaticMesh, InteriorLevel, Room, InteriorLevel.StaticMeshActors, true);
	}
	//}
}

void UHouseInteriorGenerator::AddLadders(int32 InFloorIndex, const FModularLevel& CurrentLevel,
	FGeneratedInterior& OutGenerated) const
{
	if (!Ladders.bEnabled)
	{
		UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: Skipping ladders"), *GetName());
		return;
	}
	
	UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: CurrentLevel has Id=%d"), *GetName(), CurrentLevel.GetId());
	
	if (const UHouseBuildSystemGenerator* Generator = Cast<UHouseBuildSystemGenerator>(BuildSystemPtr->GetGenerator().GetObject()))
	{
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: Generator name = %s"), *GetName(), *Generator->GetName());
		if (!Generator->GetFloor()->bWithFloorHoles)
		{
			UE_LOG(LogInteriorGenerator, Error, TEXT("%s: Can't generate ladders when Generator->bWithFloorHoles == false"),
				*GetName());
			return;
		}
	
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: Generator->FloorHoleTransforms.Num() = %d"),
			*GetName(), Generator->GetFloor()->FloorHoleIdTransforms.Num());
		
		UStaticMesh* LadderMesh = Ladders.Data.GetRandomMesh();
		FInteriorLevel& InteriorLevel = OutGenerated.InteriorLevels[InFloorIndex];
		for (const auto& HoleIdTransform : Generator->GetFloor()->FloorHoleIdTransforms)
		{
			UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: HoleIdTransform.LevelId=%d"),
					*GetName(), HoleIdTransform.LevelId);
			if (HoleIdTransform.LevelId == CurrentLevel.GetId())
			{
				UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: Spawning new static mesh for CurrentLevel with Id=%d"),
					*GetName(), CurrentLevel.GetId());

				FTransform AtTransform = HoleIdTransform.Transform;
				AtTransform.AddToTranslation(Ladders.Params.Offset);
				
				AStaticMeshActor* NewStaticMesh = GetWorld()->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(),
					AtTransform);
				
				BuildSystemPtr->AttachActor(NewStaticMesh, true, true);
				NewStaticMesh->GetStaticMeshComponent()->SetStaticMesh(LadderMesh);
				
				// Make room from the whole CurrentLevel
				const FMBSRoom LevelRoom("TempRoom", UMBSFunctionLibrary::GetModularLevelInteriorBox(BuildSystemPtr, &CurrentLevel));
				
				AddNewInteriorActorChecked(NewStaticMesh, InteriorLevel, LevelRoom, InteriorLevel.StaticMeshActors, true);
			}
		}
	}
	else
	{
		UE_LOG(LogInteriorGenerator, Error, TEXT("%s: Generator was nullptr. Cast failed"), *GetName());
	}
}

void UHouseInteriorGenerator::AddFurnace(int32 InFloorIndex, const FModularLevel& CurrentLevel,
	FGeneratedInterior& OutGenerated, FTransform& OutFurnaceTransform) const
{
	if (!Furnace.bEnabled)
	{
		UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: Skipping furnace"), *GetName());
		return;
	}

	if (InFloorIndex != 0)
	{
		UE_LOG(LogInteriorGenerator, Warning, TEXT("%s: Furnace should be on the first floor only. Skipping."),
			*GetName(), InFloorIndex);
		return;
	}

	const TSubclassOf<AActor> ActorClass = Furnace.Data.GetRandomActorClass();
	if (const UHouseBuildSystemGenerator* Generator = Cast<UHouseBuildSystemGenerator>(BuildSystemPtr->GetGenerator().GetObject()))
	{
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: Generator name = %s"), *GetName(), *Generator->GetName());
		if (!Generator->CanHaveFloors() || !Generator->GetFloor()->bEnabled)
		{
			UE_LOG(LogInteriorGenerator, Error, TEXT("%s: Can't generate furnace when Generator->bWithFloors == false"),
				*GetName());
			return;
		}

		OutFurnaceTransform = BuildSystemPtr->GetSectionTransformAtRandom(
			CurrentLevel.GetId(), BuildSystemPtr->IsOfInstancedMeshConfigurationType(), false);
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: OutFurnaceTransform=%s"),
			*GetName(), *OutFurnaceTransform.ToHumanReadableString());

		AActor* NewFurnace = GetWorld()->SpawnActor<AActor>(ActorClass, OutFurnaceTransform);
		
		BuildSystemPtr->AttachActor(NewFurnace, true, true);
		//NewStaticMesh->GetStaticMeshComponent()->SetStaticMesh(Mesh);
		
		// Make room from the whole CurrentLevel
		const FMBSRoom LevelRoom("TempRoom", UMBSFunctionLibrary::GetModularLevelInteriorBox(BuildSystemPtr, &CurrentLevel));
		
		FInteriorLevel& InteriorLevel = OutGenerated.InteriorLevels[InFloorIndex];
		AddNewInteriorActorChecked(NewFurnace, InteriorLevel, LevelRoom, InteriorLevel.Actors, true);
	}
}

void UHouseInteriorGenerator::AddFurnaceChimney(int32 InFloorIndex, const FModularLevel& CurrentLevel,
	FGeneratedInterior& OutGenerated, const FTransform& InFurnaceTransform) const
{
	if (!Furnace.bEnabled || !FurnaceChimney.bEnabled)
	{
		UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: Skipping furnace chimney"), *GetName());
		return;
	}

	if (InFloorIndex == 0)
	{
		UE_LOG(LogInteriorGenerator, Warning, TEXT("%s: FurnaceChimney should not be on the first floor. Skipping."),
			*GetName(), InFloorIndex);
		return;
	}
	
	const TSubclassOf<AActor> ActorClass = FurnaceChimney.Data.GetRandomActorClass();
	if (const UHouseBuildSystemGenerator* Generator = Cast<UHouseBuildSystemGenerator>(BuildSystemPtr->GetGenerator().GetObject()))
	{
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: Generator name = %s"), *GetName(), *Generator->GetName());
		if (!Generator->CanHaveFloors() || !Generator->GetFloor()->bEnabled)
		{
			UE_LOG(LogInteriorGenerator, Error, TEXT("%s: Can't generate furnace when Generator->bWithFloors == false"),
				*GetName());
			return;
		}

		FTransform Transform = InFurnaceTransform;
		UE_LOG(LogInteriorGenerator, VeryVerbose, TEXT("%s: InFurnaceTransform=%s"),
			*GetName(), *InFurnaceTransform.ToHumanReadableString());
		Transform.AddToTranslation(FVector(0.f, 0.f, Furnace.Data.Resolution->GetZ()));

		AActor* NewFurnaceChimney = GetWorld()->SpawnActor<AActor>(ActorClass, Transform);
		BuildSystemPtr->AttachActor(NewFurnaceChimney, true, true);

		// Make room from the whole CurrentLevel
		const FMBSRoom LevelRoom("TempRoom", UMBSFunctionLibrary::GetModularLevelInteriorBox(BuildSystemPtr, &CurrentLevel));
		
		FInteriorLevel& InteriorLevel = OutGenerated.InteriorLevels[InFloorIndex];
		AddNewInteriorActorChecked(NewFurnaceChimney, InteriorLevel, LevelRoom, InteriorLevel.Actors, true);
	}
}

FModularLevel* UHouseInteriorGenerator::GetCurrentModularLevel(const int32 Index) const
{
	return Index == 0 && bUseBasementAsFloor ? &BuildSystemPtr->Basement : &BuildSystemPtr->Floors[Index];
}

bool UHouseInteriorGenerator::AddNewInteriorActor(AActor* Actor, const FInteriorLevel& InteriorLevel,
	const FMBSRoom& Room) const
{
	bool bStillOverlaps = false;
	if (Settings.bAdjustTransformIfOverlap)
	{
		AdjustInteriorActorTransform(Actor, InteriorLevel, Room, Settings.MaxAdjustTransformTryCount, bStillOverlaps);
	}
	
	if (bStillOverlaps && Settings.bSkipIfStillOverlap)
	{
		UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: Skipping %s interior actor."), *GetName(), *Actor->GetName());
		Actor->Destroy();
		return false;
	}
	return true;
}

void UHouseInteriorGenerator::CreateSingleRoom(FMBSRoom& Room, const FModularLevel& CurrentLevel, int32 InFloorIndex,
	FBox BSBounds) const
{
	// Adjust transforms depending on pivot location of a MBS actor
	const FVector PivotOffset = BuildSystemPtr->GetLocationShiftFromSectionPivot(CurrentLevel.GetPivotLocation());

	// Set indices of a bound walls that should be skipped
	MBS::FRoomBoundWallExclusion Excluded;
	if (CurrentLevel.GetShape())
	{
		// TODO: Implement
		//Excluded.RightExcludedIndices.Append(Excluded.Until())
	}
	
	TArray<FTransform> BoundWallTransforms =
		Room.GetBoundWallTransforms(
			nullptr,
			&BSBounds,
			PivotOffset,
			Settings.bKeepBoundWallsSingle,
			Settings.bKeepTopAndRightBoundWalls,
			Excluded);

	// TODO: Filter bound wall transforms using current shape
	FilterRoomBoundWalls(BoundWallTransforms, Room, CurrentLevel);

	// Initialize bound wall sections
	UE_LOG(LogMBSInterior, Verbose, TEXT("%s: PivotLocation=%s, BoundWallTransforms.Num()=%d, BSBounds=%s; \nSpawning bound walls for InFloorIndex=%d:"),
		*GetName(),
		*PivotOffset.ToCompactString(),
		BoundWallTransforms.Num(),
		*BSBounds.ToString(),
		InFloorIndex);
	
	TArray<FModularSection> Sections = BuildSystemPtr->InitMultipleModularSections(
		InnerWalls->Data.GetMesh(InnerWalls->MeshIndex),
		BoundWallTransforms,
		CurrentLevel.GetId(),
		true,
		false);

	// Set door
	if (Sections.IsEmpty())
	{
		return;
	}
	
	Sections[0].SetMesh(Entrances->Data.GetRandomMesh());
	FModularSectionActor ActorSection = BuildSystemPtr->InitModularSectionActor(
		Sections[0].GetTransform(),
		CurrentLevel.GetId(),
		Doors->Data.GetRandomActorClass(),
		true,
		false);
	Room.BoundWallSections.Add(ActorSection.GetActor());
	
	// Append spawned sections
	TArray<FModularSectionBase*> SectionsToStretch;
	SectionsToStretch.Add(&ActorSection);
	
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		SectionsToStretch.Add(&Sections[i]);
		Room.BoundWallSections.Add(Sections[i].GetStaticMeshActor());
		UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Spawned %s at location=%s"), *GetName(),
			*Sections[i].GetName(), *BoundWallTransforms[i].GetLocation().ToString());
	}
	UE_LOG(LogMBSInterior, Verbose, TEXT("%s: Add new %d sections..."), *GetName(), Sections.Num());

	// Stretch sections
	const FTransform& BSTransform = BuildSystemPtr->GetActorTransform();
	const FVector ScaleCoefficients = BuildSystemPtr->GetStretchManager().GetScaleCoefficientsSwappedXY();
	
	UE_LOG(LogMBSInterior, Verbose, TEXT("%s:\n\tBS transform location=%s,\n\tRoom %s transform location is %s,\n\tBounds=%s,\n\tScaleCoefficients=%s"),
		*GetName(),
		*BSTransform.GetLocation().ToString(),
		*Room.Name.ToString(),
		*Room.Transform.GetLocation().ToCompactString(),
		*Room.Bounds.ToString(),
		*ScaleCoefficients.ToCompactString());
	
	//FTransform AbsoluteRoomTransform = BuildSystemPtr->GetActorTransform().GetRelativeTransform(Room.Transform);
	MBS::FStretchArgs StretchArgs(
		SectionsToStretch,
		BSTransform,
		true,
		{},
		{});
	StretchArgs.bAdjustLocation = true;
	StretchArgs.bStretchByX = false;
	StretchArgs.bStretchByY = true;
	StretchArgs.bStretchByZ = true;

	auto StretchSingle = [ScaleCoefficients, BSTransform, Room](const MBS::FStretchSingleSectionArgs& Args)
	{
		const FVector CurrentScale = Args.Section->GetTransform().GetScale3D();
		const float XDiff = Room.Bounds.Min.X - BSTransform.GetLocation().X;
			
		UE_LOG(LogMBSInterior, VeryVerbose, TEXT("Stretching %s, XDiff=%.2f"), *Args.ToString(), XDiff);
		if (!Args.bIsInSameDirection && Args.CurrentDirectionAxis == EAxis::Y)
		{
			Args.Section->Scale(FVector(ScaleCoefficients.X, CurrentScale.Y, CurrentScale.Z));
			if (XDiff != 0.f && ScaleCoefficients.X != 0.f)
			{
				const FTransform RelativeTransform = Args.Section->GetTransform().GetRelativeTransform(BSTransform);
				const FVector RelativeLocation = RelativeTransform.GetLocation();
				Args.Section->SetLocation(FVector(
					RelativeLocation.X / ScaleCoefficients.X,
					RelativeLocation.Y,
					RelativeLocation.Z));
				UE_LOG(LogMBSInterior, VeryVerbose, TEXT("Adjusting room walls starting in the middle of the building."));
			}
		}
		else if (Args.CurrentDirectionAxis == EAxis::Z || Args.CurrentDirectionAxis == EAxis::None)
		{
			Args.Section->Scale(FVector(CurrentScale.X, CurrentScale.Y, ScaleCoefficients.Z));
		}
		else if (ScaleCoefficients.X != 0.f) // If room starts not on the bottom bound of a building
		{
			const FTransform RelativeTransform = Args.Section->GetTransform().GetRelativeTransform(BSTransform);
			const FVector RelativeLocation = RelativeTransform.GetLocation();
			Args.Section->SetLocation(FVector(
				RelativeLocation.X / ScaleCoefficients.X,
				RelativeLocation.Y,
				RelativeLocation.Z));
			UE_LOG(LogMBSInterior, VeryVerbose, TEXT("Adjusting room walls starting in the middle of the building."));
			//Args.Section->Offset(FVector(-400.f))
		}

		// Move back sections that spawned in room
		//TODO:
		
		
		//if (!Args.bIsInSameDirection && Args.CurrentDirectionAxis == EAxis::Y)
		//{
		//	Args.Section->Scale(Args.LocationMultiplier);
		//}
		//if (Args.CurrentDirectionAxis == EAxis::X)
		//{
		//	Args.Section->Scale(FVector(ScaleCoefficients.X, CurrentScale.Y, CurrentScale.Z));
		//}
	};
	BuildSystemPtr->GetStretchManager().StretchSectionsUsingScaleCoefficients(StretchArgs, StretchSingle);
}

void UHouseInteriorGenerator::FilterRoomBoundWalls(TArray<FTransform>& BoundWallTransforms, const FMBSRoom& Room,
	const FModularLevel& CurrentLevel) const
{
	if (!CurrentLevel.GetShape())
	{
		return;
	}

	// TODO: Implement to avoid mesh collisions
}

TScriptInterface<IModularBuildSystemInterface> UHouseInteriorGenerator::GetBuildSystemPtr() const
{
	return BuildSystemPtr.Get();
}

bool UHouseInteriorGenerator::SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
{
	BuildSystemPtr = Cast<AHouseBuildSystemActor>(InBuildSystem.GetObject());
	return BuildSystemPtr != nullptr;
}
