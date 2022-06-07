// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "MBSSpawnConfiguration.generated.h"

UENUM(BlueprintType)
enum class EMBSClampMode : uint8
{
	Default,
	Clamp,
	Stretch,
};

UENUM(BlueprintType)
enum class EMBSExecutionMode : uint8
{
	Default,
	
	/**
	 * Always reinitialize all sections.
	 */
	Regen,
	
	/**
	 * Initialize only new sections and update transforms of existing sections when necessary.
	 */
	Smart
};

USTRUCT(BlueprintType)
struct FMBSSpawnConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ModularBuildSystem")
	EMBSClampMode ClampMode = EMBSClampMode::Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ModularBuildSystem")
	EMBSClampMode DefaultClampMode = EMBSClampMode::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ModularBuildSystem")
	EMBSExecutionMode ExecutionMode = EMBSExecutionMode::Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ModularBuildSystem")
	EMBSExecutionMode DefaultExecutionMode = EMBSExecutionMode::Default;

	/**
	  * Modular section pivot location. Used to properly snap modular sections to each other.
	  */
	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	EModularSectionPivotLocation SectionPivotLocation = EModularSectionPivotLocation::BottomRight;

	/**
	 * If true - use relative transform when spawning new modular section. Use world transform otherwise.
	 */
	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	bool bUseRelativeTransform = true;

	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	bool bReloadOnConstruction = false;
	
	/**
	 * Do reload and re-initialize build system each time build system actor moves?
	 * @note Works only when Generator is not set.
	 * TODO: Deprecate and remove.
	 */
	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	bool bReconstructOnMove = false;

	/**
	 * If true - resets modular build system actor rotation after reconstruct.
	 * TODO: Deprecate and remove.
	 */
	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	bool bResetRotationAlso = false;

	FActorSpawnParameters SectionSpawnParams;
};
