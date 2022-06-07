// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MBSBounds.generated.h"

struct FModularBuildStats;
class AModularBuildSystemActor;

USTRUCT(BlueprintType)
struct FMBSSideTransforms
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="SideTransform"/*, Meta=(MakeEditWidget=true)*/)
	FVector LeftLocation = FVector(0.f, -500.f, 0.f);

	UPROPERTY(EditAnywhere, Category="SideTransform"/*, Meta=(MakeEditWidget=true)*/)
	FVector FrontLocation = FVector(-500.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="SideTransform", Meta=(MakeEditWidget))
	FVector RightLocation = FVector(0.f, 500.f, 0.f);

	UPROPERTY(EditAnywhere, Category="SideTransform", Meta=(MakeEditWidget))
	FVector BackLocation = FVector(500.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="SideTransform", Meta=(MakeEditWidget))
	FVector TopLocation = FVector(0.f, 0.f, 500.f);

	FVector RightFront() const	{ return FVector(FrontLocation.X, RightLocation.Y, RightLocation.Z); }
	FVector RightBack() const	{ return FVector(BackLocation.X, RightLocation.Y, BackLocation.Z); }
	FVector LeftBack() const	{ return FVector(BackLocation.X, LeftLocation.Y, LeftLocation.Z); }
	FVector LeftFront() const	{ return FVector(FrontLocation.X, LeftLocation.Y, FrontLocation.Z); }
	
	FVector TopRightFront() const	{ return FVector(FrontLocation.X, RightLocation.Y, TopLocation.Z); }
	FVector TopRightBack() const	{ return FVector(BackLocation.X, RightLocation.Y, TopLocation.Z); }
	FVector TopLeftBack() const		{ return FVector(BackLocation.X, LeftLocation.Y, TopLocation.Z); }
	FVector TopLeftFront() const	{ return FVector(FrontLocation.X, LeftLocation.Y, TopLocation.Z); }
	
	static FName LeftLocationName()		{ return "LeftLocation"; }
	static FName FrontLocationName()	{ return "FrontLocation"; }
	static FName RightLocationName()	{ return "RightLocation"; }
	static FName BackLocationName()		{ return "BackLocation"; }
	static FName TopLocationName()		{ return "TopLocation"; }
};

USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FMBSBounds
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	FMBSSideTransforms Transforms = {};

	UPROPERTY(VisibleAnywhere)
	FIntVector Bounds = {};

	/**
 	 * @brief Previous bounds of a modular build system actor.
 	 */
	UPROPERTY(VisibleAnywhere)
	FIntVector PreviousBounds = FIntVector(-1);

	UPROPERTY(VisibleAnywhere)
	bool bInitialized = false;

	UPROPERTY(VisibleAnywhere)
	bool bLocked = false;

	UPROPERTY(VisibleAnywhere)
	bool bWasUpdated = false;

	UPROPERTY(VisibleAnywhere)
	FName UpdatedTransformName = {};

	static constexpr float MaxZ = UINT8_MAX;
	
public:	
	void Init(const AModularBuildSystemActor* InBuildSystem);

	FMBSSideTransforms GetTransforms() const;
	FIntVector GetBounds() const;

	//void SetBounds(const FModularBuildStats& BuildStats);
	void SetBounds(const FIntVector InBounds, const FVector StretchCoefficients = FVector::OneVector);
	
	bool UpdateBounds(const AModularBuildSystemActor* InBuildSystem, const FName InUpdatedTransformName);
	bool IsValid() const;
	
	void SetUpdatedTransformName(const FName NewUpdatedTransformName);
	void ResetUpdated();

	bool WasUpdated() const;
	bool WasAnyTransformJustModified() const;

	FVector GetUpdatedTransformLocation() const;
	FName GetUpdatedTransformName() const;

	// TODO: Refactor into Equals method returning -1 | 0 | 1
	bool IsCurrentXLessThanPrevious() const;
	bool IsCurrentYLessThanPrevious() const;
	bool IsCurrentZLessThanPrevious() const;
	
	bool IsCurrentXGreaterThanPrevious() const;
	bool IsCurrentYGreaterThanPrevious() const;
	bool IsCurrentZGreaterThanPrevious() const;

	int32 GetDifferenceX() const;
	int32 GetDifferenceY() const;
	int32 GetDifferenceZ() const;

	FBox GetBox(const FVector ScaleCoefficients = FVector::OneVector) const;

private:
	void InitTransforms(FIntVector InitialBounds, const FVector StretchCoefficients);
	void SetUpdated(const bool bUpdated, const FName NewUpdatedTransformName);
	static FName UsingBoundsUpdateName() { return FName("USING_SET_BOUNDS"); }
	
};
