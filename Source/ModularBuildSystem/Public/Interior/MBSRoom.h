// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSRoom.generated.h"

class UModularSectionResolution;

namespace MBS
{
struct FRoomBoundWallExclusion
{
	TArray<int32> LeftExcludedIndices;
	TArray<int32> RightExcludedIndices;
	TArray<int32> TopExcludedIndices;
	TArray<int32> BottomExcludedIndices;
	static TArray<int32> Until(int32 Index);
};
}

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMBSRoom
{
	GENERATED_BODY()

	FMBSRoom() {}
	FMBSRoom(const FName InName) : Name(InName) {}
	FMBSRoom(const FName InName, const FBox InBounds) : Name(InName), Transform(InBounds.Min), Bounds(InBounds) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Room)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Room)
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Room)
	FBox Bounds;

	// TODO: Should be array of FModularSectionBase derived structs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Room)
	TArray<AActor*> BoundWallSections;

	/**
	 * Calculates transforms of all potential bound wall sections from the existing Bounds.
	 * @param WallResolution Wall section resolution.
	 * @param InteriorBounds Interior or BuildSystem actor box bounds.
	 * @param PivotOffset Pivot offset from the modular level this room walls should be associated with.
	 * @param bKeepSingle Return transforms that creates walls at places where no other wall should exist.
	 * To avoid double-walls that could be created from other rooms.
	 * @param bKeepRight If true - right and 
	 * @param Excluded (Optional) Structure that holds indices of bound walls that should be excluded.
	 * @return Array of transforms where bound walls should be spawned.
	 *
	 * @see BoundWallSections
	 * @see Bounds
	 */
	TArray<FTransform> GetBoundWallTransforms(UModularSectionResolution* WallResolution = nullptr,
		const FBox* InteriorBounds = nullptr, const FVector PivotOffset = FVector::ZeroVector, bool bKeepSingle = false,
		bool bKeepRight = true, const MBS::FRoomBoundWallExclusion& Excluded = MBS::FRoomBoundWallExclusion()) const;
	
	TArray<FTransform> GetLeftBoundWallTransforms(UModularSectionResolution* WallResolution, const FVector PivotOffset,
		const TArray<int32>& ExcludedIndices = {}) const;
	TArray<FTransform> GetRightBoundWallTransforms(UModularSectionResolution* WallResolution, const FVector PivotOffset,
		const TArray<int32>& ExcludedIndices = {}) const;
	TArray<FTransform> GetTopBoundWallTransforms(UModularSectionResolution* WallResolution, const FVector PivotOffset,
		const TArray<int32>& ExcludedIndices = {}) const;
	TArray<FTransform> GetBottomBoundWallTransforms(UModularSectionResolution* WallResolution, const FVector PivotOffset,
		const TArray<int32>& ExcludedIndices = {}) const;

	/**
	 * Destroys all wall actors that form the room boundary.
	 */
	void ResetBoundWalls();
	
	FString ToString() const
	{
		return FString::Printf(TEXT("{Name=%s, FBox=%s}"), *Name.ToString(), *Bounds.ToString());
	}

private:
	FVector GetTopLeftLocation() const;
	FVector GetBottomRightLocation() const;

	TArray<FTransform> GetBoundWallTransformsHelper(const FVector V1, const FVector V2, const float Yaw,
		const FString& Text, const TArray<int32>& ExcludedIndices, const TFunction<FVector(int32)> ForEachIndex) const;
	static int32 GetBoundsCount(const float Distance);
	static FQuat GetQuat(const float Yaw);
};
