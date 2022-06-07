// Fill out your copyright notice in the Description page of Project Settings.


#include "Shape/ModularLevelShapeL.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularSectionResolution.h"
#include "SectionBuilder.h"

void UModularLevelShapeL::ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	checkf(Args.InInitializer->GetResolution(), TEXT("Resolution was nullptr on transform adjustment as UpperLShape."));

	if (!CanShapeTransform(Args))
	{
		return;
	}
	
	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Adjusting transform as %s. InIndex = %d, InBounds = (x=%d, y=%d)"),
		*GetName(), bUpper ? TEXT("UpperLShape") : TEXT("LowerLShape"), Args.InIndex, Args.InBounds.X, Args.InBounds.Y);

	switch (Args.InInitializer->GetResolution()->GetSnapMode())
	{
		case EModularSectionResolutionSnapMode::Default:
		{
			AdjustDefaultTransform(Args);
			break;
		}
		case EModularSectionResolutionSnapMode::Wall:
		case EModularSectionResolutionSnapMode::Roof:
		{
			Execute_ShapeHouseWallTransform(this, Args);
			break;
		}
		case EModularSectionResolutionSnapMode::Corner:
		{
			Execute_ShapeHouseCornerTransform(this, Args);
			break;
		}
		case EModularSectionResolutionSnapMode::Rooftop:
		{
			Execute_ShapeHouseRooftopTransform(this, Args);	
			break;
		}
		case EModularSectionResolutionSnapMode::Custom: break;
		default: ;
	}
}

void UModularLevelShapeL::PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args)
{
	UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing remaining actors..."), *GetName());
	
	check(Args.InInitializer && Args.InInitializer->GetResolution());
	UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: \n\tLevelName=%s, \n\tSnapMode=%s"),
		*GetName(),
		*Args.InBuildSystem->GetLevelWithId(Args.InLevelId)->GetName(),
		*UEnum::GetValueAsString(Args.InInitializer->GetResolution()->GetSnapMode()));

	const FModularLevel* Level = Args.InBuildSystem->GetLevelWithId(Args.InLevelId);
	switch (Args.InInitializer->GetResolution()->GetSnapMode())
	{
	case EModularSectionResolutionSnapMode::Default: break;
	case EModularSectionResolutionSnapMode::Wall: break;
	case EModularSectionResolutionSnapMode::Roof:
		Args.InCountToSpawn = 2;
		for (int32 i = 0; i < Args.InCountToSpawn; i++)
		{
			UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing remaining roof. ZMultiplier=%.2f..."),
				*GetName(), Args.InInitializer->GetZMultiplier());

			// Calculate transform for two roofs
			FTransform CalculatedTransform = Args.InInitializer->GetResolution()->GetTransformShifted(
				/*Args.InBuildSystem->GetActorTransform()*/{},
				FVector(Depth, Args.InBounds.Y - Depth - i, Args.InZMultiplier),
				false);

			// Adjust rotation
			const FRotator Rotation = FRotator(CalculatedTransform.GetRotation());
			CalculatedTransform.SetRotation(static_cast<const FQuat>(FRotator(
				Rotation.Pitch,
				i == 1 ? Rotation.Yaw + 90.f : Rotation.Yaw + 0.f,
				Rotation.Roll)));

			MBS::FSectionBuilder(Args.InBuildSystem, &*Level)
				.Mesh(CornerRoofStaticMesh, Args.InInitializer->GetStaticMesh())
				.At(CalculatedTransform)
				.Spawn(true);

			// TODO: Finish builder call
			
			/*if (Args.InBuildSystem->IsOfInstancedMeshConfigurationType())
			{
				if (CornerRoofStaticMesh)
				{
					Args.InBuildSystem->InitModularSection(
						CornerRoofStaticMesh, CalculatedTransform, Args.InLevelId, true, true);
				}
				else
				{
					Args.InBuildSystem->AddNewInstance(
						CalculatedTransform, true, Level->InstancedStaticMeshComponent);
				}
			}
			else
			{
				Args.InBuildSystem->InitModularSection(
					Args.InInitializer->GetStaticMesh(), CalculatedTransform, Args.InLevelId, true, true);
			}*/

			UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placed new remaining roof (i=%d) at %s location."),
				*GetName(), i, *CalculatedTransform.GetLocation().ToCompactString());
		}
		break;
	case EModularSectionResolutionSnapMode::Rooftop: break;
	case EModularSectionResolutionSnapMode::Corner:
		Args.InCountToSpawn = 2;
		for (int32 i = 0; i < Args.InCountToSpawn; i++)
		{
			UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placing remaining corner. ZMultiplier=%.2f..."),
				*GetName(), Args.InInitializer->GetZMultiplier());
			
			// Calculate transform for two corners
			FTransform CalculatedTransform = Args.InInitializer->GetResolution()->GetTransformShifted(
				/*Args.InBuildSystem->GetActorTransform()*/{},
				FVector(i == 0 ? 0 : Depth, Args.InBounds.Y - Depth, Args.InZMultiplier - 1.f),
				false);

			// Rotate corners
			//CalculatedTransform.SetRotation(static_cast<const FQuat>(FRotator(0.f, 180.f, 0.f)));

			MBS::FSectionBuilder(Args.InBuildSystem, &*Level)
				.Mesh(Args.InInitializer->GetStaticMesh())
				.At(CalculatedTransform)
				.Spawn(true);
			// TODO: Test builder
			
			/*if (Args.InBuildSystem->IsOfInstancedMeshConfigurationType())
			{
				Args.InBuildSystem->AddNewInstance(
					CalculatedTransform, true, Level->InstancedStaticMeshComponent);
			}
			else
			{
				Args.InBuildSystem->InitModularSection(
					Args.InInitializer->GetStaticMesh(), CalculatedTransform, Args.InLevelId, true, true);
			}*/

			UE_LOG(LogModularLevelShape, Verbose, TEXT("%s: Placed new remaining corner (i=%d) at %s location."),
				*GetName(), i, *CalculatedTransform.GetLocation().ToCompactString());
		}
		break;
	default: ;
	}
}

void UModularLevelShapeL::UpdateSectionInitializer_Implementation(const FMBSUpdateSectionInitializerArgs& Args)
{
	check(Args.Initializer && Args.Initializer->GetResolution());
	switch (Args.Initializer->GetResolution()->GetSnapMode())
	{
	case EModularSectionResolutionSnapMode::Default: break;
	case EModularSectionResolutionSnapMode::Wall: break;
	case EModularSectionResolutionSnapMode::Roof: break;
	case EModularSectionResolutionSnapMode::Rooftop: break;
	case EModularSectionResolutionSnapMode::Corner:
		Args.Initializer->AddTotalCount(2);
		break;
	default: ;
	}
}

bool UModularLevelShapeL::CanShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) const
{
	if (!Args.InInitializer->GetResolution() || Args.InBounds.X < 2 || Args.InBounds.Y < 2)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Condition failed: Bounds.X and Bounds.Y must be larger or equal to 2."), *GetName());
		return false;
	}

	if (Args.InBounds.X <= Depth || Args.InBounds.Y <= Depth)
	{
		UE_LOG(LogModularLevelShape, Error, TEXT("%s: Condition failed: Depth must be less than Bounds.X or Bounds.Y."), *GetName());
		return false;
	}

	return true;
}

void UModularLevelShapeL::ShapeHouseWallTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	switch (UModularSectionResolution::PlacementGeneration)
	{
	case EMBSResolutionAlgorithmGeneration::Gen2:
		AdjustWallTransformGen2(Args);
		break;
	case EMBSResolutionAlgorithmGeneration::Gen3:
		AdjustWallTransform(Args);
		break;
	default: ;
	}
}

void UModularLevelShapeL::ShapeHouseCornerTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	if (Args.InIndex == Args.InInitializer->GetTotalCount() - 2)
	{
		//const float ResolutionShiftX = Initializer->Resolution->GetResolution().X;
		Args.OutAdjustedTransform->AddToTranslation(
			FVector(UModularSectionResolution::DefaultSectionSize * Depth, 0.f, 0.f));
	}
	*Args.bOutShouldBeSkipped = false;	
}

void UModularLevelShapeL::ShapeHouseRooftopTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	const int32 AdjustedY = FMath::CeilToInt(static_cast<float>(Args.InBounds.Y) / 2.f);
	const int32 MaxInRow = Args.InInitializer->GetMaxInRow();
	const int32 AdjustedIndex = Args.InIndex % MaxInRow;
	
	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Adjusting Rooftop. InIndex=%d >= MaxInRow=%d * AdjustedY=%d. InIndex %% MaxInRow = %d"),
		Args.InIndex, MaxInRow, AdjustedY, Args.InIndex % MaxInRow);

	if (Args.InIndex >= MaxInRow * (Args.InBounds.Y - Depth - 1) && AdjustedIndex < Depth)
	{
		*Args.bOutShouldBeSkipped = true;
	}
	else
	{
		*Args.bOutShouldBeSkipped = false;
	}
}

void UModularLevelShapeL::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (GET_MEMBER_NAME_CHECKED(UModularLevelShapeL, Depth) == PropertyName)
	{
		//if (Depth > Bounds.X)
	}
}

void UModularLevelShapeL::AdjustDefaultTransform(const FMBSShapeTransformArgs& Args) const
{
	const int32 MaxInRow = Args.InInitializer->GetMaxInRow();
	const int32 AdjustedIndex = Args.InIndex % MaxInRow;
	
	if (Args.InIndex >= MaxInRow * (Args.InBounds.Y - Depth) && AdjustedIndex < Depth)
	{
		*Args.bOutShouldBeSkipped = true;
	}
	else
	{
		*Args.bOutShouldBeSkipped = false;
	}
}

void UModularLevelShapeL::AdjustWallTransformGen2(const FMBSShapeTransformArgs& Args)
{
	const int32 TotalCount = Args.InInitializer->GetTotalCount();
	const int32 MaxInRow = Args.InInitializer->GetMaxInRow();
	
	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Index = %d, TotalCount = %d, MaxInRow = %d"), Args.InIndex, TotalCount, MaxInRow);
	const bool bEvenBoundsX = Args.InBounds.X % 2 == 0;

	// NO ROTATIONS
	// TODO: Implement. Currently not all possible cases are tested.
	const float ResolutionShiftY = Args.InInitializer->GetResolution()->GetValue().X;
	const float RightWallShiftY = -ResolutionShiftY * /*(InBounds.Y / 2)*/Depth;

	if (bEvenBoundsX)
	{
		if (Args.InBounds.X % 4)
		{
			const int32 RightRemainder = TotalCount - MaxInRow;
			if (Args.InIndex == TotalCount - 1
				|| (Args.InIndex >= RightRemainder && Args.InIndex < RightRemainder + Depth - 1)) // Right wall
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("Right wall found at index %d"), Args.InIndex);
				Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, RightWallShiftY, 0.f));
			}
			else if (Args.InIndex == TotalCount - 2
				|| (Args.InIndex > MaxInRow - 1 + 2 * (Args.InBounds.Y - Depth - 1) && Args.InIndex < TotalCount - MaxInRow)) // Front wall
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("Checking front wall at index %d"), Args.InIndex);
				if (Args.InIndex % 2 == 0)
				{
					UE_LOG(LogModularLevelShape, Verbose, TEXT("Front wall found at index %d"), Args.InIndex);

					const float ResolutionShiftX = Args.InInitializer->GetResolution()->GetValue().X * Depth;
					Args.OutAdjustedTransform->AddToTranslation(FVector(ResolutionShiftX, 0.f, 0.f));
				}
			}
		}
		else
		{
			const int32 RightRemainder = TotalCount - MaxInRow;
			if (Args.InIndex == TotalCount - 1
				|| (Args.InIndex >= RightRemainder && Args.InIndex < RightRemainder + Depth - 1)) // Right wall
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("Right wall found at index %d"), Args.InIndex);
				Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, RightWallShiftY, 0.f));
			}
			else if (Args.InIndex == TotalCount - 2
				|| (Args.InIndex > MaxInRow - 1 + 2 * (Args.InBounds.Y - Depth - 1) && Args.InIndex < TotalCount - MaxInRow)) // Front wall
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("Checking front wall at index %d"), Args.InIndex);
				if (Args.InIndex % 2 == 0)
				{
					UE_LOG(LogModularLevelShape, Verbose, TEXT("Front wall found at index %d"), Args.InIndex);

					const float ResolutionShiftX = Args.InInitializer->GetResolution()->GetValue().X * Depth;
					Args.OutAdjustedTransform->AddToTranslation(FVector(ResolutionShiftX, 0.f, 0.f));
				}
			}
		}
	}
	else // If odd bounds
	{
		const int32 MinRightIndex = TotalCount - MaxInRow + 1;
		const int32 DepthShift = Args.InBounds.Y - Depth - 1;
		//if (InIndex == TotalCount - MaxInRow + 2) // Right wall
		if (Args.InIndex > MinRightIndex && Args.InIndex < TotalCount - DepthShift - 1)
		{
			UE_LOG(LogModularLevelShape, Verbose, TEXT("Right wall found at index %d"), Args.InIndex);
			Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, RightWallShiftY, 0.f));
		}
		//else if ((InIndex > MaxInRow + 1 && InIndex < TotalCount - MaxInRow + 2)
		//	|| InIndex == TotalCount - MaxInRow + 1) // Front wall
		else if (Args.InIndex > MaxInRow + 2 * DepthShift && Args.InIndex <= MinRightIndex) // Front wall
		{
			UE_LOG(LogModularLevelShape, Verbose, TEXT("Checking front wall at index %d"), Args.InIndex);
			if (Args.InIndex % 2 == 0)
			{
				UE_LOG(LogModularLevelShape, Verbose, TEXT("Front wall found at index %d"), Args.InIndex);

				const float ResolutionShiftX = Args.InInitializer->GetResolution()->GetValue().X * Depth;
				Args.OutAdjustedTransform->AddToTranslation(FVector(ResolutionShiftX, 0.f, 0.f));
			}
		}
	}

	*Args.bOutShouldBeSkipped = false;
}

void UModularLevelShapeL::AdjustWallTransform(const FMBSShapeTransformArgs& Args)
{
	const int32 TotalCount = Args.InInitializer->GetTotalCount();
	const int32 MaxInRow = Args.InInitializer->GetMaxInRow();
	
	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Index = %d, TotalCount = %d, MaxInRow = %d"), Args.InIndex, TotalCount, MaxInRow);
	const bool bEvenBoundsX = Args.InBounds.X % 2 == 0;
	
	const float ResolutionShiftY = Args.InInitializer->GetResolution()->GetValue().X;
	const float RightWallShiftY = -ResolutionShiftY * Depth;
	
	const int32 Difference = Args.InBounds.Y - Args.InBounds.X;
	const int32 DepthShift = (Args.InBounds.X - Depth - 1 + Difference) * 2;

	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Args.InIndex > MaxInRow - (bEvenBoundsX ? 1 : 0) + DepthShift = "
		"%d > %d - %d + %d = %d > %d"), *GetName(), Args.InIndex, MaxInRow, bEvenBoundsX ? 1 : 0, DepthShift,
		Args.InIndex, MaxInRow - (bEvenBoundsX ? 1 : 0) + DepthShift);

	const int32 MinRightIndex = TotalCount - MaxInRow + 2;
	UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("%s: Args.InIndex < TotalCount - MaxInRow + 2 = "
		"%d < %d - %d + 2 = %d < %d"), *GetName(), Args.InIndex, TotalCount, MaxInRow, Args.InIndex, MinRightIndex);
	
	// Right wall
	if (Args.InIndex >= MinRightIndex && Args.InIndex < TotalCount - Args.InBounds.X + Depth)
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Right wall found at index %d"), Args.InIndex);
		Args.OutAdjustedTransform->AddToTranslation(FVector(0.f, RightWallShiftY, 0.f));
	}
	else if (Args.InIndex > MaxInRow - (bEvenBoundsX ? 1 : 0) + DepthShift // Front wall
		&& Args.InIndex < MinRightIndex
		&& Args.InIndex % 2 == 0)
	{
		UE_LOG(LogModularLevelShape, VeryVerbose, TEXT("Front wall found at index %d"), Args.InIndex);
		const float ResolutionShiftX = Args.InInitializer->GetResolution()->GetValue().X * Depth;
		Args.OutAdjustedTransform->AddToTranslation(FVector(ResolutionShiftX, 0.f, 0.f));
	}
}

int32 UModularLevelShapeL::GetShapedMaxIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const
{
	if (bUpper)
	{
		// TODO: Implement for bUpper
		return Bounds.X;
	}
	return IndexY < Bounds.Y - Depth ? Bounds.X : Bounds.X - Depth;
}

int32 UModularLevelShapeL::GetShapedMaxIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const
{
	if (bUpper)
	{
		// TODO: implement for bUpper
		return Bounds.Y;
	}
	return IndexX < Bounds.X - Depth ? Bounds.Y - Depth : Bounds.Y;
}

int32 UModularLevelShapeL::GetShapedMinIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const
{
	return 0;
}

int32 UModularLevelShapeL::GetShapedMinIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const
{
	return 0;
}
