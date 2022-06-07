// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemActor.h"
#include "HouseBuildSystemActor.generated.h"

class UHousePresetManager;
class UHouseBuildSystemGenerator;
class UModularSectionResolution;
class UModularBuildSystemMeshList;
class UHouseBuildSystemPreset;
class UHouseBuildSystemPresetList;
class UHouseInteriorGenerator;

/**
 * Base class of all modular houses.
 */
UCLASS(NonTransient)
class MODULARBUILDSYSTEM_API AHouseBuildSystemActor : public AModularBuildSystemActor
{
	GENERATED_BODY()

public:
	AHouseBuildSystemActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	FModularLevel Basement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	TArray<FModularLevel> Floors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	TArray<FModularLevel> Walls;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	TArray<FModularLevel> Corners;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	FModularLevel Roof;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem|Levels", meta = (EditCondition = "!bBuildModeIsActivated"))
	FModularLevel Rooftop;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Instanced, Category = "Presets")
	TObjectPtr<UHousePresetManager> PresetManager;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Instanced, Category = "Generator")
	TObjectPtr<UHouseBuildSystemGenerator> Generator;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Generator")
	bool bGenerateOnChange;

private:
	UPROPERTY(EditInstanceOnly, Category=Display)
	FHouseDisplaySettings HouseDisplaySettings;
	
public:

#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectBasement();

	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectFloors();

	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectWalls();

	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectCorners();

	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectRoof();

	UFUNCTION(CallInEditor, Category = "Selection")
	void SelectRooftop();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	virtual void Init() override;
	virtual TScriptInterface<IBuildingGeneratorInterface> GetGenerator() const override;
	virtual void CollectStats() override;
	virtual void OnUpdateTransformBounds() override;
	virtual void OnBoundsTransformChange() override;
	virtual FModularLevel* GetLevelWithId(int32 Id) const override;
	virtual TArray<FModularLevel*> GetAllLevels() const override;
	virtual TScriptInterface<IMBSPresetManagerInterface> GetPresetManager() const override;

protected:
	virtual void ApplyStretch() override;

private:
	void UpdateHouseDisplay() const;

	// Friends
	friend class UHouseBuildSystemGenerator;
	friend class UHousePresetManager;
};
