// Fill out your copyright notice in the Description page of Project Settings.


#include "Shape/ModularLevelShapeNonUniformSides.h"

#include "House/HouseBuildSystemActor.h"
#include "ModularBuildSystem.h"
#include "List/ModularBuildSystemMeshList.h"
#include "ModularSectionResolution.h"

void UModularLevelShapeNonUniformSides::ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	if (!IsDataPrepared())
	{
		return;
	}
	
	switch (Args.InInitializer->GetResolution()->GetSnapMode())
	{
	case EModularSectionResolutionSnapMode::Default:
		Execute_ShapeHouseDefaultTransform(this, Args);
		break;
	case EModularSectionResolutionSnapMode::Wall:
		Execute_ShapeHouseWallTransform(this, Args);
		break;
	case EModularSectionResolutionSnapMode::Roof:
		Execute_ShapeHouseRoofTransform(this, Args);
		break;
	case EModularSectionResolutionSnapMode::Rooftop: break;
	case EModularSectionResolutionSnapMode::Corner:
		Execute_ShapeHouseCornerTransform(this, Args);
		break;
	default: ;
	}
}

void UModularLevelShapeNonUniformSides::PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args)
{
	UE_LOG(LogModularLevelShape, Log, TEXT("%s: Placing remaining actors..."), *GetName(), Args.InLevelId);

	AHouseBuildSystemActor* BuildSystem = Cast<AHouseBuildSystemActor>(Args.InBuildSystem);
	if (!BuildSystem)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: BuildSystem is nullptr"), *GetName());
		return;
	}

	if (!IsDataPrepared())
	{
		return;
	}
	
	check(Args.InSkippedIndices);
	const int32 SkippedIndicesCount = (*Args.InSkippedIndices).Num();
	UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: SkippedIndicesCount=%d"), *GetName(), SkippedIndicesCount);

	const int32 ShiftX = Args.InInitializer->GetResolution()->GetX();

	auto ContainsId = [&](const FModularLevel& Level)
	{
		return Args.InLevelId == Level.GetId();
	};
	
	// Init small walls on skipped indices
	//const bool bContainsWallId = BuildSystem->Walls.ContainsByPredicate(ContainsId);
	if (BuildSystem->Walls.ContainsByPredicate(ContainsId))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing Walls related actors"), *GetName());

		const int32 SmallWallX = SmallSideWall.Resolution->GetX();
		UStaticMesh* Mesh = SmallSideWall.MeshList->GetMesh(0 , SmallSideWall.Resolution);
		
		for (int32 i = 0; i < SkippedIndicesCount; i++)
		{
			constexpr int32 LastWallIndexShift = 1;
			const int32 SkippedIndex = (*Args.InSkippedIndices)[i];
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: SkippedIndex=%d"), *GetName(), SkippedIndex);

			// minus index because we are shifting all indices when skipping them in ShapeTransform method
			const int32 CurrentIndex = SkippedIndex - i;

			const float FirstOffsetX = i == 0
				? -ShiftX
				: -SmallWallX;
			
			PlaceRemainingSection(Args, CurrentIndex, FVector(FirstOffsetX, 0.f, 0.f), Mesh, Args.InLevelId);
			
			UE_LOG(LogMBS, VeryVerbose, TEXT("%s: LastWallIndex=%d, ShiftX=%d, SmallWallX=%d, MaxInRow=%d, CurrentIndex=%d"),
				*GetName(), LastWallIndexShift, ShiftX, SmallWallX, Args.InInitializer->GetMaxInRow(), CurrentIndex);

			const float LastOffsetX = i == 0
				? /*-ShiftX * LastWallIndexShift*/ + SmallWallX + ShiftX * (Args.InBounds.X - 2)
				: ShiftX + ShiftX * (Args.InBounds.X - 2);
			
			PlaceRemainingSection(Args, CurrentIndex,
				FVector(LastOffsetX/* + SmallWallX*/, 0.f, 0.f), Mesh, Args.InLevelId);
		}
	}
	else if (BuildSystem->Floors.Num() > 0 && BuildSystem->Floors.ContainsByPredicate(ContainsId))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing Floors or Basement related actors"), *GetName());

		UStaticMesh* Mesh;
		if (bUniqueFirstFloor && Args.InLevelId == BuildSystem->Floors[0].GetId())
		{
			Mesh = FirstSmallFloor.MeshList->GetMesh(FirstSmallFloor.MeshListIndex, FirstSmallFloor.Resolution);
		}
		else
		{
			Mesh = SmallFloor.MeshList->GetMesh(SmallFloor.MeshListIndex, SmallFloor.Resolution);
		}
		
		const int32 SmallFloorX = SmallFloor.Resolution->GetX();
		//const int32 BasementY = BuildSystem->Basement.GetResolutionVec().Y;
		
		for (int32 i = 0; i < SkippedIndicesCount; i++)
		{
			constexpr int32 LastFloorIndex = 1;
			const int32 SkippedIndex = (*Args.InSkippedIndices)[i];
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: SkippedIndex=%d"), *GetName(), SkippedIndex);

			const int32 CurrentIndex = SkippedIndex - i;
			PlaceRemainingSection(Args, CurrentIndex, FVector(-SmallFloorX /*+ BasementY*/, 0.f, 0.f), Mesh,
				Args.InLevelId);

			UE_LOG(LogMBS, VeryVerbose, TEXT("%s: LastFloorIndex=%d, ShiftX=%d, SmallFloorX=%d, MaxInRow=%d, CurrentIndex=%d"),
				*GetName(), LastFloorIndex, ShiftX, SmallFloorX, Args.InInitializer->GetMaxInRow(), CurrentIndex);

			PlaceRemainingSection(Args, CurrentIndex,
				FVector((ShiftX * LastFloorIndex)/* + BasementY*/, 0.f, 0.f), Mesh, Args.InLevelId);
		}
	}
	else if (Args.InLevelId == BuildSystem->Roof.GetId() && SideRoof.MeshList && SideRoof.Resolution
		&& BuildSystem->Walls.Num() > 0)
	{
		if (SideRoof.MeshList && SideRoof.Resolution)
		{
			const int32 SmallWallX = SmallSideWall.Resolution->GetX();
			UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing Roof related actors"), *GetName());
			UStaticMesh* SideRoofMesh = SideRoof.MeshList->GetMesh(0, SideRoof.Resolution);
			//const float RoofY = BuildSystem->Roof.GetResolutionVec().Y;
			
			//constexpr int32 SideRoofCount = 2;
			const float WallZ = Args.InInitializer->GetResolution()->GetZ();

			const int32 LastWallId = BuildSystem->Walls.Last().GetId();
			const int32 RoofId = Args.InLevelId;
			Args.InLevelId = LastWallId;
			PlaceRemainingSection(Args, GetWallLeftFirstIndex(),
				FVector(0.f, SmallWallX * 1.f, -WallZ), SideRoofMesh, RoofId);

			// Add adjustment depending on Bounds.Y
			// TODO: Should be refactored later
			const float AdjustmentY = SmallWallX * (Args.InBounds.Y - 4);
			
			PlaceRemainingSection(Args, GetWallRightLastIndex(Args),
				FVector(0.f, SmallWallX * -1.f + AdjustmentY, -WallZ), SideRoofMesh, RoofId);
		}
	}
	else
	{
		const FModularLevel* Level = BuildSystem->GetLevelWithId(Args.InLevelId);
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: ModularLevel (%s) with Id=%d is not Wall or Floor"),
			*GetName(), Level ? *Level->GetName() : TEXT(""), Args.InLevelId);
	}
}

bool UModularLevelShapeNonUniformSides::CanShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) const
{
	return IsDataPrepared();
}

void UModularLevelShapeNonUniformSides::ShapeHouseDefaultTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	if (IsFrontSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Front, ShapeHouseDefaultTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);
		*Args.bOutShouldBeSkipped = true;
	}
	else if (IsBackDefaultSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Back, ShapeHouseDefaultTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);

		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: InitialLocation=%s, SmallFloor.Resolution->GetX()=%d, Args.InInitializer->Resolution->GetX()=%d"),
			*GetName(), *Args.OutAdjustedTransform->GetLocation().ToCompactString(), SmallFloor.Resolution->GetX(), Args.InInitializer->GetResolution()->GetX());
		
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-Args.InInitializer->GetResolution()->GetX() + SmallFloor.Resolution->GetX()/* + BasementY*/, 0.f, 0.f));

		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: AdjustedLocation=%s"),
			*GetName(), *Args.OutAdjustedTransform->GetLocation().ToCompactString());
	}
}

void UModularLevelShapeNonUniformSides::ShapeHouseWallTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	// Valid
	if (const AHouseBuildSystemActor* BuildSystem = Cast<AHouseBuildSystemActor>(Args.InBuildSystem))
	{
		if (BuildSystem->Basement.GetId() == Args.InLevelId)
		{
			if (IsLeftSectionLastIndex(Args) || IsRightSectionLastIndex(Args))
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Left/Right section index=%d"), *GetName(), Args.InIndex);
				Args.OutAdjustedTransform->AddToTranslation(
					FVector(-(SmallSideWall.Resolution->GetX() * 2.f), 0.f, 0.f));
			}
			else if (IsBackWallSectionIndex(Args))
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Back section index=%d"), *GetName(), Args.InIndex);
				Args.OutAdjustedTransform->AddToTranslation(
					FVector(-(SmallSideWall.Resolution->GetX() * 2.f), 0.f, 0.f));
			}
			return;
		}
	}

	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: AlreadySkippedCount=%d"), *GetName(), Args.InAlreadySkippedCount);
	OffsetMaxInRow(Args, -Args.InAlreadySkippedCount);
	//OffsetIndex(Args, -Args.InAlreadySkippedCount);
	if (IsLeftSectionFirstIndex(Args) || IsRightSectionFirstIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: LeftFirst OR RightFirst, ShapeHouseWallTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);
		*Args.bOutShouldBeSkipped = true;
	}
	else if (IsLeftSectionIndex(Args) && !IsFrontSectionIndex(Args) && !IsBackWallSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Left, ShapeHouseWallTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-Args.InInitializer->GetResolution()->GetX() + SmallSideWall.Resolution->GetX(), 0.f, 0.f));
	}
	else if (IsRightSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Right, ShapeHouseWallTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-Args.InInitializer->GetResolution()->GetX() + SmallSideWall.Resolution->GetX(), 0.f, 0.f));
	}
	else if (IsBackWallSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Right, ShapeHouseWallTransform_Implementation Index=%d"),
			*GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector( -SmallSideWall.Resolution->GetX() * 2.f, 0.f, 0.f));
	}

	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: *Args.bOutShouldBeSkipped=%s"),
		*GetName(), (*Args.bOutShouldBeSkipped ? TEXT("true") : TEXT("false")));
}

void UModularLevelShapeNonUniformSides::ShapeHouseRoofTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	//ShapeRoofV1(Args);
	ShapeRoofV2(Args);
}

void UModularLevelShapeNonUniformSides::ShapeHouseRooftopTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
}

void UModularLevelShapeNonUniformSides::ShapeHouseCornerTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	if (Args.InIndex % 2 != 0)
	{
		UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Shifting Corner at index=%d"), *GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-SmallSideWall.Resolution->GetX() * 2.f, 0.f, 0.f));
	}
}

bool UModularLevelShapeNonUniformSides::HasFrontIndicesInverted_Implementation() const
{
	return true;
}

bool UModularLevelShapeNonUniformSides::SkipsSections_Implementation() const
{
	return true;
}

bool UModularLevelShapeNonUniformSides::IsDataPrepared_Implementation() const
{
	if (!SmallSideWall.MeshList || !SmallSideWall.Resolution)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Condition failed: SmallSideWall.MeshList and SmallSideWall.Resolution must be set."),
			*GetName());
		return false;
	}

	if (bUniqueFirstFloor && (!FirstSmallFloor.MeshList || !FirstSmallFloor.Resolution))
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Condition failed: FirstSmallFloor.MeshList and FirstSmallFloor.Resolution must be set."),
			*GetName());
		return false;
	}

	if (!SmallFloor.MeshList || !SmallFloor.Resolution)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Condition failed: SmallFloor.MeshList and SmallFloor.Resolution must be set."),
			*GetName());
		return false;
	}

	return true;
}

void UModularLevelShapeNonUniformSides::PlaceRemainingSection(const FMBSPlaceRemainingActorsArgs& Args,
	const int32 Index, const FVector Offset, UStaticMesh* Mesh, const int32 LevelIdForTransform)
{
	UE_LOG(LogModularLevelShape, Verbose, TEXT("Spawning new small section. Index=%d, Offset=%s"),
		Index, *Offset.ToCompactString());

	if (!Mesh)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Mesh was nullptr. Check if MeshList has valid meshes set"));
		return;
	}
	
	FTransform Transform = Args.InBuildSystem->GetSectionTransformAt(LevelIdForTransform, Index, false, false);
	Transform.AddToTranslation(Offset);

	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Location=%s"), *Transform.GetLocation().ToCompactString());
	Args.InBuildSystem->InitModularSection(Mesh, Transform, Args.InLevelId, true, true);
}

void UModularLevelShapeNonUniformSides::ShapeRoofV1(const FMBSShapeTransformArgs& Args)
{
	Args.OutAdjustedTransform->SetRotation(
		static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));

	const float RoofY = Args.InInitializer->GetResolution()->GetY();
	const float SmallSideWallX = SmallSideWall.Resolution->GetX();
	const int32 TotalCount = Args.InInitializer->GetTotalCount();

	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Args.InIndex = %d, Args.InMaxInRow = %d, Args.InMaxCount = %d"),
		*GetName(), Args.InIndex, Args.InMaxInRow, TotalCount);

	//ExcludeAlreadySkipped(Args);
	//Args.InMaxInRow -= Args.InAlreadySkippedCount;
	if (IsFrontSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Front, Index=%d"), *GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(RoofY, SmallSideWallX * 2.5f, 0.f));
		if (Args.InIndex != 0)
		{
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Front with Index != 0, Index=%d"), *GetName(), Args.InIndex);
			Args.OutAdjustedTransform->AddToTranslation(
			FVector(0.f, -SmallSideWallX, 0.f));
		}
	}
	else if (IsBackWallSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Back wall, Index=%d"), *GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-RoofY - SmallSideWallX * 2.f, SmallSideWallX * /*-2.5f*/0.5f, 0.f));
		
		if (IsLeftSectionIndex(Args))
		{
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Left section, Index=%d"),
				*GetName(), Args.InIndex);
			Args.OutAdjustedTransform->AddToTranslation(
			FVector(0.f, SmallSideWallX * 1.f, 0.f));
		}
		else if (Args.InIndex > Args.InMaxInRow)
		{
			// Get current wall index
			const int32 CurrentWallIndex = UModularSectionResolution::GetWallCurrentRow(Args.InIndex, Args.InMaxInRow) - 1;

			// Multiply SmallSideWallX by current wall index
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: CurrentWallIndex=%d"), *GetName(), CurrentWallIndex);
			Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, -SmallSideWallX * CurrentWallIndex, 0.f));
		}
	}
	else if (IsRightSectionLastIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: RightLast, Index=%d"), *GetName(), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-SmallSideWallX, -SmallSideWallX * 2.5f, 0.f));
		Args.OutAdjustedTransform->SetRotation(
		static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));
	}
	else if (IsLeftSectionLastIndex(Args) || IsRightSectionFirstIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: LeftLast/RightFirst, Index=%d"), *GetName(), Args.InIndex);
		Args.OutAdjustedTransform->SetRotation(
			static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));

		Args.OutAdjustedTransform->AddToTranslation(
			FVector(0.f, SmallSideWallX * 0.5f, 0.f));

		if (IsLeftSectionLastIndex(Args))
		{
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Left section last index, Index=%d"),
				*GetName(), Args.InIndex);
			Args.OutAdjustedTransform->AddToTranslation(
			FVector(-RoofY - SmallSideWallX * 2.f, SmallSideWallX * 2.f, 0.f));
		}
		else if (IsRightSectionFirstIndex(Args))
		{
			UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Right section first index, Index=%d"),
				*GetName(), Args.InIndex);
			Args.OutAdjustedTransform->AddToTranslation(
				FVector(RoofY, 0.f, 0.f));
		}
	}
	else if (IsLeftSectionIndex(Args) || IsRightSectionIndex(Args))
	{
		// TODO: Think about if it is necessary at all
		//*Args.bOutShouldBeSkipped = true;
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(-SmallSideWallX, -SmallSideWallX * .5f, 0.f));

		Args.OutAdjustedTransform->SetRotation(
		static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));
	}
	else // If front middle (TEMPORAL) - should be handled by IsFrontSectionIndex(Args) at the start
	{
		// TODO: should be removed later after fix
		//Args.OutAdjustedTransform->AddToTranslation(FVector(RoofY, 0.f, 0.f));
	}
	Args.OutAdjustedTransform->AddToTranslation(
		FVector(0.f, 0.f, Args.InInitializer->GetResolution()->GetZ()));
}

void UModularLevelShapeNonUniformSides::ShapeRoofV2(const FMBSShapeTransformArgs& Args)
{
	Args.OutAdjustedTransform->SetRotation(
		static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));
	
	const float RoofY = Args.InInitializer->GetResolution()->GetY();
	const float SmallSideWallX = SmallSideWall.Resolution->GetX();
	const int32 TotalCount = Args.InInitializer->GetTotalCount();

	auto RotateYawBy90 = [&]()
	{
		Args.OutAdjustedTransform->SetRotation(
			static_cast<const FQuat>(Args.OutAdjustedTransform->Rotator().Add(0.f, 90.f, 0.f)));
	};

	auto ShiftByRoofY = [&](float Mul = 1.f)
	{
		AddLocationYToOutTransform(Args, RoofY * Mul);
		//Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, RoofY * Mul, 0.f));
	};

	auto AdjustRoof = [&]()
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Roof adjustment"), *GetName());
		const int32 WallCurrentRow = UModularSectionResolution::GetWallCurrentRow(Args.InIndex, Args.InMaxInRow);
		AddLocationYToOutTransform(Args, -SmallSideWallX * (WallCurrentRow));
	};
	
	if (IsFrontSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Front, Index=%d"), *GetName(), Args.InIndex);
		AddLocationXToOutTransform(Args, RoofY);

		if (Args.InIndex != 0)
		{
			AdjustRoof();
		}
	}
	else if (IsLeftSectionFirstIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: First left roof, Index=%d"), *GetName(), Args.InIndex);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Now this is first back"), *GetName(), Args.InIndex);
		RotateYawBy90();
		ShiftByRoofY(-1.f);
		AddLocationXToOutTransform(Args, -SmallSideWallX);
		//ShiftRoofByX(-SmallSideWallX);
	}
	else if (IsLeftSectionLastIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Last left roof, Index=%d"), *GetName(), Args.InIndex);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Now this is second back"), *GetName(), Args.InIndex);
		RotateYawBy90();
		AddLocationXToOutTransform(Args, -RoofY * 2.f + SmallSideWallX);
	}
	else if (IsBackWallSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Back roof, Index=%d"), *GetName(), Args.InIndex);
		AddLocationXToOutTransform(Args, -RoofY * 2.f + SmallSideWallX);
		const int32 WallCurrentRow = UModularSectionResolution::GetWallCurrentRow(Args.InIndex, Args.InMaxInRow);
		AddLocationYToOutTransform(Args, -SmallSideWallX * WallCurrentRow);

		if (IsWallInMiddle(Args))
		{
			AddLocationYToOutTransform(Args, -SmallSideWallX);
		}
	}
	else if (IsRightSectionFirstIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Right first roof, Index=%d"), *GetName(), Args.InIndex);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Now this is second front"), *GetName(), Args.InIndex);
		RotateYawBy90();
		ShiftByRoofY(-1);
		AddLocationXToOutTransform(Args, RoofY);
		
		const int32 WallCurrentRow = UModularSectionResolution::GetWallCurrentRow(Args.InIndex, Args.InMaxInRow);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: WallCurrentRow=%d"), *GetName(), WallCurrentRow);
		AddLocationYToOutTransform(Args, -SmallSideWallX * (Args.InBounds.Y - 3));
	}
	else if (IsRightSectionLastIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Right last roof, Index=%d"), *GetName(), Args.InIndex);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Now this is first front"), *GetName(), Args.InIndex);
		RotateYawBy90();
		AddLocationXToOutTransform(Args, -SmallSideWallX);

		const int32 WallCurrentRow = UModularSectionResolution::GetWallCurrentRow(Args.InIndex, Args.InMaxInRow);
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: WallCurrentRow=%d"), *GetName(), WallCurrentRow);
		AddLocationYToOutTransform(Args, -SmallSideWallX * (Args.InBounds.Y - 3));
	}
	else if (!IsBackWallSectionIndex(Args))
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Front (NOT-BACK condition), Index=%d"), *GetName(), Args.InIndex);
		AddLocationXToOutTransform(Args, RoofY);
		AdjustRoof();
	}
	else
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Other roof, Index=%d"), *GetName(), Args.InIndex);
	}
	
	// Update Z value of all roofs
	AddLocationZToOutTransform(Args, Args.InInitializer->GetResolution()->GetZ());
	//Args.OutAdjustedTransform->AddToTranslation(
	//	FVector(0.f, 0.f, Args.InInitializer->Resolution->GetZ()));

	// Shift all roofs to the right depending on Bounds.Y
	AddLocationYToOutTransform(Args, SmallSideWallX * 2.0f);
	//AddLocationYToOutTransform(Args, SmallSideWallX * (Args.InBounds.Y - 2.f));
}
