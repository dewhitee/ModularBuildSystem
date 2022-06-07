// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "ModularLevel.h"
#include "MBSStretchManager.generated.h"

struct FModularSectionBase;

namespace MBS
{
struct FStretchSingleSectionArgs
{
	FStretchSingleSectionArgs(FModularSectionBase* Section, const bool bIsInSameDirection, const EAxis::Type CurrentDirectionAxis,
		const FVector LocationMultiplier)
		: Section(Section)
		, bIsInSameDirection(bIsInSameDirection)
		, CurrentDirectionAxis(CurrentDirectionAxis)
		, LocationMultiplier(LocationMultiplier) {}
	
	FModularSectionBase* Section;
	bool bIsInSameDirection;
	EAxis::Type CurrentDirectionAxis;
	FVector LocationMultiplier;

	FString ToString() const
	{
		return FString::Printf(TEXT("Section=%s, bIsInSameDirection=%s, CurrentDirectionAxis=%s, LocationMultiplier=%s"),
			Section ? *Section->ToString() : TEXT("nullptr"),
			bIsInSameDirection ? TEXT("true") : TEXT("false"),
			*UEnum::GetValueAsString(CurrentDirectionAxis),
			*LocationMultiplier.ToCompactString());
	}
};

/**
 * Structure that holds arguments to pass into StretchSectionsUsingScaleCoefficients method of FMBSStretchManager
 * @see FMBSStretchManager
 */
struct FStretchArgs
{
	FStretchArgs() {}
	
	FStretchArgs(const TArray<FModularSectionBase*>& Sections, const FTransform& Transform,
		const bool bSkipAxisDependentCorrection, const TArray<FModularLevel> AxisDependentCorrectionWhitelistedLevels,
		const TArray<FModularLevel>& LevelsSkippedInAxisDependentCorrection)
		: Sections(Sections)
		, Transform(Transform)
		, bSkipAxisDependentCorrection(bSkipAxisDependentCorrection)
		, AxisDependentCorrectionAllowedLevels(AxisDependentCorrectionWhitelistedLevels)
		, LevelsSkippedInAxisDependentCorrection(LevelsSkippedInAxisDependentCorrection) {}

	/**
	 * Array of modular sections to stretch.
	 */
	TArray<FModularSectionBase*> Sections;
	
	/**
	 * Transform of a build system actor Sections are related to.
	 * @see Sections
	 */
	FTransform Transform;

	mutable bool bSkipAxisDependentCorrection = false;
	bool bAdjustLocation = true;
	bool bAdjustByXAxis = true;
	bool bAdjustByYAxis = true;
	bool bAdjustByZAxis = true;
	
	bool bStretchByX = true;
	bool bStretchByY = true;
	bool bStretchByZ = true;
	
	/**
	 * Levels that will have axis dependent correction indifferently from bSkipAxisDependentCorrection value.
	 */
	mutable TArray<FModularLevel> AxisDependentCorrectionAllowedLevels;
	
	/**
 	 * Used to exclude axis dependent correction for a section that is associated with any of these levels.
 	 * @see CorrectAxisDependent
 	 */
	mutable TArray<FModularLevel> LevelsSkippedInAxisDependentCorrection;
};
}

/**
 * Class that handles the stretch (transform adjustment) functionality of the modular sections of a MBS actor.
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FMBSStretchManager
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category=MBS)
	bool bPreserveScale = true;
	
	/**
	 * Scale coefficients that are used to adjust each modular section's scale and location.
	 */
	UPROPERTY(VisibleAnywhere, Category=MBS)
	FVector ScaleCoefficients = FVector::OneVector;

public:
	/**
	 * Get the stretch offset from the current InValue.
	 * @param InValue Current value, remainder.
	 * @return Mapped to [0;1] range current value in a [0;UModularSectionResolution::DefaultSectionSize] range.
	 * @see UModularSectionResolution
	 * @see UModularSectionResolution::DefaultSectionSize
	 */
	static float GetStretchOffset(const float InValue);
	
	/**
	 * Calculates the scale coefficient.
	 * @param ChangedAxisLocation Value of an axis that has changed. Supposed to be a current single axis location value of a transform bounds handle (edit widget).
	 * @param BoundsAxisValue Value of the bounds axis that has changed. E.g. Bounds.X where Bounds is an FIntPoint or FIntVector.
	 * @return Stretch coefficient.
	 */
	static float CalculateScaleCoefficient(const float ChangedAxisLocation, const int32 BoundsAxisValue);

	FVector GetScaleCoefficients() const { return ScaleCoefficients; }
	FVector GetScaleCoefficientsSwappedXY() const { return FVector(ScaleCoefficients.Y, ScaleCoefficients.X, ScaleCoefficients.Z); }
	
	void SetScaleCoefficientX(const float InValue) { ScaleCoefficients.X = InValue; }
	void SetScaleCoefficientY(const float InValue) { ScaleCoefficients.Y = InValue; }
	void SetScaleCoefficientZ(const float InValue) { ScaleCoefficients.Z = InValue; }
	
	/**
	 * Resets current scale coefficients vector to it's defaults.
	 */
	void ResetScaleCoefficients() { ScaleCoefficients = FVector::OneVector; }
	
	void UpdateScaleCoefficients(const FVector ChangedLocation, const FIntVector Bounds);
	
	void StretchSections(const MBS::FStretchArgs& Args, const EAxis::Type DirectionAxis, const FVector ScaleVec,
		const FVector LocationMultiplier, const TFunction<void(const MBS::FStretchSingleSectionArgs&)>& ForEachSection) const;
	void StretchSectionsUsingScaleCoefficients(const MBS::FStretchArgs& Args,
		const TFunction<void(const MBS::FStretchSingleSectionArgs&)> ForEachSection) const;

private:
	bool CorrectAxisDependent(const EAxis::Type DirectionAxis, const FModularSectionBase* Section,
		FTransform& SectionTransform, const FVector ScaleVec) const;
	static void FixSectionsRotation(const MBS::FStretchArgs& Args);
	static bool IsInSameDirection(const EAxis::Type DirectionAxis, const FModularSectionBase* Section);
	static bool ShouldAdjustLocation(const MBS::FStretchArgs& Args, const EAxis::Type DirectionAxis, const FModularSectionBase* Section);
};
