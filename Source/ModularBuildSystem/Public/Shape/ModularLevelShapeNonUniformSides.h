// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "House/HouseLevelShapeInterface.h"
#include "MBSContainers.h"
#include "ModularLevelShape.h"
#include "ModularLevelShapeNonUniformSides.generated.h"

/**
 * Shape used for creation of buildings with non-uniform sides.
 */
UCLASS(meta = (DisplayName = "Non-uniform sides"))
class MODULARBUILDSYSTEM_API UModularLevelShapeNonUniformSides : public UModularLevelShape,
	public IHouseLevelShapeInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Shape)
	FMBSMeshListResolutionPair SmallSideWall;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Shape)
	FMBSMeshListResolutionPair SideRoof;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Shape)
	FMBSMeshListProperty SmallFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Shape)
	bool bUniqueFirstFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Shape, meta = (EditCondition = "bUniqueFirstFloor"))
	FMBSMeshListProperty FirstSmallFloor;
	
	virtual void ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args) override;
	virtual bool CanShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) const override;
	virtual void ShapeHouseDefaultTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseWallTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseRoofTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseRooftopTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseCornerTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual bool HasFrontIndicesInverted_Implementation() const override;
	virtual bool SkipsSections_Implementation() const override;
	virtual bool IsDataPrepared_Implementation() const override;

private:
	static void PlaceRemainingSection(const FMBSPlaceRemainingActorsArgs& Args, const int32 Index, const FVector Offset,
		UStaticMesh* Mesh, const int32 LevelIdForTransform);

	void ShapeRoofV1(const FMBSShapeTransformArgs& Args);
	void ShapeRoofV2(const FMBSShapeTransformArgs& Args);
};
