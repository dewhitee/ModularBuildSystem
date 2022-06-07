// Fill out your copyright notice in the Description page of Project Settings.


#include "Visualizer/HouseVisualizer.h"

#include "Visualizer/HouseVisualizationComponent.h"
#include "ModularBuildSystemActor.h"
#include "House/HouseBuildSystemActor.h"
#include "House/HouseInteriorGenerator.h"
#include "Treemap/MBSTreemap.h"
#include "Interior/MBSInterior.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"

void MBS::FHouseVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	FActorVisualizer::DrawVisualization(Component, View, PDI);

	const UHouseVisualizationComponent* VisualizationComponent = Cast<UHouseVisualizationComponent>(Component);
	
	// Visualize house
	if (!VisualizationComponent->bVisualize)
	{
		return;
	}
	
	const AHouseBuildSystemActor* MBS = Cast<const AHouseBuildSystemActor>(VisualizationComponent->GetOwner());
	const UMBSInterior* Interior = MBS->GetInterior();
	const UMBSInteriorGenerator* InteriorGenerator = Interior ? Interior->GetGenerator() : nullptr;
		
	if (VisualizationComponent->bShowBasementBounds)
	{
		const FMBSBoundsVisData& BasementBounds = VisualizationComponent->BasementBounds;
		const FMBSBoundsVisData& InteriorBounds = VisualizationComponent->GetInteriorBounds();

		if (MBS->IsOfInstancedMeshConfigurationType())
		{
			if (const FModularSectionInstanced* Section = MBS->GetInstancedSectionOfLevel(MBS->Basement))
			{
				const FBox& BasementBox = Section->GetISMC()->Bounds.GetBox();
					
				// Draw basement bounds
				DrawBoundsBox(BasementBox, BasementBounds, PDI);

				// Draw bounds at which interior can be generated
				if (InteriorGenerator)
				{
					DrawInteriorAvailableBox(BasementBox, BasementBounds, InteriorBounds, InteriorGenerator, PDI);
				}

				if (BasementBounds.bVisualizeEachSection)
				{
					for (int32 i = 0; i < Section->GetISMC()->GetInstanceCount(); i++)
					{
						FTransform InstanceTransform;
						Section->GetISMC()->GetInstanceTransform(i, InstanceTransform, true);
							
						const FBox& SingleSectionBox = FBox(InstanceTransform.GetLocation(),
							InstanceTransform.GetLocation() + FVector(MBS->Basement.GetResolutionVec()));
						DrawSingleModularSection(SingleSectionBox, BasementBounds, PDI);
					}
				}
			}
		}
		else
		{
			for (const FModularSection* Section : MBS->GetSectionsOfLevel(MBS->Basement))
			{
				const FBox& BasementBox = Section->GetStaticMeshActor()->GetComponentsBoundingBox();
					
				// Draw basement bounds. TODO: Move out of loop to draw only once
				DrawBoundsBox(BasementBox, BasementBounds, PDI);

				// Draw bounds at which interior can be generated. TODO: Move out of loop to draw only once
				//DrawInteriorAvailableBox(BasementBox, BasementBounds, InteriorBounds, InteriorGenerator, PDI);

				// Draw single section
				DrawSingleModularSection(BasementBox, BasementBounds, PDI);
			}
		}
	}

	if (VisualizationComponent->bShowWallBounds)
	{
		for (const auto& Wall : MBS->Walls)
		{
			DrawSingleModularLevel(MBS, Wall, VisualizationComponent->WallBounds, PDI);
		}
	}

	if (VisualizationComponent->bShowFloorBounds)
	{
		for (const auto& Floor : MBS->Floors)
		{
			DrawSingleModularLevel(MBS, Floor, VisualizationComponent->FloorBounds, PDI);
		}
	}

	if (VisualizationComponent->bShowCornerBounds)
	{
		for (const auto& Corner : MBS->Corners)
		{
			DrawSingleModularLevel(MBS, Corner, VisualizationComponent->CornerBounds, PDI);
		}
	}

	if (VisualizationComponent->bShowRoofBounds)
	{
		DrawSingleModularLevel(MBS, MBS->Roof, VisualizationComponent->RoofBounds, PDI);
	}

	if (VisualizationComponent->bShowRooftopBounds)
	{
		DrawSingleModularLevel(MBS, MBS->Rooftop, VisualizationComponent->RooftopBounds, PDI);
	}
}

void MBS::FHouseVisualizer::DrawBoundsBox(const FBox Box, const FMBSBoundsVisData& Bounds,
	FPrimitiveDrawInterface* PDI)
{
	DrawWireBox(PDI,
		Box,
		Bounds.Color,
		Bounds.DepthPriority,
		Bounds.LineThickness,
		Bounds.DepthBias);
}

void MBS::FHouseVisualizer::DrawInteriorAvailableBox(FBox Box, const FMBSBoundsVisData& BasementBounds,
	const FMBSBoundsVisData& InteriorBounds, const UMBSInteriorGenerator* InteriorGenerator, 
	FPrimitiveDrawInterface* PDI)
{
	check(InteriorGenerator);
	InteriorGenerator->AdjustBoxToInteriorAllowedArea(Box, nullptr);
	DrawDashedLine(PDI,
		FVector(Box.Min.X, Box.Min.Y, Box.Max.Z),
		Box.Max,
		BasementBounds.Color,
		InteriorBounds.LineThickness * 4.f,
		InteriorBounds.DepthPriority,
		InteriorBounds.DepthBias);
				
	DrawDashedLine(PDI,
		FVector(Box.Min.X, Box.Max.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Min.Y, Box.Max.Z),
		BasementBounds.Color,
		InteriorBounds.LineThickness * 4.f,
		InteriorBounds.DepthPriority,
		InteriorBounds.DepthBias);

	// Set Min.Z to Max.Z as we only need plane
	Box.Min.Z = Box.Max.Z;
	DrawWireBox(PDI,
		Box,
		BasementBounds.Color,
		InteriorBounds.DepthPriority,
		InteriorBounds.LineThickness * 2.f,
		InteriorBounds.DepthBias);
}

