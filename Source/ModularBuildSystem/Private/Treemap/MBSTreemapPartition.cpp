// Fill out your copyright notice in the Description page of Project Settings.


#include "Treemap/MBSTreemapPartition.h"

#include "MBSGridCell.h"
#include "ModularBuildSystem.h"
#include "Treemap/MBSTreemap.h"

MBS::FPartitionTreeNode::FPartitionTreeNode(const TArray<FMBSTreeNode>& InNodes)
{
	if (InNodes.IsEmpty())
	{
		UE_LOG(LogMBSTreemap, Error, TEXT("Can't create partition tree from empty node array."));
		Name = "ERROR";
		Size = -1.f;
		Level = -1;
		ParentName = "ERROR_PARENT";
	}
	else
	{
		for (const FMBSTreeNode& Node : InNodes)
		{
			if (Node.Level == 0) // If root
			{
				Name = Node.Name;
				Size = Node.Size;
				Level = 0;
				ParentName = FName(NAME_None);
			}
			else if (Node.Level == 1) // All direct descendants (children) of root
			{
				// Add all partitions recursively using ctor
				MBS::FPartitionTreeNode Child(Node, InNodes, Name);
				Children.Add(Child);
			}
			else
			{
				// Do nothing with deeper level nodes - they will be handled inside another ctor
				break;
			}
		}
	}
}

MBS::FPartitionTreeNode::FPartitionTreeNode(const FMBSTreeNode& InNode, const TArray<FMBSTreeNode>& InNodes, const FName ParentName)
	: Name(InNode.Name), Size(InNode.Size), ParentName(ParentName), Level(InNode.Level)
{
	UE_LOG(LogMBSTreemap, Verbose, TEXT("Constructing new partition tree node: Name=%s, Size=%.2f, ParentName=%s, Level=%d"),
		*Name.ToString(), Size, *ParentName.ToString(), Level);

	// Find all nodes that are direct descendants (children) of passed InNode
	TArray<FMBSTreeNode> ChildrenNodes = InNodes.FilterByPredicate([=](const FMBSTreeNode& Node)
	{
		return Node.ParentName == Name;
	});

	if (!ChildrenNodes.IsEmpty())
	{
		for (auto& ChildNode : ChildrenNodes)
		{
			Children.Add(FPartitionTreeNode(ChildNode, InNodes, Name));
		}
	}
}

void MBS::FPartitionTreeNode::LogFull()
{
	UE_LOG(LogMBSTreemap, Log, TEXT("--- Log partition tree ---\n"));
	LogSingle("");
	UE_LOG(LogMBSTreemap, Log, TEXT("--- End log partition tree ---\n"));
}

void MBS::FPartitionTreeNode::LogSingle(const FString& Prefix)
{
	UE_LOG(LogMBSTreemap, Log, TEXT("%s-%s: Size=%.2f\n"), *Prefix, *Name.ToString(), Size);
	for (auto& Child : Children)
	{
		Child.LogSingle("\t" + Prefix);
	}
}

TArray<FMBSTreemapPartition> MBS::FTreemapPartitionDivider::DividePartitionToFitComplexShape(FMBSTreemapPartition* Partition,
	FPartitionTargetShape Shape)
{
	if (!Partition || !Shape.XY.IsEmpty())
	{
		return {};
	}

	const FVector Min = Partition->Bounds.Min;
	const FVector Max = Partition->Bounds.Max;

	const auto IsFit = [Min, Max](const FTwoVectors& Vectors)
	{
		const bool bMinFit = Vectors.v1.X <= Min.X && Vectors.v1.Y <= Min.Y;
		const bool bMaxFit = Vectors.v2.X >= Max.X && Vectors.v2.Y >= Max.Y;
		return bMinFit && bMaxFit;
	};
	
	// Check if already fit
	if (Shape.XY.Num() > 1)
	{
		TArray<FMBSTreemapPartition> OutPartition;
		
		for (const auto ShapeLine : Shape.XY)
		{
			if (!IsFit(ShapeLine))
			{
				// Create new partition that fits the current shape line
				
			}
		}
	}
	else if (IsFit(Shape.XY[0]))  // MinMax fit by X and Y
	{
		return { *Partition };
	}
	
	return {};
}

TArray<FMBSTreemapPartition> MBS::FTreemapPartitionDivider::DividePartitionToFitComplexShape(FMBSTreemapPartition* Partition,
	FPartitionTargetGridShape Shape)
{
	// TODO: implement
	unimplemented();
	
	if (!Partition)
	{
		return {};
	}

	return {};
}

FTwoVectors MBS::FTreemapPartitionDivider::GetDivisionLineLeft(const FGridCellBase& Cell, int32 Len)
{
	const FVector TopLeft = Cell.TopLeft();
	return { Cell.BottomLeft(), FVector(TopLeft.X + Cell.GetSizeX() * Len, TopLeft.Y, TopLeft.Z) };
}

FTwoVectors MBS::FTreemapPartitionDivider::GetDivisionLineLeft(const TArray<FGridCellBase>& Cells)
{
	// TODO: Implement
	unimplemented();
	FVector V1;
	for (auto& Cell : Cells)
	{
		GetDivisionLineLeft(Cell);
	}
	return {};
}

FTwoVectors MBS::FTreemapPartitionDivider::GetDivisionLineRight(const FGridCellBase& Cell, int32 Len)
{
	const FVector TopRight = Cell.TopRight();
	return { Cell.BottomRight(), FVector(TopRight.X + Cell.GetSizeX() * Len, TopRight.Y, TopRight.Z) };
}

FTwoVectors MBS::FTreemapPartitionDivider::GetDivisionLineBottom(const FGridCellBase& Cell, int32 Len)
{
	const FVector BottomRight = Cell.BottomRight();
	return { Cell.BottomLeft(), FVector(BottomRight.X, BottomRight.Y + Cell.GetSizeY() * Len, BottomRight.Z) };
}

FTwoVectors MBS::FTreemapPartitionDivider::GetDivisionLineTop(const FGridCellBase& Cell, int32 Len)
{
	const FVector TopRight = Cell.TopRight();
	return { Cell.TopLeft(), FVector(TopRight.X, TopRight.Y + Cell.GetSizeY() * Len, TopRight.Z) };
}

TPair<FBox, FBox> MBS::FTreemapPartitionDivider::Divide(const FBox& Box, const FTwoVectors& DivisionLine)
{
	return TPair<FBox, FBox>(FBox(Box.Min, DivisionLine.v2), FBox(DivisionLine.v1, Box.Max));
}

TPair<FMBSTreemapPartition, FMBSTreemapPartition> MBS::FTreemapPartitionDivider::Divide(
	const FMBSTreemapPartition& Partition, const FTwoVectors& DivisionLine, bool bKeepZ)
{
	const FString Name1(Partition.Name.ToString() + "1");
	const FString Name2(Partition.Name.ToString() + "2");
	auto Boxes = Divide(Partition.Bounds, DivisionLine);
	
	if (bKeepZ)
	{
		Boxes.Key.Min.Z		= Partition.Bounds.Min.Z;
		Boxes.Key.Max.Z		= Partition.Bounds.Max.Z;
		Boxes.Value.Min.Z	= Partition.Bounds.Min.Z;
		Boxes.Value.Max.Z	= Partition.Bounds.Max.Z;
	}
	
	return TPair<FMBSTreemapPartition, FMBSTreemapPartition>(
		FMBSTreemapPartition(FName(Name1), Boxes.Key),
		FMBSTreemapPartition(FName(Name2), Boxes.Value));
}
