// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularBuildStats.h"
#include "ModularBuildSystem.h"
#include "Shape/ModularLevelShape.h"

void FModularBuildStats::Init(FModularBuildStats NewBuildStats)
{
	//if (!bInitialized)
	//{
	*this = NewBuildStats;
	bInitialized = true;
	//}
}

void FModularBuildStats::Clear()
{
	*this = {};
}

void FModularBuildStats::Update(const FModularBuildStats& InBuildStats)
{
	UE_LOG(LogMBS, Verbose, TEXT("Updating BuildStats, InBuildStats: .Bounds.X=%d, .Bounds.Y=%d, .MaxTotalCount=%d, .MaxTotalRows=%d, .MaxCountInRow=%d"),
		InBuildStats.Bounds.X, InBuildStats.Bounds.Y, InBuildStats.MaxTotalCount, InBuildStats.MaxTotalRows,
		InBuildStats.MaxCountInRow);

	if (Bounds.GetMin() <= 0)
	{
		Bounds = InBuildStats.Bounds;
	}

	if (MaxTotalCount < InBuildStats.MaxTotalCount)
	{
		MaxTotalCount = InBuildStats.MaxTotalCount;
	}

	if (MaxTotalRows < InBuildStats.MaxTotalRows)
	{
		MaxTotalRows = InBuildStats.MaxTotalRows;
	}

	if (MaxCountInRow < InBuildStats.MaxCountInRow)
	{
		MaxCountInRow = InBuildStats.MaxCountInRow;
	}

	UE_LOG(LogMBS, Verbose, TEXT("Build stats update result: Bounds.X=%d, Bounds.Y=%d, MaxTotalCount=%d, MaxTotalRows=%d, MaxCountInRow=%d"), 
		Bounds.X, Bounds.Y, MaxTotalCount, MaxTotalRows, MaxCountInRow);
}

void FModularBuildStats::OverrideBounds(FIntPoint NewBounds)
{
	UE_LOG(LogMBS, Verbose, TEXT("Overriding bounds... Bounds=%s, NewBounds=%s"),
		*Bounds.ToString(), *NewBounds.ToString());
	Bounds = NewBounds;
}

void FModularBuildStats::OverrideLevelCount(int32 InLevelCount)
{
	LevelCount = InLevelCount;
}

int32 FModularBuildStats::GetMaxSectionIndexX(const UModularLevelShape* Shape, int32 IndexY) const
{
	return Shape ? Shape->GetShapedMaxIndexX(Bounds, IndexY) : Bounds.X - 1;
}

int32 FModularBuildStats::GetMaxSectionIndexY(const UModularLevelShape* Shape, int32 IndexX) const
{
	return Shape ? Shape->GetShapedMaxIndexY(Bounds, IndexX) : Bounds.Y - 1; 
}

int32 FModularBuildStats::GetMinSectionIndexX(const UModularLevelShape* Shape, int32 IndexY) const
{
	return Shape ? Shape->GetShapedMinIndexX(Bounds, IndexY) : 0;
}

int32 FModularBuildStats::GetMinSectionIndexY(const UModularLevelShape* Shape, int32 IndexX) const
{
	return Shape ? Shape->GetShapedMinIndexY(Bounds, IndexX) : 0;
}

