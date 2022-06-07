// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shape/ModularLevelShape.h"
#include "UObject/Interface.h"
#include "HouseLevelShapeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHouseLevelShapeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULARBUILDSYSTEM_API IHouseLevelShapeInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category=Shape)
	void ShapeHouseDefaultTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeHouseDefaultTransform_Implementation(const FMBSShapeTransformArgs& Args)
	{
		unimplemented();
	};
	
	UFUNCTION(BlueprintNativeEvent, Category=Shape)
	void ShapeHouseWallTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeHouseWallTransform_Implementation(const FMBSShapeTransformArgs& Args)
	{
		unimplemented();
	};

	UFUNCTION(BlueprintNativeEvent, Category=Shape)
	void ShapeHouseRoofTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeHouseRoofTransform_Implementation(const FMBSShapeTransformArgs& Args)
	{
		unimplemented();
	};

	UFUNCTION(BlueprintNativeEvent, Category=Shape)
	void ShapeHouseRooftopTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeHouseRooftopTransform_Implementation(const FMBSShapeTransformArgs& Args)
	{
		unimplemented();
	};

	UFUNCTION(BlueprintNativeEvent, Category=Shape)
	void ShapeHouseCornerTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeHouseCornerTransform_Implementation(const FMBSShapeTransformArgs& Args)
	{
		unimplemented();
	};
};
