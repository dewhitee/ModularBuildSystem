// Fill out your copyright notice in the Description page of Project Settings.


#include "Solver/MBSTransformSolver.h"
#include "ModularBuildSystem.h"

#if WITH_EDITOR
#define WITH_ARGS \
	if (!CheckArgs())\
	{\
		return {};\
	}
#define WITH_ARGS_VOID \
	if (!CheckArgs())\
	{\
		return;\
	}
#else
#define WITH_ARGS {}
#define WITH_ARGS_VOID {}
#endif

void UMBSTransformSolver::In(FNextTransformArgs& Args)
{
	Arguments = &Args;
}

FTransform UMBSTransformSolver::Out()
{
	WITH_ARGS
	return Out(*Arguments);
}

FTransform UMBSTransformSolver::Out(const FNextTransformArgs& Args)
{
	return FTransform(Args.OutRotation, Args.OutLocation, Args.OutScale);
}

int32 UMBSTransformSolver::GetIndex() const
{
	WITH_ARGS
	return Arguments->InIndex;
}

int32 UMBSTransformSolver::GetMaxInRow() const
{
	WITH_ARGS
	return Arguments->InMaxInRow;
}

int32 UMBSTransformSolver::GetMaxCount() const
{
	WITH_ARGS
	return Arguments->InMaxCount;
}

float UMBSTransformSolver::GetLevelZMultiplier() const
{
	WITH_ARGS
	return Arguments->InLevelZMultiplier;
}

FModularBuildStats UMBSTransformSolver::GetBuildStats() const
{
	WITH_ARGS
	return Arguments->InBuildStats;
}

UModularSectionResolution* UMBSTransformSolver::GetPreviousLevelResolution() const
{
	WITH_ARGS
	return const_cast<UModularSectionResolution*>(Arguments->InPreviousLevelResolution);
}

UModularSectionResolution* UMBSTransformSolver::GetResolution() const
{
	WITH_ARGS
	return const_cast<UModularSectionResolution*>(Arguments->InResolution);
}

int32 UMBSTransformSolver::GetAdjustedIndex() const
{
	WITH_ARGS
	if (Arguments->InMaxInRow != 0.f)
	{
		return Arguments->InIndex % Arguments->InMaxInRow;
	}
	return 0.f;
}

int32 UMBSTransformSolver::GetAdjustedIndex(int32 Index, int32 MaxInRow)
{
	check(MaxInRow != 0.f);
	return Index % MaxInRow;
}

int32 UMBSTransformSolver::GetMaxRow() const
{
	WITH_ARGS
	return Arguments->InMaxCount / Arguments->InMaxInRow;
}

int32 UMBSTransformSolver::GetCurrentRow() const
{
	WITH_ARGS
	return Arguments->InIndex > 0 ? Arguments->InIndex / Arguments->InMaxInRow : 0;
}

bool UMBSTransformSolver::IsFacingFront() const
{
	WITH_ARGS
	return GetAdjustedIndex() == 0;
}

bool UMBSTransformSolver::IsFacingBack() const
{
	WITH_ARGS
	return GetAdjustedIndex() == Arguments->InMaxInRow - 1;
}

bool UMBSTransformSolver::IsFacingLeft() const
{
	WITH_ARGS
	return GetCurrentRow() == 0;
}

bool UMBSTransformSolver::IsFacingRight() const
{
	WITH_ARGS
	return GetCurrentRow() == GetMaxRow() - 1;
}

int32 UMBSTransformSolver::GetAdjustedIndex(const FModularBuildStats& BuildStats) const
{
	return Arguments->InIndex % BuildStats.MaxCountInRow;
}

int32 UMBSTransformSolver::GetMaxRow(int32 MaxCount, int32 MaxInRow)
{
	return MaxCount / MaxInRow;
}

int32 UMBSTransformSolver::GetCurrentRow(int32 Index, int32 MaxInRow)
{
	return Index > 0 ? Index / MaxInRow : 0;
}

int32 UMBSTransformSolver::GetCurrentRow(const FModularBuildStats& BuildStats) const
{
	return Arguments->InIndex > 0 ? Arguments->InIndex / BuildStats.MaxCountInRow : 0;
}

int32 UMBSTransformSolver::GetWallCurrentRow(int32 Index, int32 MaxInRow)
{
	return GetCurrentRow(Index - MaxInRow, 2) + 1;
}

bool UMBSTransformSolver::IsFacingFront(int32 AdjustedIndex)
{
	return AdjustedIndex == 0;
}

bool UMBSTransformSolver::IsFacingBack(int32 AdjustedIndex, int32 MaxInRow)
{
	return AdjustedIndex == MaxInRow - 1;
}

bool UMBSTransformSolver::IsFacingBack(const FModularBuildStats& BuildStats) const
{
	return GetAdjustedIndex() == BuildStats.MaxCountInRow - 1;
}

bool UMBSTransformSolver::IsFacingLeft(int32 CurrentRow)
{
	return CurrentRow == 0;
}

bool UMBSTransformSolver::IsFacingRight(int32 CurrentRow, int32 MaxCount, int32 MaxInRow)
{
	return CurrentRow == GetMaxRow(MaxCount, MaxInRow) - 1;
}

bool UMBSTransformSolver::IsFacingRight(const FModularBuildStats& BuildStats) const
{
	return GetCurrentRow() == GetMaxRow(BuildStats.MaxTotalCount, BuildStats.MaxCountInRow) - 1;
}

void UMBSTransformSolver::AddLocation(FVector ToAdd)
{
	WITH_ARGS_VOID
	Arguments->OutLocation += ToAdd;
}

void UMBSTransformSolver::AddLocationX(float X)
{
	WITH_ARGS_VOID
	Arguments->OutLocation.X += X;
}

void UMBSTransformSolver::AddLocationY(float Y)
{
	WITH_ARGS_VOID
	Arguments->OutLocation.Y += Y;
}

void UMBSTransformSolver::AddLocationZ(float Z)
{
	WITH_ARGS_VOID
	Arguments->OutLocation.Z += Z;
}

void UMBSTransformSolver::AddRotation(FRotator ToAdd)
{
	WITH_ARGS_VOID
	Arguments->OutRotation += ToAdd;
}

void UMBSTransformSolver::AddRotationRoll(float Roll)
{
	WITH_ARGS_VOID
	Arguments->OutRotation.Roll += Roll;
}

void UMBSTransformSolver::AddRotationPitch(float Pitch)
{
	WITH_ARGS_VOID
	Arguments->OutRotation.Pitch += Pitch;
}

void UMBSTransformSolver::AddRotationYaw(float Yaw)
{
	WITH_ARGS_VOID
	Arguments->OutRotation.Yaw += Yaw;
}

void UMBSTransformSolver::Skip()
{
	WITH_ARGS_VOID
	UE_LOG(LogTransformSolver, Verbose, TEXT("%s: Skip %d index"), *GetName(), Arguments->InIndex);
	Arguments->bOutShouldBeSkipped = true;
}

float UMBSTransformSolver::GetDefaultSectionSize()
{
	return UModularSectionResolution::DefaultSectionSize;
}

void UMBSTransformSolver::MirrorByX() const
{
	Arguments->OutScale.X *= -1;
}

void UMBSTransformSolver::MirrorByY() const
{
	Arguments->OutScale.Y *= -1;
}

void UMBSTransformSolver::MirrorByZ() const
{
	Arguments->OutScale.Z *= -1;
}

bool UMBSTransformSolver::IsIndexEquals(int32 Index) const
{
	WITH_ARGS
	return Arguments->InIndex == Index;
}

bool UMBSTransformSolver::IsIndexEqualsAnyOf(TArray<int32> Indices)
{
	WITH_ARGS
	return Indices.Contains(Arguments->InIndex);
}

bool UMBSTransformSolver::IsAdjustedIndexEquals(int32 Index) const
{
	WITH_ARGS
	return GetAdjustedIndex() == Index;
}

bool UMBSTransformSolver::IsAdjustedIndexEqualsAnyOf(TArray<int32> Indices)
{
	WITH_ARGS
	return Indices.Contains(GetAdjustedIndex());
}

bool UMBSTransformSolver::CheckArgs() const
{
#if WITH_EDITOR
	const bool bResult = Arguments != nullptr;
	if (!bResult)
	{
		const FString& ErrorMsg = FString::Printf(
			TEXT("%s: GetNextTransform method must call 'In' node at the start of execution."), *GetName());
		UE_LOG(LogTransformSolver, Error, TEXT("%s"), *ErrorMsg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *ErrorMsg);
		}
	}
	return bResult;
#else
	return Arguments != nullptr;
#endif
}

#undef WITH_ARGS
#undef WITH_ARGS_VOID