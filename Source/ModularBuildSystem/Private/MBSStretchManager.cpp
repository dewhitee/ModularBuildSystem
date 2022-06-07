// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSStretchManager.h"

#include "ModularSectionResolution.h"
#include "ModularBuildSystem.h"

float FMBSStretchManager::GetStretchOffset(const float InValue)
{
	return FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, UModularSectionResolution::DefaultSectionSize),
		FVector2D(0.f, 1.f),
		InValue);
}

float FMBSStretchManager::CalculateScaleCoefficient(const float ChangedAxisLocation, const int32 BoundsAxisValue)
{
	if (BoundsAxisValue == 0)
	{
		UE_LOG(LogMBSStretchManager, Warning, TEXT("BoundsAxisValue == 0; Returning 1.f"));
		return 1.f;
	}
	const float Remainder = FMath::Fmod(ChangedAxisLocation, UModularSectionResolution::DefaultSectionSize);
	const float StretchOffset = GetStretchOffset(Remainder);
	const float ScaleCoefficient = 1.f + StretchOffset / static_cast<float>(BoundsAxisValue);
	return ScaleCoefficient;
}

void FMBSStretchManager::UpdateScaleCoefficients(const FVector ChangedLocation, const FIntVector Bounds)
{
	SetScaleCoefficientX(CalculateScaleCoefficient(ChangedLocation.Y, Bounds.Y));
	SetScaleCoefficientY(CalculateScaleCoefficient(ChangedLocation.X, Bounds.X));
	SetScaleCoefficientZ(CalculateScaleCoefficient(ChangedLocation.Z, Bounds.Z));
	UE_LOG(LogMBSStretchManager, Log, TEXT("Updating scale coefficients: ChangedLocation=%s, Bounds=%s, (new) ScaleCoefficients=%s"),
		*ChangedLocation.ToCompactString(), *Bounds.ToString(), *ScaleCoefficients.ToCompactString());
}

void FMBSStretchManager::StretchSections(const MBS::FStretchArgs& Args, const EAxis::Type DirectionAxis,
	const FVector ScaleVec, const FVector LocationMultiplier,
	const TFunction<void(const MBS::FStretchSingleSectionArgs&)>& ForEachSection) const
{
	// TODO: Refactor
	const bool bSkipAnyLevelAxisDependentCorrection = !Args.LevelsSkippedInAxisDependentCorrection.IsEmpty();
	
	UE_LOG(LogMBSStretchManager, VeryVerbose, TEXT("Stretching sections: Sections.Num()=%d, DirectionAxis=%s, ScaleVec=%s,"
		" LocationMultiplier=%s, ScaleCoefficients=%s\n, bSkipAnyLevelAxisDependentCorrection=%s, Args.Transform.Rotator()=%s"),
		Args.Sections.Num(),
		*UEnum::GetValueAsString(DirectionAxis),
		*ScaleVec.ToCompactString(),
		*LocationMultiplier.ToCompactString(),
		*ScaleCoefficients.ToCompactString(),
		bSkipAnyLevelAxisDependentCorrection ? TEXT("true") : TEXT("false"),
		*Args.Transform.Rotator().ToString());
	
	for (FModularSectionBase* Section : Args.Sections)
	{
		FTransform SectionTransform = Section->GetTransform();
		const FTransform& RelativeSectionTransform = SectionTransform.GetRelativeTransform(Args.Transform);
		const FVector InitialRelativeLocation = RelativeSectionTransform.GetLocation();

		bool bIsInSameDirection;

		// Always run axis dependent correction for sections associated with any of the allowed levels
		if (Section->IsInLevel(Args.AxisDependentCorrectionAllowedLevels))
		{
			bIsInSameDirection = CorrectAxisDependent(DirectionAxis, Section, SectionTransform, ScaleVec);
		}
		else if (Args.bSkipAxisDependentCorrection ||
			(bSkipAnyLevelAxisDependentCorrection && Section->IsInLevel(Args.LevelsSkippedInAxisDependentCorrection)))
		{
			bIsInSameDirection = IsInSameDirection(DirectionAxis, Section);
		}
		else
		{
			bIsInSameDirection = CorrectAxisDependent(DirectionAxis, Section, SectionTransform, ScaleVec);
		}

		const FVector NewLocation = InitialRelativeLocation * LocationMultiplier;
		SectionTransform.SetLocation(NewLocation);
		
		// Update transform of a section with newly calculated
		if (Args.bAdjustLocation)
		{
			Section->SetTransform(SectionTransform, true);
		}
		
		UE_LOG(LogMBSStretchManager, VeryVerbose, TEXT("- %s - InitialRelativeLocation=%s, NewLocation=%s"),
			*Section->GetName(), *InitialRelativeLocation.ToCompactString(), *NewLocation.ToCompactString());

		if (ForEachSection)
		{
			ForEachSection(MBS::FStretchSingleSectionArgs(Section, bIsInSameDirection, DirectionAxis, LocationMultiplier));
		}
	}
}

void FMBSStretchManager::StretchSectionsUsingScaleCoefficients(const MBS::FStretchArgs& Args,
	const TFunction<void(const MBS::FStretchSingleSectionArgs&)> ForEachSection) const
{
	UE_LOG(LogMBSStretchManager, Verbose, TEXT("Stretching sections using scale coefficients: Sections.Num()=%d, ScaleCoefficients=%s"),
		Args.Sections.Num(), *ScaleCoefficients.ToCompactString());
	
	const FVector ScaleVectorX = FVector(ScaleCoefficients.X, 1.f, 1.f);
	if (ScaleCoefficients.X != 1.f && Args.bStretchByX)
	{
		StretchSections(
			Args,
			EAxis::X,
			ScaleVectorX,
			FVector(1.f, ScaleCoefficients.X, 1.f),
			ForEachSection);
	}
	
	const FVector ScaleVectorY = FVector(1.f, ScaleCoefficients.Y, 1.f);
	if (ScaleCoefficients.Y != 1.f && Args.bStretchByY)
	{
		StretchSections(
			Args,
			EAxis::Y,
			ScaleVectorY,
			FVector(ScaleCoefficients.Y, 1.f, 1.f),
			ForEachSection);
	}

	const FVector ScaleVectorZ = FVector(1.f, 1.f, ScaleCoefficients.Z);
	Args.bSkipAxisDependentCorrection = true;
	if (ScaleCoefficients.Z != 1.f && Args.bStretchByZ)
	{
		StretchSections(
			Args,
			EAxis::None, // TODO: Use EAxis::Z
			ScaleVectorZ,
			ScaleVectorZ,
			ForEachSection);
	}
	
	// TODO: Refactor
	FixSectionsRotation(Args);
}

bool FMBSStretchManager::CorrectAxisDependent(const EAxis::Type DirectionAxis, const FModularSectionBase* Section,
	FTransform& SectionTransform, const FVector ScaleVec) const
{
	const bool bIsInSameDirection = IsInSameDirection(DirectionAxis, Section);

	UE_LOG(LogMBSStretchManager, VeryVerbose, TEXT("- %s - (CorrectAxisDependent) DirectionAxis=%s, ScaleVec=%s, bIsInSameDirection=%s"),
		*Section->GetName(),
		*UEnum::GetValueAsString(DirectionAxis),
		*ScaleVec.ToCompactString(),
		bIsInSameDirection ? TEXT("true") : TEXT("false"));
	
	if (!bIsInSameDirection)
	{
		SectionTransform.SetScale3D(ScaleVec);
	}
		
	return bIsInSameDirection;
}

void FMBSStretchManager::FixSectionsRotation(const MBS::FStretchArgs& Args)
{
	for (FModularSectionBase* Section : Args.Sections)
	{
		FTransform SectionTransform = Section->GetTransform();
		FTransform RelativeSectionTransform = SectionTransform.GetRelativeTransform(Args.Transform);
		const FRotator InitialRotation = RelativeSectionTransform.Rotator();
		const FRotator NewRotation = InitialRotation - Args.Transform.Rotator();
		UE_LOG(LogMBSStretchManager, VeryVerbose, TEXT("- %s - InitialRotation=%s, NewRotation=%s"),
			*Section->GetName(),
			*InitialRotation.ToString(),
			*NewRotation.ToString());
		SectionTransform.SetRotation(NewRotation.Quaternion());
		Section->SetTransform(SectionTransform, false);
	}
}

bool FMBSStretchManager::IsInSameDirection(const EAxis::Type DirectionAxis, const FModularSectionBase* Section)
{
	float AbsoluteSectionAxis = 0.f;
	const FVector AbsoluteSectionDirection = Section->GetForwardVector();
	switch (DirectionAxis)
	{
	case EAxis::None:
		AbsoluteSectionAxis = 0.f;
		break;
	case EAxis::X:
		AbsoluteSectionAxis = FMath::Abs(AbsoluteSectionDirection.X);
		break;
	case EAxis::Y:
		AbsoluteSectionAxis = FMath::Abs(AbsoluteSectionDirection.Y);
		break;
	case EAxis::Z:
		AbsoluteSectionAxis = FMath::Abs(AbsoluteSectionDirection.Z);
		break;
	default: ;
	}
	
	return FMath::IsNearlyEqual(AbsoluteSectionAxis, 1.f, 0.1f);
}

bool FMBSStretchManager::ShouldAdjustLocation(const MBS::FStretchArgs& Args, const EAxis::Type DirectionAxis,
	const FModularSectionBase* Section)
{
	switch (DirectionAxis)
	{
	case EAxis::None:	return Args.bAdjustLocation;
	case EAxis::X:		return Args.bAdjustLocation && Args.bAdjustByXAxis && IsInSameDirection(EAxis::X, Section);
	case EAxis::Y:		return Args.bAdjustLocation && Args.bAdjustByYAxis && IsInSameDirection(EAxis::Y, Section);
	case EAxis::Z:		return Args.bAdjustLocation && Args.bAdjustByZAxis && IsInSameDirection(EAxis::Z, Section);
	default:			return Args.bAdjustLocation;
	}
}
