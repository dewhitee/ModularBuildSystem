// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TextRenderComponent.h"
#include "MBSActorVisualizationComponent.generated.h"

namespace MBS
{
class FActorVisualizer;
}

USTRUCT(BlueprintType)
struct FMBSVisDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=1, ClampMax=128))
	int32 LineThickness = 20;

	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=0, ClampMax=256))
	int32 DepthPriority = 10;
	
	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=1, ClampMax=8192))
	int32 DepthBias = 50;

	UPROPERTY(EditAnywhere, Category=Visualizer)
	FLinearColor Color = FLinearColor::Blue;
};

USTRUCT(BlueprintType)
struct FMBSBoundsVisData : public FMBSVisDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Visualizer)
	bool bVisualizeEachSection = false;
};

USTRUCT(BlueprintType)
struct FMBSDimensionsVisData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=1, ClampMax=128))
	int32 LineThickness = 20;

	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=0, ClampMax=256))
	int32 DepthPriority = 10;
	
	UPROPERTY(EditAnywhere, Category=Visualizer, meta=(ClampMin=1, ClampMax=8192))
	int32 DepthBias = 50;

	UPROPERTY(EditAnywhere, Category=Visualizer)
	int32 FontSize = 256;

	UPROPERTY(EditAnywhere, Category=Visualizer)
	FVector TextOffsets = FVector(100.f);

	UPROPERTY(EditAnywhere, Category=Visualizer)
	FLinearColor XTextColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, Category=Visualizer)
	FLinearColor YTextColor = FLinearColor::Green;
	
	UPROPERTY(EditAnywhere, Category=Visualizer)
	FLinearColor ZTextColor = FLinearColor::Blue;

	UPROPERTY(VisibleAnywhere, Category=Visualizer)
	TObjectPtr<UTextRenderComponent> XText = nullptr;

	UPROPERTY(VisibleAnywhere, Category=Visualizer)
	TObjectPtr<UTextRenderComponent> YText = nullptr;

	UPROPERTY(VisibleAnywhere, Category=Visualizer)
	TObjectPtr<UTextRenderComponent> ZText = nullptr;

	void SetTextData(UTextRenderComponent* Text, const FLinearColor& Color);
	bool IsValid() const { return XText && YText && ZText; }
};

USTRUCT(BlueprintType)
struct FMBSGridVisData : public FMBSVisDataBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 FontSize = 256;

	UPROPERTY(EditAnywhere)
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, meta=(ClampMin=1, ClampMax=32))
	int32 CellCount = 10;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0, ClampMax=32))
	int32 ZIndex = 0;

	UPROPERTY(EditAnywhere)
	bool bShowText = true;
};

USTRUCT(BlueprintType)
struct FMBSCustomVisDataElement : public FMBSVisDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName DebugName;

	UPROPERTY(EditAnywhere)
	TArray<FTwoVectors> LinesToDraw;

	UPROPERTY(EditAnywhere)
	bool bRelative = false;
};

USTRUCT(BlueprintType)
struct FMBSCustomVisData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(TitleProperty="DebugName"))
	TArray<FMBSCustomVisDataElement> CustomDrawElements;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MODULARBUILDSYSTEMEDITOR_API UMBSActorVisualizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMBSActorVisualizationComponent();
	
	static TArray<FColor> InteriorRoomColors;
	
protected:
	UPROPERTY(EditAnywhere, Category="Visualizer")
	bool bVisualize = true;

private:
	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem")
	bool bShowBuildSystemBounds = true;

	UPROPERTY(EditAnywhere, Category="Visualizer|Interior")
	bool bShowInteriorBounds = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Interior")
	bool bShowInteriorRooms = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem")
	bool bShowTransformBounds = false;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem")
	bool bShowDimensions = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Grid")
	bool bShowGrid = false;

	UPROPERTY(EditAnywhere, Category="Visualizer|Custom")
	bool bShowCustom = false;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem", meta=(EditCondition=bShowBuildSystemBounds))
	FMBSBoundsVisData BuildSystemBounds;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Interior", meta=(EditCondition=bShowInteriorBounds))
	FMBSBoundsVisData InteriorBounds;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Interior", meta=(EditCondition=bShowInteriorRooms))
	FMBSBoundsVisData InteriorRooms;

	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem", meta=(EditCondition=bShowTransformBounds))
	FMBSBoundsVisData TransformBounds;

	UPROPERTY(EditAnywhere, Category="Visualizer|BuildSystem", meta=(EditCondition=bShowDimensions))
	FMBSDimensionsVisData Dimensions;

	UPROPERTY(EditAnywhere, Category="Visualizer|Grid", meta=(EditCondition=bShowGrid))
	FMBSGridVisData Grid;

	UPROPERTY(EditAnywhere, Category="Visualizer|Interior", meta=(EditCondition=bShowInteriorBounds))
	bool bShowInteriorActorNames = true;
	
	UPROPERTY(EditAnywhere, Category="Visualizer|Custom")
	FMBSCustomVisData Custom;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	const FMBSBoundsVisData& GetInteriorBounds() const { return InteriorBounds; }
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	static FColor GetOrMakeInteriorRoomColor(int32 AtIndex);

protected:	
	UFUNCTION(CallInEditor, Category="Visualizer")
	void ResetColors();

	friend class MBS::FActorVisualizer;
};
