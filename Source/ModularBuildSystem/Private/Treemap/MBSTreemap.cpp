// Fill out your copyright notice in the Description page of Project Settings.


#include "Treemap/MBSTreemap.h"

#include "MBSGridCell.h"
#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"
#include "Algo/Count.h"
#include "Shape/ModularLevelShape.h"
#include "Treemap/MBSTreemapPartition.h"
#include "Treemap/MBSTreemapTreePreset.h"

TArray<FMBSTreeNode> UMBSTreemap::GetTree() const
{
	return bUseTreePreset && TreePreset ? TreePreset->Tree : Tree;
}

EMBSTreemapSizeValueMeasure UMBSTreemap::GetMeasure() const
{
	return bUseTreePreset && TreePreset ? TreePreset->Measure : Measure;
}

TArray<FMBSTreemapPartition> UMBSTreemap::CreatePartitions(const FBox& RootBox, const FTransform& RootTransform,
	TArray<FMBSTreeNode> Nodes)
{
	TArray<FMBSTreemapPartition> OutPartitions;

	if (Nodes.IsEmpty())
	{
		UE_LOG(LogMBSTreemap, Error, TEXT("%s: Can't create partitions out of empty nodes array."), *GetName());
		return OutPartitions;
	}

	// Sort nodes in ascending order by Level property
	Nodes.Sort([&](const FMBSTreeNode& Node1, const FMBSTreeNode& Node2)
	{
		return Node1.Level < Node2.Level;
	});

	// Create partition tree from node array
	MBS::FPartitionTreeNode Root(Nodes);
	Root.LogFull();
	
	// Save relative to the RootTransform root box translation
	const FTransform MinRelativeTransform = FTransform(RootBox.Min).GetRelativeTransform(RootTransform);
	const FTransform MaxRelativeTransform = FTransform(RootBox.Max).GetRelativeTransform(RootTransform);
	const FBox RelativeRootBox(MinRelativeTransform.GetLocation(), MaxRelativeTransform.GetLocation());

	// Save the max size from the Root node
	const float MaxSize = Nodes[0].Size;
	UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: MaxSize=%.2f\n, RootTransform.GetLocation()=%s\n, RootBox=%s\n, RootBox.GetSize()=%s\n, RelativeRootBox=%s"),
		*GetName(),
		MaxSize,
		*RootTransform.GetLocation().ToCompactString(),
		*RootBox.ToString(),
		*RootBox.GetSize().ToCompactString(),
		*RelativeRootBox.ToString());

	// Create partitions for each node in a sorted array
	UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: Node=%s"), *GetName(), *Root.ToString());
	OutPartitions.Add(CreateSinglePartition(RootBox, Root, nullptr, nullptr));
	CreatePartitionsRecursively(RootBox, RootTransform, Root, OutPartitions);
	
	if (bOnlyLeafNodes)
	{
		return GetLeafPartitions(GetLeafPartitionNodesRecursive(Root), OutPartitions);
	}
	
	return OutPartitions;
}

void UMBSTreemap::CreatePartitionsRecursively(const FBox& RootBox, const FTransform& RootTransform,
	MBS::FPartitionTreeNode& Node, TArray<FMBSTreemapPartition>& Partitions)
{
	UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: CreatePartitionsRecursively Node.Name=%s"), *GetName(), *Node.Name.ToString());

	// TODO: Save previous partition (on the same level)
	const FMBSTreemapPartition* Previous = nullptr;
	
	if (bClampSizes)
	{
		ClampPartitionSizes(Node.Children, GetAxisFromNodeLevel(Node.Level));
	}
	
	for (MBS::FPartitionTreeNode& Child : Node.Children)
	{
		const FMBSTreemapPartition* ParentPartition = GetParentPartition(Child.ParentName, Partitions);
		if (ParentPartition)
		{
			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Found parent partition %s"), *GetName(), *ParentPartition->ToString());
			// Get the min and max location of parent partition if we found one
		}
		else
		{
			UE_LOG(LogMBSTreemap, Error, TEXT("%s: Parent not found for node=%s"), *GetName(), *Child.ToString());
		}
		
		Partitions.Add(CreateSinglePartition(RootBox, Child, ParentPartition, Previous));
		Previous = &Partitions.Last();
		
		CreatePartitionsRecursively(RootBox, RootTransform, Child, Partitions);
	}
}

FBox UMBSTreemap::CalculatePartitionBoundsHorizontally(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
	const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const
{
	if (ParentPartition)
	{
		// TODO: Implement for Section measure
		const FBox ParentBox = ParentPartition->Bounds;

		if (PreviousPartition)
		{
			// Adjust MaxY using PreviousPartition
			const float YSize = GetNewAxisValue(
				TRange<float>(0.f, ParentBox.Max.Y - ParentBox.Min.Y), PartitionTreeNode.Size, RootBox, EAxis::Y, false);

			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: YSize=%.2f"), *GetName(), YSize);
			return FBox(
				FVector(ParentBox.Min.X, PreviousPartition->Bounds.Max.Y, ParentBox.Min.Z),
				FVector(ParentBox.Max.X, PreviousPartition->Bounds.Max.Y + YSize, ParentBox.Min.Z));
		}
		else
		{
			const float MaxY = GetNewAxisValue(
				TRange<float>(ParentBox.Min.Y, ParentBox.Max.Y), PartitionTreeNode.Size, RootBox, EAxis::Y, true);

			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: \n\tMaxY from ParentBox=%.2f \n\tParentBox=%s, \n\tSize=%.2f"),
				*GetName(), MaxY, *ParentBox.ToString(), PartitionTreeNode.Size);

			return FBox(ParentBox.Min, FVector(ParentBox.Max.X, MaxY, ParentBox.Min.Z));
		}
	}
	else // If no parent partition found, e.g. if current node is Root node
	{
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: \n\tRootBox=%s, \n\tSize=%.2f"),
			*GetName(), *RootBox.ToString(), PartitionTreeNode.Size);
		
		const FVector CurrentMaxLocation(RootBox.Max.X, RootBox.Max.Y, RootBox.Min.Z);
		return FBox(RootBox.Min, CurrentMaxLocation);
	}
}

FBox UMBSTreemap::CalculatePartitionBoundsVertically(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
	const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const
{
	if (ParentPartition)
	{
		// TODO: Implement for Section measure
		const FBox ParentBox = ParentPartition->Bounds;

		if (PreviousPartition)
		{
			// Adjust MaxX using PreviousPartition
			const float XSize = GetNewAxisValue(
				TRange<float>(0.f, ParentBox.Max.X - ParentBox.Min.X), PartitionTreeNode.Size, RootBox, EAxis::X, false);

			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: XSize=%.2f"), *GetName(), XSize);
			return FBox(
				FVector(PreviousPartition->Bounds.Max.X, ParentBox.Min.Y, ParentBox.Min.Z),
				FVector(PreviousPartition->Bounds.Max.X + XSize, ParentBox.Max.Y, ParentBox.Min.Z));
		}
		
		const float MaxX = GetNewAxisValue(
			TRange<float>(ParentBox.Min.X, ParentBox.Max.X), PartitionTreeNode.Size, RootBox, EAxis::X, true);

		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: \n\tMaxX from ParentBox=%.2f \n\tParentBox=%s, \n\tSize=%.2f"),
			*GetName(), MaxX, *ParentBox.ToString(), PartitionTreeNode.Size);
			
		return FBox(ParentBox.Min, FVector(MaxX, ParentBox.Max.Y, ParentBox.Min.Z));
	}
	
	// If no parent partition found, e.g. if current node is Root node
	UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: (ROOT NODE) \n\tRootBox=%s, \n\tSize=%.2f"),
		*GetName(), *RootBox.ToString(), PartitionTreeNode.Size);
		
	const FVector CurrentMaxLocation(RootBox.Max.X, RootBox.Max.Y, RootBox.Min.Z);
	return FBox(RootBox.Min, CurrentMaxLocation);
}

FMBSTreemapPartition UMBSTreemap::CreateSinglePartition(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
	const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const
{
	FMBSTreemapPartition OutPartition;
	OutPartition.Name = PartitionTreeNode.Name;

	UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: --- Creating new partition for Node: %s ---"),
		*GetName(), *PartitionTreeNode.Name.ToString());

	if (PreviousPartition)
	{
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: PreviousPartition=%s"), *GetName(), *PreviousPartition->ToString());
	}
	else
	{
		UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: PreviousPartition was nullptr for Node=%s"), *GetName(), *PartitionTreeNode.ToString());
	}
	
	if (PartitionTreeNode.Level % 2 == 0) // Each even level node and partition should be split and calculated vertically
	{
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Node.Level=%d => Calculating vertically"), *GetName(), PartitionTreeNode.Level);
		OutPartition.Bounds = CalculatePartitionBoundsVertically(RootBox, PartitionTreeNode, ParentPartition, PreviousPartition);
	}
	else // Each odd level node and partition should be split and calculated horizontally
	{
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Node.Level=%d => Calculating horizontally"), *GetName(), PartitionTreeNode.Level);
		OutPartition.Bounds = CalculatePartitionBoundsHorizontally(RootBox, PartitionTreeNode, ParentPartition, PreviousPartition);
	}

	if (bSnapToGrid)
	{
		UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: Snapping to resolution..."), *GetName());
		OutPartition.Bounds.Min = OutPartition.Bounds.Min.GridSnap(UModularSectionResolution::DefaultSectionSize);
		OutPartition.Bounds.Max = OutPartition.Bounds.Max.GridSnap(UModularSectionResolution::DefaultSectionSize);
	}

	UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Created partition for Node %s. OutPartition=%s"),
		*GetName(),
		*PartitionTreeNode.Name.ToString(),
		*OutPartition.ToString());

	return OutPartition;
}

TArray<FMBSTreemapPartition> UMBSTreemap::GetLeafPartitions(const TArray<MBS::FPartitionTreeNode>& Nodes,
	const TArray<FMBSTreemapPartition>& Partitions)
{
	const TArray<MBS::FPartitionTreeNode> Leaves = GetLeafPartitionNodes(Nodes);
	return Partitions.FilterByPredicate([Leaves](const FMBSTreemapPartition& Partition)
	{
		return Leaves.ContainsByPredicate([Partition](const MBS::FPartitionTreeNode& Node)
		{
			return Node.Name == Partition.Name;
		});
	});
}

TArray<MBS::FPartitionTreeNode> UMBSTreemap::GetLeafPartitionNodes(const TArray<MBS::FPartitionTreeNode>& Nodes)
{
	return Nodes.FilterByPredicate([](const MBS::FPartitionTreeNode& Node) -> bool
	{
		return Node.Children.IsEmpty();
	});
}

TArray<MBS::FPartitionTreeNode> UMBSTreemap::GetLeafPartitionNodesRecursive(const MBS::FPartitionTreeNode& Node)
{
	TArray<MBS::FPartitionTreeNode> OutNodes;
	if (Node.Children.IsEmpty())
	{
		OutNodes.Add(Node);
	}
	else
	{
		for (auto Child : Node.Children)
		{
			OutNodes.Append(GetLeafPartitionNodesRecursive(Child));
		}
	}
	return OutNodes;
}

void UMBSTreemap::AdjustPartitionsByShape(TArray<FMBSTreemapPartition>& Partitions, const UModularLevelShape* Shape,
	FIntPoint Bounds, const FTransform& FloorTransform)
{
	if (!Shape)
	{
		// Do nothing if shape is not provided.
		return;
	}
	
	// TODO: Implement
	// Find out which sections are missing (currently will only work with L-Shapes)
	
	// TODO: Find out which IndexY value should be put here
	//const int32 MaxIndexX = Shape->GetShapedMaxIndexX(Bounds, 0);

	// Get division line
	int32 MaxIndexX = Bounds.X;
	for (int32 i = 0; i < Bounds.Y; i++)
	{
		const int32 CurrentMaxIndexX = Shape->GetShapedMaxIndexX(Bounds.X, i);
		if (CurrentMaxIndexX < Bounds.X) // If there is a shift at current index
		{
			MaxIndexX = CurrentMaxIndexX;
			break;
		}
	}

	int32 MaxIndexY = Bounds.Y;
	for (int32 i = 0; i < Bounds.Y; i++)
	{
		const int32 CurrentMaxIndexY = Shape->GetShapedMaxIndexY(Bounds.Y, i);
		if (CurrentMaxIndexY < Bounds.Y)
		{
			MaxIndexY = CurrentMaxIndexY;
			break;
		}
	}

	// Find cell that will be used to create a division line
	// TODO: Test this. Working atm but need more test cases to be sure.
	// TODO: Currently, this can only work for L-Shape custom shape. In future, there should be a universal algo
	// that could handle this logic for any possible shape.
	//
	// Test cases:
	// Should be:				 ... Bounds=(X=8, Y=10), LevelCount=5
	//	(1,9,1) when MaxIndexX=7, MaxIndexY=9, Depth=1	/ Currently=(7,9,1)
	//  (2,8,1) when MaxIndexX=6, MaxIndexY=8, Depth=2	/ Currently=(6,8,1)
	//	(3,7,1) when MaxIndexX=5, MaxIndexY=7, Depth=3	/ Currently=(5,7,1)
	//	(4,6,1) when MaxIndexX=4, MaxIndexY=6, Depth=4	/ Currently=(4,6,1) => Right one
	//		e.t.c...
	const int32 XDiff = Bounds.X - MaxIndexX;
	const MBS::FGridCell3D DivCell(XDiff, MaxIndexY, 1, FloorTransform, ScaleCoefficients, true);
	
	// TODO: Literal 3 only works with Depth=4  => replace with variable
	FTwoVectors VerticalDivLine = MBS::FTreemapPartitionDivider::GetDivisionLineLeft(DivCell, /*3*/MaxIndexX - 1); // TODO: Do not use literals

	// Change div line to Absolute
	VerticalDivLine.v1 += FloorTransform.GetLocation();
	VerticalDivLine.v2 += FloorTransform.GetLocation();
	
	FTwoVectors HorizontalDivLine = MBS::FTreemapPartitionDivider::GetDivisionLineBottom(DivCell, /*3*/MaxIndexX); // TODO: Do not use literals
	HorizontalDivLine.v1 += FloorTransform.GetLocation();
	HorizontalDivLine.v2 += FloorTransform.GetLocation();
	
	UE_LOG(LogMBSTreemap, Verbose, TEXT("%s: \n\tMaxIndexX=%d, \n\tMaxIndexY=%d, \n\tDivCell=%s, \n\tVerticalDivLine=%s, \n\tHorizontalDivLine=%s"),
		*GetName(), MaxIndexX, MaxIndexY, *DivCell.ToString(), *VerticalDivLine.ToString(), *HorizontalDivLine.ToString());
	
	// Find out which partitions should be divided and divide them in place
	const int32 Count = Partitions.Num();
	for (int32 i = 0; i < Count; i++)
	{
		MBS::FGridCell3D Cell(Partitions[i].Bounds, FloorTransform, ScaleCoefficients);
		MBS::FGridCell3D CellMin(Partitions[i].Bounds.Min, FloorTransform, ScaleCoefficients, true);
		MBS::FGridCell3D CellMax(Partitions[i].Bounds.Max, FloorTransform, ScaleCoefficients, true);
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: [i=%d] Partition=%s, Cell=%s"), *GetName(), i, *Partitions[i].ToString(), *Cell.ToString());
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: [i=%d] Partition=%s, Cell.Min=%s"), *GetName(), i, *Partitions[i].ToString(), *CellMin.ToString());
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: [i=%d] Partition=%s, Cell.Max=%s"), *GetName(), i, *Partitions[i].ToString(), *CellMax.ToString());

		// Divide horizontally if division line is in middle of current partition
		if (CellMin.GetIndexXYZ().Y < MaxIndexY && CellMax.GetIndexXYZ().Y > MaxIndexY)
		{
			// TODO: Implement
			TPair<FMBSTreemapPartition, FMBSTreemapPartition> Divided =
				MBS::FTreemapPartitionDivider::Divide(Partitions[i], VerticalDivLine, true);
			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Divided %s in two:\n\t%s\n\t%s\n\tUsing this vertical divider:\n\t%s"),
				*GetName(),
				*Partitions[i].ToString(),
				*Divided.Key.ToString(),
				*Divided.Value.ToString(),
				*VerticalDivLine.ToString());
			
			Partitions[i] = Divided.Key;
			Partitions.Add(Divided.Value);
		}
		// TODO: Also divide, if division line is horizontal and passes through the current partition
		// Divide if division line is right on the left border of a partition
		else if (CellMin.GetIndexXYZ().Y >= MaxIndexY)
		{
			TPair<FMBSTreemapPartition, FMBSTreemapPartition> Divided =
				MBS::FTreemapPartitionDivider::Divide(Partitions[i], HorizontalDivLine, true);
			UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: Divided %s in two:\n\t%s\n\t%s\n\tUsing this horizontal divider:\n\t%s"),
				*GetName(),
				*Partitions[i].ToString(),
				*Divided.Key.ToString(),
				*Divided.Value.ToString(),
				*HorizontalDivLine.ToString());
			
			Partitions[i] = Divided.Value;
		}
	}
}

#if WITH_EDITOR
bool UMBSTreemap::CanEditChange(const FProperty* InProperty) const
{
	const bool bResult = Super::CanEditChange(InProperty);
	const FName PropertyName = InProperty->GetFName();
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMBSTreemap, bClampSizes)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UMBSTreemap, bStretchSizes)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UMBSTreemap, SizeClampBounds))
	{
		return bResult && GetMeasure() == EMBSTreemapSizeValueMeasure::Section;
	}

	return bResult;
}
#endif

float UMBSTreemap::GetNewAxisValue(TRange<float> OutputRange, float Size, const FBox& RootBox, EAxis::Type Axis, bool bRelative) const
{
	switch (GetMeasure())
	{
	case EMBSTreemapSizeValueMeasure::Ratio:	return GetNewAxisValueFromRatio(OutputRange, Size);
	case EMBSTreemapSizeValueMeasure::Section:	return GetNewAxisValueFromSection(Size, RootBox, Axis, bRelative);
	default:									return -1.f;
	}
}

float UMBSTreemap::GetNewAxisValueFromRatio(TRange<float> OutputRange, float Size) const
{
	return FMath::GetMappedRangeValueClamped(TRange<float>(0.f, 1.f), OutputRange, Size);
}

float UMBSTreemap::GetNewAxisValueFromSection(float Size, const FBox& RootBox, EAxis::Type Axis, bool bRelative) const
{
	if (Axis == EAxis::None)
	{
		// Return relative
		return UModularSectionResolution::DefaultSectionSize * Size;
	}
	
	// Return world
	UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: GetNewAxisValueFromSection: \n\tScaleCoefficients=%s, \n\tScaleCoefficient.GetComponentForAxis()=%.2f, \n\tAxis=%s"), *GetName(),
		*ScaleCoefficients.ToString(), ScaleCoefficients.GetComponentForAxis(Axis), *UEnum::GetValueAsString(Axis));

	if (bRelative)
	{
		FVector Location(RootBox.Min);
		Location.SetComponentForAxis(Axis,
			UModularSectionResolution::DefaultSectionSize * Size * ScaleCoefficients.GetComponentForAxis(Axis) * -1.f);
			
		return FTransform(RootBox.Min)
		       .GetRelativeTransform(FTransform(Location))
		       .GetLocation()
		       .GetComponentForAxis(Axis);
	}
		
	return UModularSectionResolution::DefaultSectionSize * Size * ScaleCoefficients.GetComponentForAxis(Axis);
}

void UMBSTreemap::ClampPartitionSizes(TArray<MBS::FPartitionTreeNode>& Nodes, EAxis::Type Axis) const
{
	UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: ClampPartitionSizes: Nodes.Num()=%d, Axis=%s"),
		*GetName(), Nodes.Num(), *UEnum::GetValueAsString(Axis));
	
	if (Nodes.Num() <= 0)
	{
		return;
	}
	
	const float AxisMax = FVector(SizeClampBounds.Y, SizeClampBounds.X, SizeClampBounds.Z).GetComponentForAxis(Axis);
	float TotalSize = 0.f;
	
	for (auto& Node : Nodes)
	{
		TotalSize += Node.Size;
		const float Diff = TotalSize - AxisMax;
		UE_LOG(LogMBSTreemap, VeryVerbose, TEXT("%s: \tClamping... Node=%s, Size=%.2f, TotalSize=%.2f, AxisMax=%.2f, Diff=%.2f-%.2f=%.2f"), *GetName(),
			*Node.Name.ToString(), Node.Size, TotalSize, AxisMax, TotalSize, AxisMax, Diff);
			
		if (Diff > 0.f)
		{
			Node.Size -= Diff;
		}
	}

	if (bStretchSizes)
	{
		if (TotalSize < AxisMax)
		{
			Nodes.Last().Size += AxisMax - TotalSize;
		}
	}
}

EAxis::Type UMBSTreemap::GetAxisFromNodeLevel(const int32 NodeLevel)
{
	return NodeLevel % 2 == 0 ? EAxis::X : EAxis::Y;
}

FMBSTreeNode* UMBSTreemap::GetParentNode(const FMBSTreeNode& CurrentNode, const TArray<FMBSTreeNode>& Nodes)
{
	if (CurrentNode.ParentName.IsNone())
	{
		return nullptr;
	}
	
	const FMBSTreeNode* FoundNode = Nodes.FindByPredicate([CurrentNode](const FMBSTreeNode& Node)
	{
		return Node.ParentName == CurrentNode.ParentName;
	});
	return const_cast<FMBSTreeNode*>(FoundNode);
}

FMBSTreemapPartition* UMBSTreemap::GetParentPartition(const FName CurrentNodeParentName, const TArray<FMBSTreemapPartition>& Partitions)
{
	const FMBSTreemapPartition* FoundPartition = Partitions.FindByPredicate([CurrentNodeParentName](const FMBSTreemapPartition& Partition)
	{
		return Partition.Name == CurrentNodeParentName;
	});
	return const_cast<FMBSTreemapPartition*>(FoundPartition);
}
