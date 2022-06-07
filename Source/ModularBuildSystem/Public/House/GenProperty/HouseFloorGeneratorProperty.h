// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSContainers.h"
#include "MBSGeneratorProperty.h"
#include "HouseFloorGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Floor")
class MODULARBUILDSYSTEM_API UHouseFloorGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseFloorGeneratorProperty()
	{
		Data.DebugPropertyName = "House Floor";
		FloorHoles.DebugPropertyName = "House Floor Holes";
		FloorHoleDoor.Data.DebugPropertyName = "House Floor Hole Door";
	}
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled"))
	bool bFloorForEachLevel;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled", ClampMin = 1, ClampMax = 8))
	int32 MaxFloorCount = 1;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled"))
	float FloorZOffset = 0.f;

	// TODO: Add property to add an offset between each floor of a house
	// ...
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled"))
	bool bUniqueFirstFloor = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled && bUniqueFirstFloor"))
	int32 FirstFloorMeshIndex = 0;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Floor", meta=(EditCondition="bEnabled && bUniqueFirstFloor"))
	int32 OtherFloorMeshIndex = 0;

	// Floor hole
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FloorHoles", meta=(EditCondition="bEnabled"))
	bool bCanHaveFloorHoles;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FloorHoles", meta=(EditCondition="bCanHaveFloorHoles"))
	bool bWithFloorHoles;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FloorHoles", meta=(EditCondition="bCanHaveFloorHoles"))
	FMBSGeneratorPropertyData FloorHoles;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FloorHoles", meta=(EditCondition="bWithFloorHoles"))
	int32 FloorHoleMeshIndex = 0;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="FloorHoles", meta=(EditCondition="bWithFloorHoles"))
	TArray<FMBSLevelIdTransformPair> FloorHoleIdTransforms;

	// Floor hole door
	// TODO: Replace with UMBSProperty or FMBSGeneratorPropertyData?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FloorHoles", meta=(EditCondition="bWithFloorHoles"))
	FMBSGeneratorProperty_Old FloorHoleDoor;
};
