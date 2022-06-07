// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSPresetManagerInterface.h"
#include "House/HouseBuildSystemActor.h"
#include "UObject/NoExportTypes.h"
#include "HousePresetManager.generated.h"

class UHouseBuildSystemPresetList;
class UHouseBuildSystemPreset;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MODULARBUILDSYSTEM_API UHousePresetManager : public UObject, public IMBSPresetManagerInterface
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<AHouseBuildSystemActor> BS;

private:	
	UPROPERTY(VisibleAnywhere, Category="Presets")
	bool bBuildModeIsActivated;
	
	UPROPERTY(EditInstanceOnly, Category="Presets")
	TObjectPtr<UHouseBuildSystemPreset> Preset;

	UPROPERTY(EditInstanceOnly, Category="Presets", meta=(EditCondition="PresetList != nullptr", ClampMin=0, ClampMax=16))
	uint8 PresetSlider;
	
	UPROPERTY(EditInstanceOnly, Category="Presets")
	TObjectPtr<UHouseBuildSystemPresetList> PresetList;
	
public:
	virtual void SavePreset_Implementation() override;
	virtual void LoadPreset_Implementation() override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
