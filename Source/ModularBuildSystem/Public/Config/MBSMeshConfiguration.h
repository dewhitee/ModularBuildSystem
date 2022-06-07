// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "MBSMeshConfiguration.generated.h"

class AModularBuildSystemActor;

UENUM(BlueprintType)
enum class EMBSMeshConfigurationType : uint8
{
	Default,
	StaticMeshes,
	InstancedStaticMeshes,
	HierarchicalInstancedStaticMeshes
};


/**
 * Structure that holds all mesh related configuration of a modular build system.
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FMBSMeshConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	EMBSMeshConfigurationType Type = EMBSMeshConfigurationType::Default;

	UPROPERTY(VisibleAnywhere, Config, Category = "ModularBuildSystem")
	EMBSMeshConfigurationType DefaultType = EMBSMeshConfigurationType::Default;

	/**
	 * If true - instanced static mesh component will be created only for each unique mesh.
	 * @note That way many levels may reference the same static mesh component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	bool bUseSingleComponentPerUniqueMesh = false;

	/**
	 * If true - modular section static meshes will be updated after each build system actor update.
	 */
	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	bool bDynamicallyChangeMeshes = true;

	UPROPERTY(EditInstanceOnly, AdvancedDisplay, Category = "ModularBuildSystem")
	EModularSectionReloadMode ReloadMode = EModularSectionReloadMode::CurrentLevelOnly;
	
	/**
	 * @return True if configuration type is set to InstancedStaticMeshes or HierarchicalInstancedStaticMeshes.
	 */
	bool IsOfInstancedType() const;

};
