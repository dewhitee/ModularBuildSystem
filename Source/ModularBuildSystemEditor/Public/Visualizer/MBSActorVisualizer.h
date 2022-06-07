// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "UObject/NoExportTypes.h"

class AModularBuildSystemActor;
struct FModularLevel;
struct FMBSBoundsVisData;
struct FMBSVisDataBase;
struct FMBSDimensionsVisData;

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FActorVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
		const FSceneView* View, FCanvas* Canvas) override;

protected:
	static void DrawSingleModularSection(const FBox Box, const FMBSBoundsVisData& Bounds,
		FPrimitiveDrawInterface* PDI);

	static void DrawSingleModularLevel(const AModularBuildSystemActor* BS, const FModularLevel& InLevel,
		const FMBSBoundsVisData& Bounds, FPrimitiveDrawInterface* PDI);
	
private:
	static void DrawDiamond(FVector Front, FVector Right, FVector Back, FVector Left,
		FMBSVisDataBase VisData, FPrimitiveDrawInterface* PDI);

	static void DrawRectangle(FVector Front, FVector Right, FVector Back, FVector Left,
		FMBSVisDataBase VisData, FPrimitiveDrawInterface* PDI);

	static void DrawLine(FVector Start, FVector End, FMBSVisDataBase VisData,
		FPrimitiveDrawInterface* PDI);

	static void DrawLine(FVector Start, FVector End, FMBSDimensionsVisData VisData,
		const FLinearColor& Color, FPrimitiveDrawInterface* PDI);

	static void DrawText(const FVector Location, const FString DisplayString, const FLinearColor FontColor,
		const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas, const UFont* Font = GEngine->GetSmallFont());
};
}