// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "House/HouseLevelShapeInterface.h"
#include "ModularLevelShape.h"
#include "ModularLevelShapeL.generated.h"

/**
 * Modular level L-Shape.
 */
UCLASS(meta = (DisplayName = "L-Shape"))
class MODULARBUILDSYSTEM_API UModularLevelShapeL final : public UModularLevelShape, public IHouseLevelShapeInterface
{
	GENERATED_BODY()
	
public:
	/**
	 * @brief Larger depth value is - the thinner the building shape is.
	 * @note Value should be less than bounds of the building.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = 1, ClampMax = 16))
	uint8 Depth = 1;

	/**
	 * @brief Mirrors shape rotation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
	bool bUpper = false;

	/**
	 * @brief Static mesh used for remaining roof static mesh actor. If not provided - default roof mesh will be used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
	TObjectPtr<UStaticMesh> CornerRoofStaticMesh;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// UModularLevelShape
	virtual void ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args) override;
	virtual void UpdateSectionInitializer_Implementation(const FMBSUpdateSectionInitializerArgs& Args) override;
	virtual bool CanShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) const override;
	virtual int32 GetShapedMaxIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const override;
	virtual int32 GetShapedMaxIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const override;
	virtual int32 GetShapedMinIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const override;
	virtual int32 GetShapedMinIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const override;

	// IHouseLevelShapeInterface
	virtual void ShapeHouseWallTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseCornerTransform_Implementation(const FMBSShapeTransformArgs& Args) override;
	virtual void ShapeHouseRooftopTransform_Implementation(const FMBSShapeTransformArgs& Args) override;

private:
	void AdjustDefaultTransform(const FMBSShapeTransformArgs& Args) const;
	void AdjustWallTransformGen2(const FMBSShapeTransformArgs& Args);
	void AdjustWallTransform(const FMBSShapeTransformArgs& Args);
	
};
