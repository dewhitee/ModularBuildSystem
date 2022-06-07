// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSGridCell.h"

#include "ModularSectionResolution.h"

namespace
{
	constexpr float GDss = UModularSectionResolution::DefaultSectionSize;
}

FVector MBS::FGridCellBase::BottomLeftRelative() const
{
	return Relative(BottomLeft());
}

FVector MBS::FGridCellBase::TopLeftRelative() const
{
	return Relative(TopLeft());
}

FVector MBS::FGridCellBase::TopRightRelative() const
{
	return Relative(TopRight());
}

FVector MBS::FGridCellBase::BottomRightRelative() const
{
	return Relative(BottomRight());
}

float MBS::FGridCellBase::GetSizeX() const
{
	return GDss * Multiplier.X;
}

float MBS::FGridCellBase::GetSizeY() const
{
	return GDss * Multiplier.Y;
}

float MBS::FGridCellBase::GetSizeZ() const
{
	return GDss * Multiplier.Z;
}

int32 MBS::FGridCellBase::SecX(float V) const
{
	/*UE_LOG(LogTemp, Log, TEXT("SecX=%.2f / (%.2f * %.2f) = %d (%.2f)"),
	V,
	UModularSectionResolution::DefaultSectionSize,
	Multiplier.X,
	FMath::RoundToInt(V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.X) : 0),
	V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.X) : 0);*/
	return FMath::RoundToInt(V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.X) : 0);
}

int32 MBS::FGridCellBase::SecY(float V) const
{
	/*UE_LOG(LogTemp, Log, TEXT("SecY=%.2f / (%.2f * %.2f) = %d (%.2f)"),
		V,
		UModularSectionResolution::DefaultSectionSize,
		Multiplier.Y,
		FMath::RoundToInt(V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.Y) : 0),
		V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.Y) : 0);*/
	return FMath::RoundToInt(V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.Y) : 0);
}

int32 MBS::FGridCellBase::SecZ(float V) const
{
	return V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * Multiplier.Z) : 0;
}

int32 MBS::FGridCellBase::Sec(float V, const float InMultiplierComponent)
{
	return V != 0 ? V / (UModularSectionResolution::DefaultSectionSize * InMultiplierComponent) : 0;
}

FVector MBS::FGridCellBase::Relative(const FVector V) const
{
	return V + RelativePivot;
	//return FTransform(V).GetRelativeTransform(FTransform(RelativePivot)).GetLocation();
	//return FTransform(RelativePivot).GetRelativeTransform(FTransform(V)).GetLocation();
}

float MBS::FGridCellBase::AddX(const float V) const
{
	return V + GDss * Multiplier.X;
}

float MBS::FGridCellBase::AddY(const float V) const
{
	return V + GDss * Multiplier.Y;
}

float MBS::FGridCellBase::AddZ(const float V) const
{
	return V + GDss * Multiplier.Z;
}

float MBS::FGridCellBase::MulX(const float V) const
{
	return V * (GDss * Multiplier.X);
}

float MBS::FGridCellBase::MulY(const float V) const
{
	return V * (GDss * Multiplier.Y);
}

float MBS::FGridCellBase::MulZ(const float V) const
{
	return V * (GDss * Multiplier.Z);
}

MBS::FGridCell2D::FGridCell2D(int32 XIndex, int32 YIndex)
{
	const FVector Min(MulX(XIndex), MulY(YIndex), 0.f);
	const FVector Max(AddX(Min.X), AddY(Min.Y), 0.f);
	Bounds = FBox(Min, Max);
}

MBS::FGridCell2D::FGridCell2D(int32 XIndex, int32 YIndex, const FVector InMultiplier)
	: FGridCellBase(InMultiplier)
{
	const FVector Min(MulX(XIndex), MulY(YIndex), 0.f);
	const FVector Max(AddX(Min.X), AddY(Min.Y), 0.f);
	Bounds = FBox(Min, Max);
}

MBS::FGridCell2D::FGridCell2D(int32 XIndex, int32 YIndex, const FTransform& RelativeTo)
	: FGridCell2D(XIndex, YIndex)
{
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridCell2D::FGridCell2D(int32 XIndex, int32 YIndex, const FTransform& RelativeTo, const FVector InMultiplier)
	: FGridCell2D(XIndex, YIndex, InMultiplier)
{
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridCell2D::FGridCell2D(const FBox& Bounds, const FVector InMultiplier)
	: FGridCellBase(InMultiplier)
{
	const FVector Min(Bounds.Min.GridSnap(GDss));
	const FVector Max(AddX(Min.X), AddY(Min.Y), 0.f);
	this->Bounds = FBox(Min, Max);
}

MBS::FGridCell2D::FGridCell2D(const FBox& Bounds)
	: FGridCell2D(Bounds, FVector::OneVector)
{
}

MBS::FGridCell2D::FGridCell2D(const FBox& Bounds, const FTransform& RelativeTo)
	: FGridCell2D(Bounds, RelativeTo, FVector::OneVector)
{
}

MBS::FGridCell2D::FGridCell2D(const FBox& Bounds, const FTransform& RelativeTo, const FVector InMultiplier)
	: FGridCellBase(InMultiplier)
{
	const FVector Min = FTransform(Bounds.Min).GetRelativeTransform(RelativeTo).GetLocation();
	const FVector Max(AddX(Min.X), AddY(Min.Y), 0.f);
	this->Bounds = FBox(Min, Max);
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

FIntVector MBS::FGridCell2D::GetIndexXYZ() const
{
	//const FVector Min = Relative(Bounds.Min);
	const FVector Min = Bounds.Min;
	return FIntVector(SecX(Min.X), SecY(Min.Y), 0);
}

FIntVector MBS::FGridCell3D::GetIndexXYZ() const
{
	//const FVector Min = Relative(Bounds.Min);
	const FVector Min = Bounds.Min;
	return FIntVector(SecX(Min.X), SecY(Min.Y), SecZ(Min.Z));
}

MBS::FGridCell3D::FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, bool bFlatten)
{
	const FVector Min(MulX(XIndex), MulY(YIndex), MulZ(ZIndex));
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	Bounds = FBox(Min, Max);
}

MBS::FGridCell3D::FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FVector InMultiplier, bool bFlatten)
	: FGridCellBase(InMultiplier)
{
	const FVector Min(MulX(XIndex), MulY(YIndex), MulZ(ZIndex));
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	Bounds = FBox(Min, Max);
}

MBS::FGridCell3D::FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FTransform& RelativeTo,
	const FVector InMultiplier, bool bFlatten)
	: FGridCell3D(XIndex, YIndex, ZIndex, InMultiplier, bFlatten)
{
	//const FVector Min = FTransform(Bounds.Min).GetRelativeTransform(RelativeTo).GetLocation();
	//const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	//this->Bounds = FBox(Min, Max);
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridCell3D::FGridCell3D(const FBox& Bounds, bool bFlatten)
	: FGridCell3D(Bounds, FVector::OneVector, bFlatten)
{
}

MBS::FGridCell3D::FGridCell3D(const FBox& Bounds, const FTransform& RelativeTo, bool bFlatten)
	: FGridCell3D(Bounds, RelativeTo, FVector::OneVector, bFlatten)
{
}

MBS::FGridCell3D::FGridCell3D(const FBox& Bounds, const FVector InMultiplier, bool bFlatten)
	: FGridCellBase(InMultiplier)
{
	const FVector Min(Bounds.Min.GridSnap(GDss));
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	this->Bounds = FBox(Min, Max);
}

MBS::FGridCell3D::FGridCell3D(const FBox& Bounds, const FTransform& RelativeTo, const FVector InMultiplier, bool bFlatten)
	: FGridCellBase(InMultiplier)
{
	const FVector Min = FTransform(Bounds.Min).GetRelativeTransform(RelativeTo).GetLocation();
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	this->Bounds = FBox(Min, Max);
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridCell3D::FGridCell3D(const FVector V, bool bFlatten)
	: FGridCell3D(V, FVector::OneVector, bFlatten)
{
}

MBS::FGridCell3D::FGridCell3D(const FVector V, const FVector InMultiplier, bool bFlatten)
	: FGridCellBase(InMultiplier)
{
	const FVector Min(V.GridSnap(GDss));
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	this->Bounds = FBox(Min, Max);
}

MBS::FGridCell3D::FGridCell3D(const FVector V, const FTransform& RelativeTo, bool bFlatten)
	: FGridCell3D(V, RelativeTo, FVector::OneVector, bFlatten)
{
}

MBS::FGridCell3D::FGridCell3D(const FVector V, const FTransform& RelativeTo, const FVector InMultiplier, bool bFlatten)
	: FGridCellBase(InMultiplier)
{
	const FVector Min = FTransform(V).GetRelativeTransform(RelativeTo).GetLocation();
	const FVector Max(AddX(Min.X), AddY(Min.Y), bFlatten ? Min.Z : AddZ(Min.Z));
	this->Bounds = FBox(Min, Max);
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridCell3D::FGridCell3D(int32 XIndex, int32 YIndex, int32 ZIndex, const FTransform& RelativeTo, bool bFlatten)
	: FGridCell3D(XIndex, YIndex, ZIndex, bFlatten)
{
	this->RelativePivot = RelativeTo.GetLocation();
	this->bRelative = true;
}

MBS::FGridMultiCell2D::FGridMultiCell2D(TArray<FGridCell2D> GridCells)
{
	BoundsBoxes.Reserve(GridCells.Num());
	for (const auto& Cell : GridCells)
	{
		BoundsBoxes.Add(Cell.GetBounds());
	}
}

MBS::FGridMultiCell2D::FGridMultiCell2D(const FBox& Bounds)
{
	const int32 CountX = FGridCellBase::Sec(Bounds.Max.X, 1);
	const int32 CountY = FGridCellBase::Sec(Bounds.Max.Y, 1);
	BoundsBoxes.Reserve(CountX * CountY);
	for (int32 x = 0; x < CountX; x++)
	{
		for (int32 y = 0; y < CountY; y++)
		{
			BoundsBoxes.Add(FGridCell2D(x, y).GetBounds());
		}
	}
}

MBS::FGridMultiCell2D::FGridMultiCell2D(const FBox& Bounds, const FTransform& RelativeTo)
{
	const FVector Max = FTransform(Bounds.Max).GetRelativeTransform(RelativeTo).GetLocation();
	const int32 CountX = FGridCellBase::Sec(Max.X, 1);
	const int32 CountY = FGridCellBase::Sec(Max.Y, 1);
	BoundsBoxes.Reserve(CountX * CountY);
	for (int32 x = 0; x < CountX; x++)
	{
		for (int32 y = 0; y < CountY; y++)
		{
			BoundsBoxes.Add(FGridCell2D(x, y, RelativeTo).GetBounds());
		}
	}
}

MBS::FGridMultiCell3D::FGridMultiCell3D(TArray<FGridCell3D> GridCells)
{
	BoundsBoxes.Reserve(GridCells.Num());
	for (const auto& Cell : GridCells)
	{
		BoundsBoxes.Add(Cell.GetBounds());
	}
}

MBS::FGridMultiCell3D::FGridMultiCell3D(const FBox& Bounds)
{
	// TODO: Implement multiplier
	const int32 CountX = FGridCellBase::Sec(Bounds.Max.X, 1);
	const int32 CountY = FGridCellBase::Sec(Bounds.Max.Y, 1);
	const int32 CountZ = FGridCellBase::Sec(Bounds.Max.Z, 1);
	BoundsBoxes.Reserve(CountX * CountY * CountZ);
	for (int32 x = 0; x < CountX; x++)
	{
		for (int32 y = 0; y < CountY; y++)
		{
			for (int32 z = 0; z < CountZ; z++)
			{
				BoundsBoxes.Add(FGridCell3D(x, y, z).GetBounds());
			}
		}
	}
}

MBS::FGridMultiCell3D::FGridMultiCell3D(const FBox& Bounds, const FTransform& RelativeTo)
{
	// TODO: Implement multiplier
	const FVector Max = FTransform(Bounds.Max).GetRelativeTransform(RelativeTo).GetLocation();
	const int32 CountX = FGridCellBase::Sec(Max.X, 1);
	const int32 CountY = FGridCellBase::Sec(Max.Y, 1);
	const int32 CountZ = FGridCellBase::Sec(Max.Z, 1);
	BoundsBoxes.Reserve(CountX * CountY * CountZ);
	for (int32 x = 0; x < CountX; x++)
	{
		for (int32 y = 0; y < CountY; y++)
		{
			for (int32 z = 0; z < CountZ; z++)
			{
				BoundsBoxes.Add(FGridCell3D(x, y, z, RelativeTo).GetBounds());
			}
		}
	}
}
