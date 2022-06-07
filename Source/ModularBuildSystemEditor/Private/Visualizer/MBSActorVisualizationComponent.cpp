// Fill out your copyright notice in the Description page of Project Settings.


#include "Visualizer/MBSActorVisualizationComponent.h"

#include "ModularBuildSystemActor.h"
#include "ModularBuildSystemEditor.h"

TArray<FColor> UMBSActorVisualizationComponent::InteriorRoomColors = TArray<FColor>();

void FMBSDimensionsVisData::SetTextData(UTextRenderComponent* Text, const FLinearColor& Color)
{
	if (Text)
	{
		Text->SetWorldSize(FontSize);
		Text->SetTextRenderColor(Color.ToFColor(false));
	}
}

// Sets default values for this component's properties
UMBSActorVisualizationComponent::UMBSActorVisualizationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMBSActorVisualizationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMBSActorVisualizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMBSActorVisualizationComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const auto StructProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
	const FName StructName = StructProperty->GetFName();
	
	UE_LOG(LogMBSEditor, Verbose, TEXT("%s: Property %s has been changed!"), *GetName(), *PropertyName.ToString());
	if (GET_MEMBER_NAME_CHECKED(UMBSActorVisualizationComponent, bShowDimensions) == PropertyName
		|| GET_MEMBER_NAME_CHECKED(UMBSActorVisualizationComponent, Dimensions) == StructName)
	{
		UE_LOG(LogMBSEditor, Verbose, TEXT("%s: Dimensions property has been changed!"), *GetName());
		if (bShowDimensions)
		{
			AModularBuildSystemActor* MBS = Cast<AModularBuildSystemActor>(GetOwner());
			auto AddTextComponent = [&](TObjectPtr<UTextRenderComponent>& ToSet, const FVector AtLocation)
			{
				if (!ToSet)
				{
					const FName ComponentName = MakeUniqueObjectName(MBS, UTextRenderComponent::StaticClass());
					ToSet = NewObject<UTextRenderComponent>(MBS, UTextRenderComponent::StaticClass(), ComponentName);
					ToSet->SetMobility(EComponentMobility::Static);
					ToSet->SetupAttachment(MBS->GetRootComponent());
					ToSet->SetRelativeLocation(AtLocation);
					ToSet->SetWorldSize(Dimensions.FontSize);
					//ToSet->SetTextRenderColor(Color.ToFColor(false));
					ToSet->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
					ToSet->RegisterComponent();
				}
			};
			const FMBSBounds& Bounds = MBS->GetTransformBounds();
			AddTextComponent(Dimensions.XText, Bounds.GetTransforms().FrontLocation);
			Dimensions.SetTextData(Dimensions.XText, Dimensions.XTextColor);
			
			AddTextComponent(Dimensions.YText, Bounds.GetTransforms().RightLocation);
			Dimensions.SetTextData(Dimensions.YText, Dimensions.YTextColor);

			AddTextComponent(Dimensions.ZText, Bounds.GetTransforms().TopLocation);
			Dimensions.SetTextData(Dimensions.ZText, Dimensions.ZTextColor);
		}
		else
		{
			auto RemoveTextComponent = [&](TObjectPtr<UTextRenderComponent>& ToSet)
			{
				if (ToSet)
				{
					ToSet->UnregisterComponent();
					ToSet->DestroyComponent();
					ToSet = nullptr;
				}
			};
			RemoveTextComponent(Dimensions.XText);
			RemoveTextComponent(Dimensions.YText);
			RemoveTextComponent(Dimensions.ZText);
		}
	}
	
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UMBSActorVisualizationComponent::ResetColors()
{
	InteriorRoomColors.Empty();
}

FColor UMBSActorVisualizationComponent::GetOrMakeInteriorRoomColor(int32 AtIndex)
{
	if (!InteriorRoomColors.IsValidIndex(AtIndex))
	{
		const FColor Color = FColor::MakeRandomColor();
		InteriorRoomColors.Add(Color);
		return Color;
	}
	return InteriorRoomColors[AtIndex];
}

