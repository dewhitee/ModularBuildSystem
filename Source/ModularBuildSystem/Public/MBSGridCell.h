// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace MBS
{

class MODULARBUILDSYSTEM_API FGridCellBase
{
protected:
	FBox	Bounds;
	FVector Multiplier		= FVector::OneVector;
	FVector RelativePivot	= FVector::ZeroVector;
	bool	bRelative		= false;

public:
	FGridCellBase(const FVector InMultiplier = FVector::OneVector) : Multiplier(InMultiplier) {}
	virtual ~FGridCellBase() = default;
	virtual FIntVector GetIndexXYZ() const = 0;

	FVector CenterRelative() const { return Relative(Bounds.GetCenter()); }
	FVector BottomLeftRelative() const;
	FVector TopLeftRelative() const;
	FVector TopRightRelative() const;
	FVector BottomRightRelative() const;
	
	FVector BottomLeft()	const { return Bounds.Min; }
	FVector TopLeft()		const { return FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z); }
	FVector TopRight()		const { return Bounds.Max; }
	FVector BottomRight()	const { return FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z); }

	float GetSizeX() const;
	float GetSizeY() const;
	float GetSizeZ() const;

	FBox GetBounds() const { return Bounds; }
	
	FString GetCode() const
	{
		const FIntVector XYZ = GetIndexXYZ();
		return FString::Printf(TEXT("(%d,%d,%d)"), XYZ.X, XYZ.Y, XYZ.Z);
	}
	
	FString ToString() const
	{
		return FString::Printf(
			TEXT("\n%s {\n\tBounds=%s, \n\tRelative Bounds=%s, \n\tXYZ={ %s }, \n\tMultiplier=%s, \n\tRelativePoint=%s, ")
			TEXT("\n\tSize=%s\n}"),
			*GetCode(),
			*Bounds.ToString(),
			*FBox(Relative(Bounds.Min), Relative(Bounds.Max)).ToString(),
			*GetIndexXYZ().ToString(),
			*Multiplier.ToCompactString(),
			*RelativePivot.ToCompactString(),
			*FVector(GetSizeX(), GetSizeY(), GetSizeZ()).ToCompactString());
	}

	int32 SecX(float V) const;
	int32 SecY(float V) const;
	int32 SecZ(float V) const;
	static int32 Sec(float V, const float InMultiplierComponent);

protected:
	FVector Relative(const FVector V) const;
	float AddX(const float V) const;
	float AddY(const float V) const;
	float AddZ(const float V) const;
	float MulX(const float V) const;
	float MulY(const float V) const;
	float MulZ(const float V) const;
};
	
class MODULARBUILDSYSTEM_API FGridCell2D : public FGridCellBase
{
public:
	FGridCell2D(int32 XIndex, int32 YIndex);
	FGridCell2D(int32 XIndex, int32 YIndex, const FVector InMultiplier);
	FGridCell2D(int32 XIndex, int32 YIndex, const FTransform& RelativeTo);
	FGridCell2D(int32 XIndex, int32 YIndex, const FTransform& RelativeTo, const FVector InMultiplier);
	FGridCell2D(const FBox& Bounds);
	FGridCell2D(const FBox& Bounds, const FVector InMultiplier);
	FGridCell2D(const FBox& Bounds, const FTransform& RelativeTo);
	FGridCell2D(const FBox& Bounds, const FTransform& RelativeTo, const FVector InMultiplier);
	
	virtual FIntVector GetIndexXYZ() const override;
};
	
class MODULARBUILDSYSTEM_API FGridCell3D : public FGridCellBase
{
public:
	FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, bool bFlatten = false);
	FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FVector InMultiplier, bool bFlatten = false);
	FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FTransform& RelativeTo, bool bFlatten = false);
	FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FTransform& RelativeTo, const FVector InMultiplier, bool bFlatten = false);
	FGridCell3D(const FBox& Bounds, bool bFlatten = false);
	FGridCell3D(const FBox& Bounds, const FVector InMultiplier, bool bFlatten = false);
	FGridCell3D(const FBox& Bounds, const FTransform& RelativeTo, bool bFlatten = false);
	FGridCell3D(const FBox& Bounds, const FTransform& RelativeTo, const FVector InMultiplier, bool bFlatten = false);
	FGridCell3D(const FVector V, bool bFlatten = false);
	FGridCell3D(const FVector V, const FVector InMultiplier, bool bFlatten = false);
	FGridCell3D(const FVector V, const FTransform& RelativeTo, bool bFlatten = false);
	FGridCell3D(const FVector V, const FTransform& RelativeTo, const FVector InMultiplier, bool bFlatten = false);
	
	virtual FIntVector GetIndexXYZ() const override;
};

class MODULARBUILDSYSTEM_API FGridMultiCell2D
{
	TArray<FBox> BoundsBoxes;
	
public:
	FGridMultiCell2D(TArray<FGridCell2D> GridCells);
	FGridMultiCell2D(const FBox& Bounds);
	FGridMultiCell2D(const FBox& Bounds, const FTransform& RelativeTo);
};
	
class MODULARBUILDSYSTEM_API FGridMultiCell3D
{
	TArray<FBox> BoundsBoxes;
	
public:
	FGridMultiCell3D(TArray<FGridCell3D> GridCells);
	FGridMultiCell3D(const FBox& Bounds);
	FGridMultiCell3D(const FBox& Bounds, const FTransform& RelativeTo);
};
	
}
