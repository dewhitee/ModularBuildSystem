// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSActorVisualizationComponent.h"
#include "HouseVisualizationComponent.generated.h"

namespace MBS
{
class FHouseVisualizer;
}

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARBUILDSYSTEMEDITOR_API UHouseVisualizationComponent : public UMBSActorVisualizationComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category="Visualizer|Basement")
	bool bShowBasementBounds = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Floors")
	bool bShowFloorBounds = false;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Walls")
	bool bShowWallBounds = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Corners")
	bool bShowCornerBounds = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Roof")
	bool bShowRoofBounds = false;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Rooftop")
	bool bShowRooftopBounds = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Basement", meta=(EditCondition=bShowBasementBounds))
	FMBSBoundsVisData BasementBounds;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Floors", meta=(EditCondition=bShowFloorBounds))
	FMBSBoundsVisData FloorBounds;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Walls", meta=(EditCondition=bShowWallBounds))
	FMBSBoundsVisData WallBounds;

	UPROPERTY(EditAnywhere, Category="Visualizer|Corner", meta=(EditCondition=bShowCornerBounds))
	FMBSBoundsVisData CornerBounds;

	UPROPERTY(EditAnywhere, Category="Visualizer|Roof", meta=(EditCondition=bShowRoofBounds))
	FMBSBoundsVisData RoofBounds;

	UPROPERTY(EditAnywhere, Category="Visualizer|Rooftop", meta=(EditCondition=bShowRooftopBounds))
	FMBSBoundsVisData RooftopBounds;
	
	friend class MBS::FHouseVisualizer;
};
