// Fill out your copyright notice in the Description page of Project Settings.


#include "Visualizer/MBSActorVisualizer.h"

#include "Visualizer/MBSActorVisualizationComponent.h"
#include "CanvasItem.h"
#include "DrawDebugHelpers.h"
#include "ModularBuildSystemEditor.h"
#include "ModularBuildSystemActor.h"

#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MBSGridCell.h"
#include "Treemap/MBSTreemap.h"
#include "Interior/MBSInterior.h"
#include "Engine/Canvas.h"

void MBS::FActorVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UMBSActorVisualizationComponent* VisualizationComponent = Cast<UMBSActorVisualizationComponent>(Component);
	//UE_LOG(LogBuildSystemEditor, Warning, TEXT("%s: Drawing visualization."), *VisualizationComponent->GetOwner()->GetName());

	check(VisualizationComponent);
	if (!VisualizationComponent->bVisualize)
	{
		return;
	}
	
	const AModularBuildSystemActor* MBS = Cast<const AModularBuildSystemActor>(VisualizationComponent->GetOwner());
	if (!MBS)
	{
		UE_LOG(LogMBSEditor, Error, TEXT("MBS is nullptr!"));
		return;
	}
	
	// Draw box
	if (VisualizationComponent->bShowBuildSystemBounds)
	{
		const FMBSBoundsVisData& BuildSystemBounds = VisualizationComponent->BuildSystemBounds;
		const FBox& Bounds = MBS->GetBoundsBox(false);
		DrawWireBox(PDI,
			//VisualizationComponent->GetOwner()->GetComponentsBoundingBox(),
			Bounds,
			BuildSystemBounds.Color,
			BuildSystemBounds.DepthPriority,
			BuildSystemBounds.LineThickness,
			BuildSystemBounds.DepthBias);
	}

	const FMBSBoundsVisData& InteriorBounds = VisualizationComponent->InteriorBounds;
	if (VisualizationComponent->bShowInteriorBounds)
	{
		if (const UMBSInterior* Interior = MBS->GetInterior())
		{
			for (const auto& Actor : Interior->GetInteriorActors())
			{
				DrawWireBox(PDI,
					Actor->GetComponentsBoundingBox(),
					InteriorBounds.Color,
					InteriorBounds.DepthPriority,
					InteriorBounds.LineThickness,
					InteriorBounds.DepthBias);
			}
		}
	}

	if (VisualizationComponent->bShowInteriorRooms)
	{
		if (const UMBSInterior* Interior = MBS->GetInterior())
		{
			TArray<FMBSRoom> Rooms = Interior->GetRooms();
			if (Rooms.IsEmpty())
			{
				UE_LOG(LogMBSEditor, Error, TEXT("No rooms!"));
			}
			for (int32 i = 0; i < Rooms.Num(); i++)
			{
				//FColor::MakeRandomColor();
				const FColor Color = UMBSActorVisualizationComponent::GetOrMakeInteriorRoomColor(i);
					
				DrawWireBox(PDI,
					Rooms[i].Bounds,
					Color,
					InteriorBounds.DepthPriority,
					InteriorBounds.LineThickness,
					InteriorBounds.DepthBias);

				DrawDashedLine(PDI,
					Rooms[i].Bounds.Min,
					Rooms[i].Bounds.Max,
					Color,
					InteriorBounds.LineThickness,
					InteriorBounds.DepthPriority,
					InteriorBounds.DepthBias);
					
				//DrawLine(Rooms[i].Bounds.Min, Rooms[i].Bounds.Max, InteriorBounds, PDI);
			}
		}
	}

	if (VisualizationComponent->bShowTransformBounds)
	{
		const FMBSBounds& Bounds = MBS->GetTransformBounds();
		if (Bounds.IsValid())
		{
			const FMBSBoundsVisData& VisData = VisualizationComponent->TransformBounds;
			const FVector BuildSystemLocation = MBS->GetActorLocation();
			//const FRotator BuildSystemRotation = MBS->GetActorRotation();

			// TODO: Currently, those locations are relative to the MBS, and I need the world coords of them
			const FVector Front = Bounds.GetTransforms().FrontLocation + BuildSystemLocation;
			const FVector Right = Bounds.GetTransforms().RightLocation + BuildSystemLocation;
			const FVector Back = Bounds.GetTransforms().BackLocation + BuildSystemLocation;
			const FVector Left = Bounds.GetTransforms().LeftLocation + BuildSystemLocation;
			// TODO: Rotate the coords by the MBS actor rotation
			DrawRectangle(Front, Right, Back, Left, VisData, PDI);
				
			// Then again the same way at the Top.Z location
			// TODO: Same as with the above locations
			const FVector Top = Bounds.GetTransforms().TopLocation + BuildSystemLocation;
			DrawRectangle(
				FVector(Front.X, Front.Y, Top.Z),
				FVector(Right.X, Right.Y, Top.Z),
				FVector(Back.X, Back.Y, Top.Z),
				FVector(Left.X, Left.Y, Top.Z),
				VisData, PDI);
		}
	}

	if (VisualizationComponent->bShowDimensions)
	{
		const FMBSBounds& Bounds = MBS->GetTransformBounds();
		if (Bounds.IsValid())
		{
			const FMBSDimensionsVisData& VisData = VisualizationComponent->Dimensions;
			const FVector BuildSystemLocation = MBS->GetActorLocation();
			//check(VisData.XText && VisData.YText && VisData.ZText);
			if (VisData.IsValid())
			{
				auto DrawDimensionLines = [&](const FVector Start, const FVector End, const FVector Moustache,
					const FLinearColor& Color)
				{
					DrawLine(Start, End, VisData, Color, PDI);
					DrawLine(Start - Moustache, Start + Moustache, VisData, Color, PDI);
					DrawLine(End - Moustache, End + Moustache, VisData, Color, PDI);
				};

				// X
				{
					// Init XText
					VisData.XText->SetText(FText::FromString(FString::FromInt(Bounds.GetBounds().X)));
					VisData.XText->SetRelativeLocation(Bounds.GetTransforms().RightLocation + FVector(0.f, VisData.TextOffsets.X, 0.f));
					VisData.XText->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

					// Draw main line of XText
					const FVector ShiftXText = FVector(0.f, 50.f, 0.f);
					const FVector RightBack = Bounds.GetTransforms().RightBack() + BuildSystemLocation + ShiftXText;
					const FVector RightFrontX = Bounds.GetTransforms().RightFront() + BuildSystemLocation + ShiftXText;
					DrawDimensionLines(
						RightBack, RightFrontX, FVector(0.f, 20.f, 0.f), VisData.XTextColor);
				}

				// Y
				{
					// Init YText
					VisData.YText->SetText(FText::FromString(FString::FromInt(Bounds.GetBounds().Y)));
					VisData.YText->SetRelativeLocation(Bounds.GetTransforms().FrontLocation - FVector(VisData.TextOffsets.Y, 0.f, 0.f));
					VisData.YText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

					// Draw main line of YText
					const FVector ShiftYText = FVector(-50.f, 0.f, 0.f);
					const FVector LeftFront = Bounds.GetTransforms().LeftFront() + BuildSystemLocation + ShiftYText;
					const FVector RightFrontY = Bounds.GetTransforms().RightFront() + BuildSystemLocation + ShiftYText;
					DrawDimensionLines(
						LeftFront, RightFrontY, FVector(20.f, 0.f, 0.f), VisData.YTextColor);
				}

				// Z
				{
					// Init ZText
					VisData.ZText->SetText(FText::FromString(FString::FromInt(Bounds.GetBounds().Z)));
					VisData.ZText->SetRelativeLocation(Bounds.GetTransforms().TopLocation - FVector(VisData.TextOffsets.Z, 0.f, 0.f));
					VisData.ZText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
						
					// Draw ZText
					const FVector LeftFrontZ = Bounds.GetTransforms().LeftFront() + BuildSystemLocation + FVector(0.f, 0.f, 50.f);
					const FVector TopLeftFront = Bounds.GetTransforms().TopLeftFront() + BuildSystemLocation + FVector(0.f, 0.f, -50.f);
					DrawDimensionLines(
						LeftFrontZ, TopLeftFront, FVector(20.f, 0.f, 0.f), VisData.ZTextColor);
				}
			}
		}
	}

	if (VisualizationComponent->bShowGrid)
	{
		const FMBSGridVisData& VisData = VisualizationComponent->Grid;
		const FTransform& MBSTransform = MBS->GetActorTransform();
		const FVector StretchCoefficients = MBS->GetStretchManager().GetScaleCoefficientsSwappedXY();
		for (int32 x = 0; x < VisData.CellCount; x++)
		{
			for (int32 y = 0; y < VisData.CellCount; y++)
			{
				const MBS::FGridCell3D Cell(x, y, VisData.ZIndex, MBSTransform, StretchCoefficients, true);
				const FVector Front = Cell.BottomLeftRelative();
				const FVector Right = Cell.BottomRightRelative();
				const FVector Back = Cell.TopRightRelative();
				const FVector Left = Cell.TopLeftRelative();
				DrawRectangle(Front, Right, Back, Left, static_cast<FMBSVisDataBase>(VisData), PDI);
			}
		}
	}

	if (VisualizationComponent->bShowCustom)
	{
		const FMBSCustomVisData& VisData = VisualizationComponent->Custom;
		const FVector& MBSLoc = MBS->GetActorTransform().GetLocation();
		for (const auto& Elem : VisData.CustomDrawElements)
		{
			for (const auto& Line : Elem.LinesToDraw)
			{
				if (Elem.bRelative)
				{
					DrawLine(Line.v1 + MBSLoc, Line.v2 + MBSLoc, static_cast<FMBSVisDataBase>(Elem), PDI);
				}
				else
				{
					DrawLine(Line.v1, Line.v2, static_cast<FMBSVisDataBase>(Elem), PDI);
				}
			}
		}
	}
}

void MBS::FActorVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
	const FSceneView* View, FCanvas* Canvas)
{
	const UMBSActorVisualizationComponent* VisualizationComponent = Cast<UMBSActorVisualizationComponent>(Component);

	check(VisualizationComponent);
	if (!VisualizationComponent->bVisualize)
	{
		return;
	}
	
	const AModularBuildSystemActor* MBS = Cast<const AModularBuildSystemActor>(VisualizationComponent->GetOwner());
	if (const UMBSInterior* Interior = MBS->GetInterior())
	{
		if (VisualizationComponent->bShowInteriorActorNames)
		{
			for (const auto& Actor : Interior->GetInteriorActors())
			{
				DrawText(
					Actor->GetActorLocation(),
					Actor->GetName(),
					FLinearColor::White,
					Viewport,
					View,
					Canvas);
			}
		}

		if (VisualizationComponent->bShowInteriorRooms)
		{
			for (int32 i = 0; i < Interior->GetRooms().Num(); i++)
			{
				DrawText(
					Interior->GetRooms()[i].Bounds.GetCenter(),
					Interior->GetRooms()[i].Name.ToString(),
					UMBSActorVisualizationComponent::GetOrMakeInteriorRoomColor(i),
					Viewport,
					View,
					Canvas,
					GEngine->GetMediumFont());
			}
		}
	}

	if (VisualizationComponent->bShowGrid)
	{
		const FMBSGridVisData& VisData = VisualizationComponent->Grid;
		if (VisData.bShowText)
		{
			const FTransform& MBSTransform = MBS->GetActorTransform();
			const FVector StretchCoefficients = MBS->GetStretchManager().GetScaleCoefficientsSwappedXY();
			for (int32 x = 0; x < VisData.CellCount; x++)
			{
				for (int32 y = 0; y < VisData.CellCount; y++)
				{
					const MBS::FGridCell3D Cell(x, y, VisData.ZIndex, MBSTransform, StretchCoefficients, true);
					DrawText(Cell.CenterRelative(), Cell.GetCode(), VisData.TextColor, Viewport, View, Canvas);
				}
			}
		}
	}
}

void MBS::FActorVisualizer::DrawDiamond(FVector Front, FVector Right, FVector Back, FVector Left,
	FMBSVisDataBase VisData, FPrimitiveDrawInterface* PDI)
{
	// From front to right
	DrawDashedLine(
		PDI, Front, Right, VisData.Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);

	// Front right to back
	DrawDashedLine(
		PDI, Right, Back, VisData.Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);

	// From back to left
	DrawDashedLine(
		PDI, Back, Left, VisData.Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);

	// From left to front
	DrawDashedLine(
		PDI, Left, Front, VisData.Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);
}

void MBS::FActorVisualizer::DrawRectangle(FVector Front, FVector Right, FVector Back, FVector Left,
	FMBSVisDataBase VisData, FPrimitiveDrawInterface* PDI)
{
	// From front to right
	const FVector FrontRightEdge = FVector(Front.X, Right.Y, Right.Z);
	DrawLine(Front, FrontRightEdge, VisData, PDI);
	DrawLine(FrontRightEdge, Right, VisData, PDI);

	// Front right to back
	const FVector RightBackEdge = FVector(Back.X, Right.Y, Back.Z);
	DrawLine(Right, RightBackEdge, VisData, PDI);
	DrawLine(RightBackEdge, Back, VisData, PDI);

	// From back to left
	const FVector BackLeftEdge = FVector(Back.X, Left.Y, Left.Z);
	DrawLine(Back, BackLeftEdge, VisData, PDI);
	DrawLine(BackLeftEdge, Left, VisData, PDI);

	// From left to front
	const FVector LeftFrontEdge = FVector(Front.X, Left.Y, Front.Z);
	DrawLine(Left, LeftFrontEdge, VisData, PDI);
	DrawLine(LeftFrontEdge, Front, VisData, PDI);
}

void MBS::FActorVisualizer::DrawLine(FVector Start, FVector End, FMBSVisDataBase VisData,
	FPrimitiveDrawInterface* PDI)
{
	DrawDashedLine(PDI, Start, End, VisData.Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);
}

void MBS::FActorVisualizer::DrawLine(FVector Start, FVector End, FMBSDimensionsVisData VisData,
	const FLinearColor& Color, FPrimitiveDrawInterface* PDI)
{
	DrawDashedLine(PDI, Start, End, Color, VisData.LineThickness, VisData.DepthPriority, VisData.DepthBias);
}

void MBS::FActorVisualizer::DrawText(const FVector Location, const FString DisplayString, const FLinearColor FontColor,
	const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas, const UFont* Font)
{
	const int32 HalfX = 0.5f * Viewport->GetSizeXY().X;
	const int32 HalfY = 0.5f * Viewport->GetSizeXY().Y;
 
	const FPlane Proj = View->Project(Location);
	if (Proj.W > 0.f)
	{
		const int32 XPos = HalfX + (HalfX * Proj.X);
		const int32 YPos = HalfY + (HalfY * (Proj.Y * -1.f));
		FCanvasTextItem TextItem(FVector2D(XPos + 5, YPos), FText::FromString(DisplayString),
			Font, FLinearColor::White);
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);
	}
}

void MBS::FActorVisualizer::DrawSingleModularSection(const FBox Box, const FMBSBoundsVisData& Bounds,
	FPrimitiveDrawInterface* PDI)
{
	const auto DrawSingleLine = [&](const FVector& Start, const FVector& End)
	{
		DrawDashedLine(PDI,
			Start,
			End,
			Bounds.Color,
			Bounds.LineThickness * 2.f,
			Bounds.DepthPriority,
			Bounds.DepthBias);
	};
	
	DrawWireBox(PDI,
		Box,
		Bounds.Color,
		Bounds.DepthPriority,
		Bounds.LineThickness,
		Bounds.DepthBias);

	DrawSingleLine(
		FVector(Box.Min.X, Box.Min.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Max.Y, Box.Max.Z));
}

void MBS::FActorVisualizer::DrawSingleModularLevel(const AModularBuildSystemActor* BS, const FModularLevel& InLevel,
	const FMBSBoundsVisData& Bounds, FPrimitiveDrawInterface* PDI)
{
	if (BS->IsOfInstancedMeshConfigurationType())
	{
		if (const FModularSectionInstanced* Section = BS->GetInstancedSectionOfLevel(InLevel))
		{
			DrawSingleModularSection(Section->GetISMC()->Bounds.GetBox(), Bounds, PDI);
		}
	}
	else
	{
		for (const FModularSection* Section : BS->GetSectionsOfLevel(InLevel))
		{
			DrawSingleModularSection(Section->GetStaticMeshActor()->GetComponentsBoundingBox(), Bounds, PDI);
		}
	}
}
