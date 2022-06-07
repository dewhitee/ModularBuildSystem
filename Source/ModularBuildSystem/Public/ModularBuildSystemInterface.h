// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Config/MBSMeshConfiguration.h"
#include "Config/MBSSpawnConfiguration.h"
#include "UObject/Interface.h"
#include "ModularBuildSystemInterface.generated.h"

class IBuildingGeneratorInterface;

namespace MBS
{
class FModularLevelObserver;
class FModularLevelInitializer;
}

class IMBSPresetManagerInterface;
struct FMBSSections;
struct FMBSBounds;
struct FModularBuildStats;
class UMBSInterior;
struct FModularLevel;
class UModularBuildSystemGenerator;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UModularBuildSystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULARBUILDSYSTEM_API IModularBuildSystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Init() = 0;
	
	virtual TScriptInterface<IBuildingGeneratorInterface> GetGenerator() const = 0;
	
	virtual UMBSInterior* GetInterior() const = 0;
	
	virtual const FMBSSections& GetSections() const = 0;
	
	virtual TArray<FModularLevel*> GetAllLevels() const = 0;
	
	virtual const FModularBuildStats& GetBuildStats() const = 0;
	
	virtual const FMBSBounds& GetTransformBounds() const = 0;
	
	virtual FModularLevel* GetLevelWithId(int32 Id) const = 0;
	
	virtual const FMBSSpawnConfiguration& GetSpawnConfiguration() const = 0;
	
	virtual FMBSMeshConfiguration GetMeshConfiguration() const = 0;
	
	virtual void ResetBuildSystem(bool bResetSections = true, bool bResetActorSections = true, bool bResetInstancedSections = true,
		 bool bResetMergedSectionsStaticMeshActor = true) = 0;

	/**
	  * Attaches provided actor to this modular build system actor.
	  * @param ToAttach Actor to attach to this modular build system actor.
	  * @param bWithRelativeTransform If true - provided actor will be attached while keeping the relative transform.
	  * @param bForceReattach If true - ToAttach actor will be detached from the actor it may be attached to
	  * before attachment to this modular build system actor.
	  */
	virtual void AttachActor(AActor* ToAttach, bool bWithRelativeTransform, bool bForceReattach = false) = 0;

	virtual TScriptInterface<IMBSPresetManagerInterface> GetPresetManager() const = 0;
	
	virtual bool IsBuildModeActivated() const = 0;
	
	virtual void SetBuildModeActivated(const bool bActivated) = 0;
	
	virtual FTransform GetBuildSystemTransform() const = 0;

	virtual USceneComponent* GetRoot() const = 0;
};
