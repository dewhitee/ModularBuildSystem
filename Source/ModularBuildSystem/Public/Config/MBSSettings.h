// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSSpawnConfiguration.h"
#include "MBSSettings.generated.h"

enum class EMBSMeshConfigurationType : uint8;

USTRUCT(BlueprintType)
struct FMBSDisplaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowInteriorOnly = false;
};

USTRUCT(BlueprintType)
struct FHouseDisplaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowBasement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowFloors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowWalls = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowCorners = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowRoof = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bShowRooftop = true;
};

/**
 * 
 */
UCLASS(Config=EditorUserSettings, DefaultConfig)
class MODULARBUILDSYSTEM_API UMBSSettings : public UObject
{
	GENERATED_BODY()

public:
	UMBSSettings();

	UPROPERTY(EditAnywhere, Config, Category=MBS)
	EMBSMeshConfigurationType DefaultMeshConfigurationType;

	UPROPERTY(EditAnywhere, Config, Category=MBS)
	EMBSClampMode DefaultClampMode;

	UPROPERTY(EditAnywhere, Config, Category=MBS)
	EMBSExecutionMode DefaultExecutionMode;

	UPROPERTY(EditAnywhere, Config, Category=MBS)
	bool bUseSingleInstancedComponentPerUniqueMesh;
	
	/**
	 * TODO: Implement. Currently not supported.
	 */
	UPROPERTY(EditAnywhere, Config, Category=MBS)
	bool bAutoAttachVisualizationComponent;

	UPROPERTY(VisibleAnywhere, Config, Category=MBS)
	FIntVector DefaultSectionSize;
};
