// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSBounds.h"

#include "ModularBuildSystemActor.h"
#include "ModularSectionResolution.h"
#include "ModularBuildSystem.h"

void FMBSBounds::Init(const AModularBuildSystemActor* InBuildSystem)
{
	checkf(InBuildSystem, TEXT("BuildSystem == nullptr."));

	// TODO: Move bounds from BuildStats to this class
	const FModularBuildStats& BuildStats = InBuildSystem->GetBuildStats();
	UE_LOG(LogMBSBounds, Log, TEXT("%s: Initilizing bounds: BuildStats=%s"),
		*InBuildSystem->GetName(), *InBuildSystem->GetBuildStats().ToString());
	
	Bounds = FIntVector(BuildStats.Bounds.X, BuildStats.Bounds.Y, BuildStats.LevelCount);
	PreviousBounds = Bounds;
	
	InitTransforms(Bounds, FVector::OneVector);
	bInitialized = true;
}

FMBSSideTransforms FMBSBounds::GetTransforms() const
{
	return Transforms;
}

FIntVector FMBSBounds::GetBounds() const
{
	return Bounds;
}

void FMBSBounds::SetBounds(const FIntVector InBounds, const FVector StretchCoefficients)
{
	PreviousBounds = Bounds;
	Bounds = InBounds;
	InitTransforms(Bounds, StretchCoefficients);
	SetUpdated(true, UsingBoundsUpdateName());
}

bool FMBSBounds::UpdateBounds(const AModularBuildSystemActor* InBuildSystem, const FName InUpdatedTransformName)
{
	//UE_LOG(LogBuildSystem, Log, TEXT("UpdateBounds..."));
	if (!bInitialized && InBuildSystem)
	{
		Init(InBuildSystem);
	}
	
	if (!InBuildSystem)
	{
		UE_LOG(LogMBSBounds, Error, TEXT("InBuildSystem == nullptr"));
		return false;
	}

	// Get current modular build system bounds
	FModularBuildStats& BuildStats = const_cast<FModularBuildStats&>(InBuildSystem->GetBuildStats());
	const FIntVector InitialBounds = FIntVector(BuildStats.Bounds.X, BuildStats.Bounds.Y, BuildStats.LevelCount);
	Bounds = InitialBounds;
	PreviousBounds = Bounds;

	static constexpr float DefaultSize = UModularSectionResolution::DefaultSectionSize;

	// Calculate new Y
	if (Transforms.RightLocation.Y == 0.f)
	{
		UE_LOG(LogMBSBounds, Error, TEXT("Transforms.RightLocation.Y == 0.f"));
		return false;
	}
	Bounds.Y = FMath::Abs(Transforms.RightLocation.Y / DefaultSize);

	// Calculate new X
	if (Transforms.BackLocation.X == 0.f)
	{
		UE_LOG(LogMBSBounds, Error, TEXT("Transforms.BackLocation.X == 0.f"));
		return false;
	}
	Bounds.X = FMath::Abs(Transforms.BackLocation.X / DefaultSize);

	// Calculate new Z
	if (Transforms.TopLocation.Z == 0.f)
	{
		UE_LOG(LogMBSBounds, Error, TEXT("Transforms.TopLocation.Z == 0.f"));
		return false;
	}
	Bounds.Z = FMath::Clamp((Transforms.TopLocation.Z - DefaultSize * 2.f) / DefaultSize, 0.f, MaxZ);
	
	// Override existing bounds with new, calculated bounds
	if (Bounds != InitialBounds)
	{
		SetUpdated(true, InUpdatedTransformName);
		BuildStats.OverrideBounds(FIntPoint(Bounds.X, Bounds.Y));
		BuildStats.OverrideLevelCount(Bounds.Z);
		return true;
	}

	// TODO: Refactor
	// Set updated transform name
	//SetUpdated(false, InUpdatedTransformName);
	
	return false;
}

bool FMBSBounds::IsValid() const
{
	return !bLocked && bInitialized;
}

void FMBSBounds::SetUpdated(const bool bUpdated, const FName NewUpdatedTransformName)
{
	bWasUpdated = bUpdated;
	UpdatedTransformName = NewUpdatedTransformName;
}

void FMBSBounds::SetUpdatedTransformName(const FName NewUpdatedTransformName)
{
	UpdatedTransformName = NewUpdatedTransformName;
}

void FMBSBounds::ResetUpdated()
{
	SetUpdated(false, FName());
}

bool FMBSBounds::WasUpdated() const
{
	return bWasUpdated;
}

bool FMBSBounds::WasAnyTransformJustModified() const
{
	return !UpdatedTransformName.IsNone();
}

FVector FMBSBounds::GetUpdatedTransformLocation() const
{
	if (UpdatedTransformName.IsEqual(Transforms.LeftLocationName()))
	{
		return Transforms.LeftLocation;
	}
	if (UpdatedTransformName.IsEqual(Transforms.FrontLocationName()))
	{
		return Transforms.FrontLocation;
	}
	if (UpdatedTransformName.IsEqual(Transforms.RightLocationName()))
	{
		return Transforms.RightLocation;
	}
	if (UpdatedTransformName.IsEqual(Transforms.BackLocationName()))
	{
		return Transforms.BackLocation;
	}
	if (UpdatedTransformName.IsEqual(Transforms.TopLocationName()))
	{
		return Transforms.TopLocation;
	}
	return FVector::ZeroVector;
}

FName FMBSBounds::GetUpdatedTransformName() const
{
	return UpdatedTransformName;
}

bool FMBSBounds::IsCurrentXLessThanPrevious() const
{
	return Bounds.X < PreviousBounds.X;
}

bool FMBSBounds::IsCurrentYLessThanPrevious() const
{
	return Bounds.Y < PreviousBounds.Y;
}

bool FMBSBounds::IsCurrentZLessThanPrevious() const
{
	return Bounds.Z < PreviousBounds.Z;
}

bool FMBSBounds::IsCurrentXGreaterThanPrevious() const
{
	return Bounds.X > PreviousBounds.X;
}

bool FMBSBounds::IsCurrentYGreaterThanPrevious() const
{
	UE_LOG(LogMBSBounds, VeryVerbose, TEXT("IsCurrentYGreaterThanPrevious: Bounds=%s, PreviousBounds=%s"),
		*Bounds.ToString(), *PreviousBounds.ToString());
	
	return Bounds.Y > PreviousBounds.Y;
}

bool FMBSBounds::IsCurrentZGreaterThanPrevious() const
{
	UE_LOG(LogMBSBounds, Log, TEXT("IsCurrentZGreaterThanPrevious: Bounds=%s, PreviousBounds=%s"),
		*Bounds.ToString(), *PreviousBounds.ToString());
	
	return Bounds.Z > PreviousBounds.Z;
}

int32 FMBSBounds::GetDifferenceX() const
{
	return Bounds.X - PreviousBounds.X;
}

int32 FMBSBounds::GetDifferenceY() const
{
	return Bounds.Y - PreviousBounds.Y;
}

int32 FMBSBounds::GetDifferenceZ() const
{
	return Bounds.Z - PreviousBounds.Z;
}

FBox FMBSBounds::GetBox(const FVector ScaleCoefficients) const
{
	// Adjust Z component of bounds vector
	const FVector AdjustedBounds = FVector(Bounds.X, Bounds.Y, Bounds.Z + 2.f);
	
	// TODO: Refactor this. We shouldn't need to swap coefficients vector components.
	const FVector SwappedScaleCoefficients = FVector(ScaleCoefficients.Y, ScaleCoefficients.X, ScaleCoefficients.Z);
	
	return FBox(
		FVector::ZeroVector,
		AdjustedBounds * -UModularSectionResolution::DefaultSectionSize * SwappedScaleCoefficients);
}

void FMBSBounds::InitTransforms(const FIntVector InitialBounds, const FVector StretchCoefficients)
{
	// Initialize transforms using bounds
	// Currently assume that we have default modular section (400.f)
	// TODO: Use custom section size instead of default one
	const float CurrentX = InitialBounds.X * UModularSectionResolution::DefaultSectionSize * StretchCoefficients.X;
	const float CurrentY = InitialBounds.Y * UModularSectionResolution::DefaultSectionSize * StretchCoefficients.Y;
	Transforms.LeftLocation = FVector(/*0.f*/CurrentX * .5f, /*CurrentY * -1.f*/0.f, 0.f);
	Transforms.FrontLocation = FVector(/*CurrentX * -1.f*/0.f, CurrentY * .5f, 0.f);
	Transforms.RightLocation = FVector(CurrentX * .5f, CurrentY * 1.f, 0.f);
	Transforms.BackLocation = FVector(CurrentX * 1.f, /*0.f*/CurrentY * .5f, 0.f);

	// Then calculate top location using count of levels of MBS
	// Adjusting top handle location
	// TODO: Expose a property for ability to change this correction value in custom generators
	constexpr static float CorrectionZ = 2;
	const float CurrentZ = (InitialBounds.Z + CorrectionZ) * UModularSectionResolution::DefaultSectionSize * StretchCoefficients.Z;
	Transforms.TopLocation = FVector(0.f, CurrentY * .5f, CurrentZ);
}
