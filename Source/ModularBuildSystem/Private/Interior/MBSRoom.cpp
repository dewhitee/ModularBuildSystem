// Fill out your copyright notice in the Description page of Project Settings.


#include "Interior/MBSRoom.h"

#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"

TArray<int32> MBS::FRoomBoundWallExclusion::Until(int32 Index)
{
	TArray<int32> Out;
	for (int32 i = 0; i < Index; i++)
	{
		Out.Add(i);
	}
	return Out;
}

TArray<FTransform> FMBSRoom::GetBoundWallTransforms(UModularSectionResolution* WallResolution, const FBox* InteriorBounds,
	const FVector PivotOffset, bool bKeepSingle, bool bKeepRight, const MBS::FRoomBoundWallExclusion& Excluded) const
{
	TArray<FTransform> OutTransforms;
	if (WallResolution)
	{
		// TODO: Implement
		unimplemented();
		return {};
	}
	
	// Use default section size
	// Calculate transforms
	if (InteriorBounds)
	{
		// Exclude transforms lying on the interior bounds to avoid possible clipping of geometry
		const bool bIncludeLeftOnly = bKeepSingle && !bKeepRight;
		const bool bIncludeRightOnly = bKeepSingle && bKeepRight;
		
		// Left bounds
		if (Bounds.Min.Y != InteriorBounds->Min.Y && !bIncludeRightOnly)
		{
			OutTransforms.Append(GetLeftBoundWallTransforms(WallResolution, PivotOffset, Excluded.LeftExcludedIndices));
		}
		else
		{
			UE_LOG(LogMBSRoom, Verbose, TEXT("%s: Skipping left bound wall transforms"), *Name.ToString());
		}

		// Right bounds
		if (Bounds.Max.Y != InteriorBounds->Max.Y && !bIncludeLeftOnly)
		{
			OutTransforms.Append(GetRightBoundWallTransforms(WallResolution, PivotOffset, Excluded.RightExcludedIndices));
		}
		else
		{
			UE_LOG(LogMBSRoom, Verbose, TEXT("%s: Skipping right bound wall transforms"), *Name.ToString());
		}

		// Bottom bounds
		if (GetBottomRightLocation().X != InteriorBounds->Min.X && !bIncludeRightOnly)
		{
			OutTransforms.Append(GetBottomBoundWallTransforms(WallResolution, PivotOffset, Excluded.BottomExcludedIndices));
		}
		else
		{
			UE_LOG(LogMBSRoom, Verbose, TEXT("%s: Skipping bottom bound wall transforms"), *Name.ToString());
		}

		// Top bounds
		if (GetTopLeftLocation().X != InteriorBounds->Max.X && !bIncludeLeftOnly)
		{
			OutTransforms.Append(GetTopBoundWallTransforms(WallResolution, PivotOffset, Excluded.TopExcludedIndices));
		}
		else
		{
			UE_LOG(LogMBSRoom, Verbose, TEXT("%s: Skipping top bound wall transforms"), *Name.ToString());
		}
	}
	else
	{
		OutTransforms.Append(GetLeftBoundWallTransforms(WallResolution, PivotOffset, Excluded.LeftExcludedIndices));
		OutTransforms.Append(GetRightBoundWallTransforms(WallResolution, PivotOffset, Excluded.RightExcludedIndices));
		OutTransforms.Append(GetTopBoundWallTransforms(WallResolution, PivotOffset, Excluded.TopExcludedIndices));
		OutTransforms.Append(GetBottomBoundWallTransforms(WallResolution, PivotOffset, Excluded.BottomExcludedIndices));
	}
	return OutTransforms;
}

TArray<FTransform> FMBSRoom::GetLeftBoundWallTransforms(UModularSectionResolution* WallResolution,
	const FVector PivotOffset, const TArray<int32>& ExcludedIndices) const
{
	return GetBoundWallTransformsHelper(Bounds.Min, GetTopLeftLocation(), 180.f, "LeftBound", ExcludedIndices,
		[this, PivotOffset](int32 i)
		{
			return Bounds.Min + FVector(UModularSectionResolution::DefaultSectionSize * i + PivotOffset.Y, 0.f, 0.f);
		});
}

TArray<FTransform> FMBSRoom::GetRightBoundWallTransforms(UModularSectionResolution* WallResolution,
	const FVector PivotOffset, const TArray<int32>& ExcludedIndices) const
{
	return GetBoundWallTransformsHelper(Bounds.Max, GetBottomRightLocation(), 0.f, "RightBound", ExcludedIndices,
		[this, PivotOffset](int32 i)
		{
			return GetBottomRightLocation()
				+ FVector(UModularSectionResolution::DefaultSectionSize * i
				+ PivotOffset.Y - UModularSectionResolution::DefaultSectionSize, 0.f, 0.f);
		});
}

TArray<FTransform> FMBSRoom::GetTopBoundWallTransforms(UModularSectionResolution* WallResolution,
	const FVector PivotOffset, const TArray<int32>& ExcludedIndices) const
{
	return GetBoundWallTransformsHelper(GetTopLeftLocation(), Bounds.Max, -90.f, "TopBound", ExcludedIndices,
		[this](int32 i)
		{
			return Bounds.Max - FVector(0.f, UModularSectionResolution::DefaultSectionSize * i, 0.f);
		});
}

TArray<FTransform> FMBSRoom::GetBottomBoundWallTransforms(UModularSectionResolution* WallResolution,
	const FVector PivotOffset, const TArray<int32>& ExcludedIndices) const
{
	return GetBoundWallTransformsHelper(Bounds.Min, GetBottomRightLocation(), 90.f, "BottomBound", ExcludedIndices,
		[this](int32 i)
		{
			return Bounds.Min + FVector(0.f, UModularSectionResolution::DefaultSectionSize * i, 0.f);
		});
}

void FMBSRoom::ResetBoundWalls()
{
	for (auto& Actor : BoundWallSections)
	{
		Actor->Destroy();
		Actor = nullptr;
	}
	BoundWallSections.Empty();
}

FVector FMBSRoom::GetTopLeftLocation() const
{
	return FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z);
}

FVector FMBSRoom::GetBottomRightLocation() const
{
	return FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z);
}

TArray<FTransform> FMBSRoom::GetBoundWallTransformsHelper(const FVector V1, const FVector V2, const float Yaw, const FString& Text,
	const TArray<int32>& ExcludedIndices, const TFunction<FVector(int32 Index)> ForEachIndex) const
{
	TArray<FTransform> OutTransforms;
	const float Distance = FVector::Distance(V1, V2);
	const int32 BoundsCount = GetBoundsCount(Distance);
	const FQuat Rotation = GetQuat(Yaw);
	UE_LOG(LogMBSRoom, Verbose, TEXT("%s: Distance=%.2f, BoundsCount=%d, Rotation=%s"),
		*Name.ToString(), Distance, BoundsCount, *Rotation.ToString());

	for (int32 i = 0; i < BoundsCount; i++)
	{
		if (ExcludedIndices.Contains(i))
		{
			continue;
		}
		FTransform T(Rotation, ForEachIndex(i));
		UE_LOG(LogMBSRoom, VeryVerbose, TEXT("%s: %s %d => %s"), *Name.ToString(), i, *Text, *T.GetLocation().ToCompactString());
		OutTransforms.Add(T);
	}
	return OutTransforms;
}

int32 FMBSRoom::GetBoundsCount(const float Distance)
{
	return Distance / UModularSectionResolution::DefaultSectionSize;
}

FQuat FMBSRoom::GetQuat(const float Yaw)
{
	return FRotator(0.f, Yaw, 0.f).Quaternion();
}
