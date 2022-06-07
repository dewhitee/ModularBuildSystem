// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularLevel.h"

class IModularBuildSystemInterface;
class UMBSTransformSolver;
enum class EModularSectionPivotLocation : uint8;
class UModularLevelShape;
class UModularSectionResolution;
class UModularBuildSystemMeshList;

namespace MBS
{

/**
 * Modular building level builder class. Currently not used.
 */
class MODULARBUILDSYSTEM_API FModularLevelBuilder
{
public:
	FModularLevelBuilder();
	FModularLevelBuilder(const FModularLevel& InLevel);
	~FModularLevelBuilder();

	FModularLevel& SetMeshList(UModularBuildSystemMeshList* InMeshList);
	FModularLevel& SetMeshListIndex(int32 InIndex);
	FModularLevel& SetResolution(UModularSectionResolution* InResolution);
	FModularLevel& SetTotalCount(int32 InTotalCount);
	FModularLevel& SetMaxInRow(int32 InMaxInRow);
	FModularLevel& SetFromMeshList();
	FModularLevel& SetShape(UModularLevelShape* InShape);
	FModularLevel& SetInstancedStaticMeshComponent(UStaticMesh* InStaticMesh, TScriptInterface<IModularBuildSystemInterface> InBuildSystem, bool bUseExistingMesh);
	FModularLevel& SetPivotLocation(EModularSectionPivotLocation NewPivotLocation);
	FModularLevel& SetName(const FName NewName);
	FModularLevel& SetAffectsNextZMultiplier(bool bAffects);
	FModularLevel& SetSolver(UMBSTransformSolver* InSolver);
	FModularLevel& SetUpdated(bool bNewUpdated);
	
private:
	FModularLevel Level;
	
};


}

