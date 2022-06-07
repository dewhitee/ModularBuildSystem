// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSTreemapPartition.generated.h"

struct FMBSTreemapPartition;
struct FMBSTreeNode;

namespace MBS
{
class FGridCellBase;
class FGridCell2D;
	
struct FPartitionTreeNode
{
	FPartitionTreeNode(const TArray<FMBSTreeNode>& InNodes);
	FPartitionTreeNode(const FMBSTreeNode& InNode, const TArray<FMBSTreeNode>& InNodes, const FName ParentName);

	FName Name;
	float Size;
	FName ParentName;
	int32 Level;
	TArray<FPartitionTreeNode> Children;

	void LogFull();

	FString ToString() const
	{
		return FString::Printf(TEXT("{Name=%s, Size=%.2f, ParentName=%s, Level=%d}"),
			*Name.ToString(), Size, *ParentName.ToString(), Level);
	}

private:	
	void LogSingle(const FString& Prefix);
};

struct FPartitionTargetGridShape
{
	TArray<FIntPoint> XY;
	FPartitionTargetGridShape(const TArray<FIntPoint>& XY) : XY(XY) {}
};

struct FPartitionTargetShape
{
	TArray<FTwoVectors> XY;
	FPartitionTargetShape(const TArray<FTwoVectors>& XY) : XY(XY) {}
};

struct FTreemapPartitionDivider
{
	static TPair<FBox, FBox> Divide(const FBox& Box, const FTwoVectors& DivisionLine);
	static TPair<FMBSTreemapPartition, FMBSTreemapPartition> Divide(const FMBSTreemapPartition& Partition, const FTwoVectors& DivisionLine, bool bKeepZ);
	TArray<FMBSTreemapPartition> DividePartitionToFitComplexShape(FMBSTreemapPartition* Partition, FPartitionTargetShape Shape);
	TArray<FMBSTreemapPartition> DividePartitionToFitComplexShape(FMBSTreemapPartition* Partition, FPartitionTargetGridShape Shape);
	static FTwoVectors GetDivisionLineLeft(const FGridCellBase& Cell, int32 Len = 0);
	static FTwoVectors GetDivisionLineLeft(const TArray<FGridCellBase>& Cells);
	static FTwoVectors GetDivisionLineRight(const FGridCellBase& Cell, int32 Len = 0);
	static FTwoVectors GetDivisionLineBottom(const FGridCellBase& Cell, int32 Len = 0);
	static FTwoVectors GetDivisionLineTop(const FGridCellBase& Cell, int32 Len = 0);
};
	
}

/**
 * Single partition of a treemap.
 */
USTRUCT(BlueprintType)
struct FMBSTreemapPartition
{
	GENERATED_BODY()

	FMBSTreemapPartition() {}
	FMBSTreemapPartition(const FName InName, const FBox& InBounds)
		: Name(InName)
		, Bounds(InBounds) {}

	UPROPERTY(EditAnywhere, Category=Treemap)
	FName Name;

	UPROPERTY(EditAnywhere, Category=Treemap)
	FBox Bounds;

	FString ToString() const
	{
		return FString::Printf(TEXT("{Name=%s, Bounds=%s}"), *Name.ToString(), *Bounds.ToString());
	}
};
