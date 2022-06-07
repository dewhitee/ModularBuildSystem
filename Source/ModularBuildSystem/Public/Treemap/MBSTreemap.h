// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MBSTreemap.generated.h"

class UModularLevelShape;

namespace MBS
{
struct FPartitionTreeNode;
}

struct FMBSTreemapPartition;
class UMBSTreemapTreePreset;

UENUM(BlueprintType)
enum class EMBSTreemapSizeValueMeasure : uint8
{
	Ratio,
	Section
};

USTRUCT(BlueprintType)
struct FMBSTreeNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Treemap)
	FName Name;

	UPROPERTY(EditAnywhere, Category=Treemap, meta=(Delta=0.01, EditCondition="Name != Root"))
	float Size;

	UPROPERTY(EditAnywhere, Category=Treemap)
	FName ParentName;

	UPROPERTY(EditAnywhere, Category=Treemap)
	int32 Level;

	FString ToString() const
	{
		return FString::Printf(TEXT("{Name=%s, Size=%.2f, ParentName=%s, Level=%d}"),
			*Name.ToString(), Size, *ParentName.ToString(), Level);
	}
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MODULARBUILDSYSTEM_API UMBSTreemap : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Treemap, meta=(TitleProperty="Name={Name}, Parent={ParentName}, Size={Size}"))
	TArray<FMBSTreeNode> Tree;

	/**
	 * Do snap tree node sizes to DefaultSectionSize.
	 * @see UModularSectionResolution
	 */
	UPROPERTY(EditAnywhere, Category=Treemap, AdvancedDisplay)
	bool bSnapToGrid = false;

	UPROPERTY(EditAnywhere, Category=Treemap)
	EMBSTreemapSizeValueMeasure Measure;

	UPROPERTY(EditAnywhere, Category=Treemap, meta=(InlineEditConditionToggle))
	bool bUseTreePreset = false;
	
	UPROPERTY(EditAnywhere, Category=Treemap, meta=(EditCondition=bUseTreePreset))
	TObjectPtr<UMBSTreemapTreePreset> TreePreset;

	UPROPERTY(VisibleAnywhere, Category=Treemap, AdvancedDisplay)
	FVector ScaleCoefficients = FVector::OneVector;

	/**
	 * Should only leaf nodes be returned upon creation of partitions?
	 * @note This will ensure that there is no overlaps between partitions in case when parent partition has the same
	 * bounds as the child partition.
	 */
	UPROPERTY(EditAnywhere, Category=Treemap)
	bool bOnlyLeafNodes = true;

	/**
	 * Should tree nodes be clamped in case they are exceeding the target bounds size (this will collapse them to fit)?
	 * @note Requires the Measure property be set to Section.
	 */
	UPROPERTY(EditAnywhere, Category=Treemap)
	bool bClampSizes = false;

	/**
	 * Should tree nodes sizes be stretched to fit target building shape?
	 * E.g. this will stretch them to fit the target shape if the current bounds size of a building actor is larger.
	 * @note Will work only if StretchCoefficients vector is not OneVector
	 * @note Requires the Measure property to be set to Section.
	 */
	UPROPERTY(EditAnywhere, Category=Treemap)
	bool bStretchSizes = false;

	/**
	 * Max target bounds value for clamp.
	 * @note Requires the Measure property to be set to Section.
	 */
	UPROPERTY(VisibleAnywhere, Category=Treemap, AdvancedDisplay)
	FVector SizeClampBounds;

public:
	TArray<FMBSTreeNode> GetTree() const;
	
	FVector GetScaleCoefficients() const { return ScaleCoefficients; }
	void SetScaleCoefficients(FVector Value) { ScaleCoefficients = Value; }
	
	EMBSTreemapSizeValueMeasure GetMeasure() const;
	
	FVector GetSizeClampBounds() const { return SizeClampBounds; }
	void SetSizeClampBounds(FVector InBounds) { SizeClampBounds = InBounds; }

	bool GetReturnOnlyLeafNodes() const { return bOnlyLeafNodes; }
	void SetReturnOnlyLeafNodes(bool bValue) { bOnlyLeafNodes = bValue; }
	
	/**
	 * Creates array of treemap partitions from the Blueprint-exposed tree node array.
	 * @param RootBox Bounds of a root node.
	 * @param RootTransform Transform of a root node.
	 * @param Nodes Array of nodes.
	 * @return Array of treemap partitions.
	 */
	TArray<FMBSTreemapPartition> CreatePartitions(const FBox& RootBox, const FTransform& RootTransform,
		TArray<FMBSTreeNode> Nodes);
	
	void AdjustPartitionsByShape(TArray<FMBSTreemapPartition>& Partitions, const UModularLevelShape* Shape,
		FIntPoint Bounds, const FTransform& FloorTransform);
	
protected:
	void CreatePartitionsRecursively(const FBox& RootBox, const FTransform& RootTransform,
		MBS::FPartitionTreeNode& Node, TArray<FMBSTreemapPartition>& Partitions);

	FBox CalculatePartitionBoundsHorizontally(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
		const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const;
	
	FBox CalculatePartitionBoundsVertically(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
		const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const;
	
	FMBSTreemapPartition CreateSinglePartition(const FBox& RootBox, const MBS::FPartitionTreeNode& PartitionTreeNode,
		const FMBSTreemapPartition* ParentPartition, const FMBSTreemapPartition* PreviousPartition) const;
	
	static TArray<FMBSTreemapPartition> GetLeafPartitions(const TArray<MBS::FPartitionTreeNode>& Nodes, const TArray<FMBSTreemapPartition>& Partitions);
	static TArray<MBS::FPartitionTreeNode> GetLeafPartitionNodes(const TArray<MBS::FPartitionTreeNode>& Nodes);
	static TArray<MBS::FPartitionTreeNode> GetLeafPartitionNodesRecursive(const MBS::FPartitionTreeNode& Node);
	
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

private:
	float GetNewAxisValue(TRange<float> OutputRange, float Size, const FBox& RootBox, EAxis::Type Axis, bool bRelative) const;
	float GetNewAxisValueFromRatio(TRange<float> OutputRange, float Size) const;
	float GetNewAxisValueFromSection(float Size, const FBox& RootBox, EAxis::Type Axis, bool bRelative) const;

	void ClampPartitionSizes(TArray<MBS::FPartitionTreeNode>& Nodes, EAxis::Type Axis) const;

	static EAxis::Type GetAxisFromNodeLevel(const int32 NodeLevel);
	
	static FMBSTreeNode* GetParentNode(const FMBSTreeNode& CurrentNode, const TArray<FMBSTreeNode>& Nodes);
	static FMBSTreemapPartition* GetParentPartition(const FName CurrentNodeParentName, const TArray<FMBSTreemapPartition>& Partitions);
};
