// Fill out your copyright notice in the Description page of Project Settings.


#include "Shape/ModularLevelShape.h"
#include "ModularBuildSystem.h"
#include "ModularLevel.h"
#include "ModularSectionResolution.h"

FMBSShapeTransformArgs::FMBSShapeTransformArgs()
	: InIndex(0)
	, InMaxInRow(0)
	, InAlreadySkippedCount(0)
	, InBounds(FIntPoint())
	, InInitializer(nullptr)
	, InBuildSystem(nullptr)
	, InLevelId(FModularLevel::InvalidLevelId)
	, OutAdjustedTransform(nullptr)
	, bOutShouldBeSkipped(nullptr) {};

void UModularLevelShape::ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args)
{
	unimplemented();
	UE_LOG(LogModularLevelShape, Warning, TEXT("%s: Default implementation of AdjustTransform was called. This should never happen."),
		*GetName());
	
	//UE_LOG(LogModularLevelShape, Warning, TEXT("%s: Checking if object is ready for ShapeTransform method to run."),
	//	*GetName());
	//return IsValidForShapeTransform(Args);
}

void UModularLevelShape::PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args)
{
	unimplemented();
	UE_LOG(LogModularLevelShape, Warning, TEXT("%s: Default implementation of PlaceRemainingActors was called. This should never happen."),
		*GetName());
}

void UModularLevelShape::UpdateSectionInitializer_Implementation(const FMBSUpdateSectionInitializerArgs& Initializer)
{
	unimplemented();
	UE_LOG(LogModularLevelShape, Warning, TEXT("%s: Default implementation of UpdateSectionInitializer was called. This should never happen."),
		*GetName());
}

void UModularLevelShape::BreakShapeTransformArgs(const FMBSShapeTransformArgs& InArgs, int32& InIndex,
	FIntPoint& InBounds, FModularSectionInitializer& InInitializer, FTransform& OutAdjustedTransform,
	bool& bOutShouldBeSkipped)
{
	InIndex = InArgs.InIndex;
	InBounds = InArgs.InBounds;
	InInitializer = *InArgs.InInitializer;
	OutAdjustedTransform = *InArgs.OutAdjustedTransform;
	bOutShouldBeSkipped = *InArgs.bOutShouldBeSkipped;
}

void UModularLevelShape::OffsetIndex(const FMBSShapeTransformArgs& InArgs, int32 Offset)
{
	InArgs.InIndex += Offset;
	UE_LOG(LogModularLevelShape, Verbose, TEXT("InArgs.InIndex = %d after OffsetIndex (Offset=%d)"),
		InArgs.InIndex, Offset);
}

void UModularLevelShape::OffsetMaxInRow(const FMBSShapeTransformArgs& InArgs, int32 Offset)
{
	UE_LOG(LogModularLevelShape, Verbose, TEXT("InArgs.MaxInRow initial = %d, InArgs.MaxInRow = %d after OffsetMaxInRow (Offset=%d)"),
		InArgs.InMaxInRow, InArgs.InMaxInRow + Offset, Offset);
	InArgs.InMaxInRow += Offset;
}

void UModularLevelShape::SetOutParams(FMBSShapeTransformArgs& InArgs, const FTransform& OutAdjustedTransform,
	const bool bOutShouldBeSkipped)
{
	*InArgs.OutAdjustedTransform = OutAdjustedTransform;
	*InArgs.bOutShouldBeSkipped = bOutShouldBeSkipped;
}

void UModularLevelShape::AddLocationToOutTransform(const FMBSShapeTransformArgs& InArgs, const FVector Location)
{
	check(InArgs.OutAdjustedTransform);
	InArgs.OutAdjustedTransform->AddToTranslation(Location);
}

void UModularLevelShape::AddLocationXToOutTransform(const FMBSShapeTransformArgs& InArgs, const float X)
{
	AddLocationToOutTransform(InArgs, FVector(X, 0.f, 0.f));
}

void UModularLevelShape::AddLocationYToOutTransform(const FMBSShapeTransformArgs& InArgs, const float Y)
{
	AddLocationToOutTransform(InArgs, FVector(0.f, Y, 0.f));
}

void UModularLevelShape::AddLocationZToOutTransform(const FMBSShapeTransformArgs& InArgs, const float Z)
{
	AddLocationToOutTransform(InArgs, FVector(0.f, 0.f, Z));
}

void UModularLevelShape::AddRotationToOutTransform(FMBSShapeTransformArgs& InArgs, const FRotator Rotation)
{
	check(InArgs.OutAdjustedTransform);
	InArgs.OutAdjustedTransform->SetRotation(static_cast<const FQuat>(InArgs.OutAdjustedTransform->Rotator() + Rotation));
}

bool UModularLevelShape::IsWallInMiddle(const FMBSShapeTransformArgs& InArgs)
{
	return UModularSectionResolution::IsWallInMiddle(InArgs.InIndex, InArgs.InMaxInRow, InArgs.InInitializer->GetTotalCount());
}

bool UModularLevelShape::IsFrontSectionIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingFront(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow(),
		InArgs.InInitializer->GetTotalCount());
	//return InArgs.InIndex % InArgs.InInitializer->MaxInRow == 0;
}

bool UModularLevelShape::IsBackDefaultSectionIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsDefaultFacingBack(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow());
}

bool UModularLevelShape::IsBackWallSectionIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingBack(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow(),
		InArgs.InInitializer->GetTotalCount());
	//return InArgs.InIndex % InArgs.InInitializer->MaxInRow == InArgs.InInitializer->MaxInRow - 1;
}

bool UModularLevelShape::IsRightSectionIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingRight(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow(),
		InArgs.InInitializer->GetTotalCount());
	//return InArgs.InIndex >= InArgs.InInitializer->TotalCount - InArgs.InInitializer->MaxInRow + 2;
}

bool UModularLevelShape::IsRightSectionLastIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingRightLast(InArgs.InIndex, InArgs.InInitializer->GetTotalCount());
}

bool UModularLevelShape::IsRightSectionFirstIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingRightFirst(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow(),
		InArgs.InInitializer->GetTotalCount());
}

bool UModularLevelShape::IsLeftSectionIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	//return InArgs.InIndex < InArgs.InInitializer->MaxInRow;
	return UModularSectionResolution::IsWallFacingLeft(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow());
}

bool UModularLevelShape::IsLeftSectionLastIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingLeftLast(InArgs.InIndex, InArgs.InInitializer->GetMaxInRow());
}

bool UModularLevelShape::IsLeftSectionFirstIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::IsWallFacingLeftFirst(InArgs.InIndex);
}

void UModularLevelShape::ExcludeAlreadySkipped(const FMBSShapeTransformArgs& InArgs)
{
	OffsetMaxInRow(InArgs, -InArgs.InAlreadySkippedCount);
}

int32 UModularLevelShape::GetWallLeftFirstIndex()
{
	return UModularSectionResolution::GetWallLeftFirstIndex();
}

int32 UModularLevelShape::GetWallRightFirstIndex(const FMBSShapeTransformArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::GetWallRightFirstIndex(InArgs.InMaxInRow, InArgs.InInitializer->GetTotalCount());
}

int32 UModularLevelShape::GetWallRightFirstIndex(const FMBSPlaceRemainingActorsArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::GetWallRightFirstIndex(
		InArgs.InInitializer->GetMaxInRow(), InArgs.InInitializer->GetTotalCount());
}

int32 UModularLevelShape::GetWallRightLastIndex(const FMBSPlaceRemainingActorsArgs& InArgs)
{
	check(InArgs.InInitializer);
	return UModularSectionResolution::GetWallRightLastIndex(InArgs.InInitializer->GetTotalCount());
}

void UModularLevelShape::SwitchSectionDirection(const FMBSShapeTransformArgs& InArgs, EMBSSectionDirection& Direction,
	FMBSShapeTransformArgs& OutArgs)
{
	OutArgs = InArgs;
	
	if (IsFrontSectionIndex(InArgs))
	{
		Direction = EMBSSectionDirection::Front;
		return;
	}
	
	if (IsBackDefaultSectionIndex(InArgs))
	{
		Direction = EMBSSectionDirection::Back;
		return;
	}

	// TODO: Implement Right and Back section directions
	unimplemented();
}
