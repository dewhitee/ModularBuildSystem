// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSActorVisualizer.h"
#include "UObject/NoExportTypes.h"

class UMBSInteriorGenerator;
struct FMBSBoundsVisData;

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FHouseVisualizer : public FActorVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;

private:
	static void DrawBoundsBox(const FBox Box, const FMBSBoundsVisData& Bounds, FPrimitiveDrawInterface* PDI);
	
	static void DrawInteriorAvailableBox(FBox Box, const FMBSBoundsVisData& BasementBounds,
		const FMBSBoundsVisData& InteriorBounds, const UMBSInteriorGenerator* InteriorGenerator, 
		FPrimitiveDrawInterface* PDI);
};
}
