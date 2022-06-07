// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSBounds.h"
#include "MBSMerger.h"
#include "MBSSections.h"
#include "Config/MBSMeshConfiguration.h"
#include "Config/MBSSpawnConfiguration.h"
#include "Config/MBSSettings.h"
#include "MBSStretchManager.h"
#include "ModularBuildStats.h"
#include "ModularBuildSystemInterface.h"
#include "GameFramework/Actor.h"
#include "ModularSection.h"
#include "ModularLevel.h"
#include "ModularLevelInitializer.h"
#include "ModularLevelObserver.h"
#include "ModularBuildSystemActor.generated.h"

namespace MBS
{
class FActorDetails;
class FModularLevelObserver;
class FModularLevelInitializer;
}

class USplineComponent;
class UMBSBounds;
class AStaticMeshActor;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UModularSectionResolution;
class UModularBuildSystemMeshList;
class UModularBuildSystemPreset;
class UModularBuildSystemPresetList;
class UModularBuildSystemGenerator;
class UModularLevelShape;
class UMBSInterior;
class UStaticMesh;

/**
 * Abstract base class of all build system actors of ModularBuildSystem plugin.
 * Each modular build system actor is a single separate building with optional interior.
 * 
 */
UCLASS(Abstract, Config=Game, HideCategories=(Replication, Rendering, Collision, Input, LOD, Cooking))
class MODULARBUILDSYSTEM_API AModularBuildSystemActor : public AActor, public IModularBuildSystemInterface
{
	GENERATED_BODY()
	
protected:	
	// Sets default values for this actor's properties
	AModularBuildSystemActor();

	/**
	 * Stats of modular build system actor generated building.
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	FModularBuildStats BuildStats;

	/**
	 * Struct that contains the modifications of this MBS actor sections' transforms, and handles updates to them.
	 */
	UPROPERTY(EditInstanceOnly, Config, Category = "Stretch")
	FMBSStretchManager StretchManager;

	UPROPERTY(EditInstanceOnly, Category="Bounds")
	FMBSBounds TransformBounds;

	// TODO: Test new
	MBS::FModularLevelInitializer	LevelInitializer;
	MBS::FModularLevelObserver		LevelObserver;

private:
	/**
	 * Build mode supposed to be active when building is finally generated. While this mode is active it is safe to modify
	 * any modular sections of this build system manually.
	 * 
	 * TODO: Think about renaming this property, as it may be confusing
	 */
	UPROPERTY(VisibleInstanceOnly, Category = "ModularBuildSystem")
	bool bBuildModeIsActivated;

	UPROPERTY(EditInstanceOnly, Config, Category = "Config")
	FMBSMeshConfiguration MeshConfiguration;
	
	UPROPERTY(EditInstanceOnly, Config, Category = "Config")
	FMBSSpawnConfiguration SpawnConfiguration;

	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, DuplicateTransient, Category = "ModularBuildSystem")
	FMBSSections Sections;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, Category=Merge)
	FMBSMerger Merger;

	UPROPERTY(VisibleInstanceOnly, Category=Display)
	FMBSDisplaySettings DisplaySettings;
#endif

	/**
	 * Pointer to the interior object.
	 */
	UPROPERTY(EditInstanceOnly, Instanced, Category = "Interior")
	TObjectPtr<UMBSInterior> Interior;
	
	/**
	 * Stores this actor pre OnConstruction method call transform. Might be removed later.
	 */
	FTransform PreviousTransform;

public:
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	FModularSection InitModularSection(UStaticMesh* InStaticMesh, const FTransform& InTransform, int32 InLevelId,
		bool bAddToSections, bool bWithRelativeTransform);

	TArray<FModularSection> InitMultipleModularSections(UStaticMesh* InStaticMesh, const TArray<FTransform>& InTransforms,
		int32 InLevelId, bool bAddToSections, bool bWithRelativeTransform);

	/**
	 * Initializes single modular section actor at the specified relative (to the build system actor) transform.
	 * @param InTransform 
	 * @param InLevelId 
	 * @param InClass
	 * @param bAddToActorSections
	 * @param bWithRelativeTransform
	 *
	 * TODO: Add description and put InClass parameter as the first (more convenient with above methods)
	 * @return 
	 */
	FModularSectionActor InitModularSectionActor(const FTransform& InTransform, int32 InLevelId,
		TSubclassOf<AActor> InClass, bool bAddToActorSections, bool bWithRelativeTransform);

	/**
	 * Adds new instance to instanced modular section's instanced static mesh component.
	 * @param InTransform 
	 * @param bWithRelativeTransform
	 * @param InInstancedStaticMeshComponent 
	 */
	void AddNewInstance(const FTransform& InTransform, bool bWithRelativeTransform,
		UInstancedStaticMeshComponent* InInstancedStaticMeshComponent);

	/**
	 * Selects all modular sections of this modular build system actor.
	 */
	UFUNCTION(CallInEditor, BlueprintNativeEvent, Category = "ModularBuildSystem")
	void SelectAll();
	virtual void SelectAll_Implementation();
	
	/**
	 * Unselects all modular sections of this modular build system actor.
	 */
	UFUNCTION(CallInEditor, BlueprintNativeEvent, Category = "ModularBuildSystem")
	void UnselectAll();
	virtual void UnselectAll_Implementation();

	/**
	 * Initializes all modular levels.
	 * 
	 * @warning This function should not be called from a details panel if Generator object is set.
	 * In case Generator object is set - use Generate event instead of this.
	 */
	virtual void Init() override;

	/**
	 * Generates modular sections for this build system actor if Generator object is provided.
	 */
	UFUNCTION(CallInEditor, BlueprintNativeEvent, Category = "Generator")
	void Generate();
	virtual void Generate_Implementation();
	
#if WITH_EDITOR
	UFUNCTION(CallInEditor, BlueprintNativeEvent, Category = "Display")
	void ToggleShowOnlyInterior();
	virtual void ToggleShowOnlyInterior_Implementation();

	UFUNCTION(CallInEditor, Category = "Merge")
	void MergeIntoStaticMesh();

	UFUNCTION(CallInEditor, Category = "Merge")
	void UnmergeIntoModularSections();

	/**
	 * Fixes transforms of all modular sections of this build system actor
	 */
	UFUNCTION(CallInEditor, Category = "Manager")
	void FixModularSectionsTransform();

	void SelectSections(const FModularLevel& InLevel) const;
	void SelectSections(const TArray<FModularLevel>& InLevels) const;
	void UnselectSections(const FModularLevel& InLevel) const;
	void UnselectSections(const TArray<FModularLevel>& InLevels) const;

#endif

	/**
	 * Clears invalid sections and destroys all specified sections and/or section actors
	 * @param bResetSections 
	 * @param bResetActorSections 
	 * @param bResetInstancedSections
	 * @param bResetMergedSectionsStaticMeshActor
	 */
	virtual void ResetBuildSystem(bool bResetSections = true, bool bResetActorSections = true, bool bResetInstancedSections = true,
		bool bResetMergedSectionsStaticMeshActor = true) override;

	void RemoveSectionsOfLevel(int32 LevelId);
	void RemoveActorSectionsAfterIndex(int32 Index, int32 LevelId);
	void RemoveInstancedSectionsAfterIndex(int32 Index, int32 LevelId);
	void RemoveInstancedSectionInstancesAfterIndex(int32 Index, int32 LevelId);
	
	/**
	 * Checks if this MBS actor was just reset with ResetBuildSystem method.
	 * @return True if there was a reset.
	 * @see ResetBuildSystem
	 */
	bool WasReset() const { return Sections.WasReset(); }
	
	/**
	 * Gets this modular build system actor interior sub-object
	 * @return Pointer to an Interior sub-object.
	 */
	virtual UMBSInterior* GetInterior() const override { return Interior; }
	
	/**
	 * Gets this modular build system actor generated building stats.
	 * @return Reference to the stats structure.
	 * @see BuildStats
	 */
	virtual const FModularBuildStats& GetBuildStats() const override { return BuildStats; }

	/**
	 * Gets this modular build system actor transform bounds.
	 * @return Reference to the transform bounds structure.
	 * @see TransformBounds
	 */
	virtual const FMBSBounds& GetTransformBounds() const override { return TransformBounds; }
	
	virtual FMBSMeshConfiguration GetMeshConfiguration() const override { return MeshConfiguration; }
	bool IsOfInstancedMeshConfigurationType() const { return MeshConfiguration.IsOfInstancedType(); }

	virtual const FMBSSpawnConfiguration& GetSpawnConfiguration() const override { return SpawnConfiguration; }
	bool IsMerged() const { return Merger.bIsMerged; }

	const FMBSStretchManager& GetStretchManager() const { return StretchManager; }

	virtual const FMBSSections& GetSections() const override { return Sections; }
	TArray<FModularSection> GetStaticSections() const { return Sections.GetStatic(); }
	TArray<FModularSectionInstanced> GetInstancedSections() const { return Sections.GetInstanced(); }
	TArray<FModularSectionActor> GetActorSections() const { return Sections.GetActor(); }
	
	TArray<FModularSection*> GetSectionsOfLevel(const FModularLevel& InLevel) const;
	TArray<FModularSection*> GetSectionsOfLevel(const int32 InLevelId) const;

	TArray<FModularSectionActor*> GetActorSectionsOfLevel(const FModularLevel& InLevel) const;
	TArray<FModularSectionActor*> GetActorSectionsOfLevel(const int32 InLevelId) const;

	FModularSectionInstanced* GetInstancedSectionOfLevel(const FModularLevel& InLevel) const;
	FModularSectionInstanced* GetInstancedSectionOfLevel(const int32 InLevelId) const;

	/**
	 * Returns the bounds box based on the TransformBounds vector value adjusted by StretchManager scale coefficients.
	 */
	FBox GetBoundsBox(bool bRelative) const;
	
	/**
	 * Checks if provided section index is valid.
	 * @param InLevel Level to check.
	 * @param Index Index to check.
	 * @return True if index is valid, false otherwise.
	 */
	bool IsValidSectionIndex(const FModularLevel& InLevel, int32 Index) const;
	
	/**
	 * Offsets all modular sections associated with a provided level.
	 * @param Level Level to offset.
	 * @param Offset Offset from the relative position of a level.
	 * @param bUpdateZMultiplier Do update Z-axis multiplier of a Level's initializer?
	 * TODO: Move to UMBSFunctionLibrary?
	 */
	void OffsetModularLevel(const FModularLevel& Level, FVector Offset, bool bUpdateZMultiplier = false) const;

	// TODO: Move to UMBSFunctionLibrary?
	void OffsetModularSection(const FModularLevel& Level, int32 SectionIndex, FVector Offset, bool bRelative = true) const;
	
	/**
	 * 
	 * @param InLevel 
	 * @param InEachElement 
	 * @param InMesh
	 * TODO: Add description
	 */
	void SetMeshForEach(const FModularLevel& InLevel, int32 InEachElement, UStaticMesh* InMesh);

	/**
	 * Sets static mesh for a modular section at provided index (InElement) and row number (InRow).
	 * @param InLevel Level in which modular section should be found.
	 * @param InElement An element in a row (Index in row). 
	 * @param InRow Row number.
	 * @param InMesh Static mesh to set.
	 */
	void SetMeshAt(const FModularLevel& InLevel, int32 InElement, int32 InRow, UStaticMesh* InMesh);
	void SetMeshAt(const FModularLevel& InLevel, int32 InIndex, UStaticMesh* InMesh);

	/**
	 * Returns pointer to a Level's modular section at specified index.
	 * @return Pointer to a Level's modular section at specified index.
	 */
	FModularSection* GetSectionAt(const FModularLevel& InLevel, int32 InIndex) const;

	/**
	 * Returns pointer to a Level's modular section element at specified element index in row.
	 * @return Pointer to a Level's modular section at specified element index in row.
	 */
	FModularSection* GetSectionAt(const FModularLevel& InLevel, int32 InElement, int32 InRow) const;
	
	FTransform GetSectionTransformAt(int32 InLevelId, int32 InIndex, bool bInstanced, bool bWorldSpace) const;
	FTransform GetSectionTransformAt(const FModularLevel& InLevel, int32 InIndex, bool bInstanced, bool bWorldSpace) const;
	TArray<FTransform> GetSectionTransformAt(const FModularLevel& InLevel, TArray<int32> InIndices, bool bInstanced, bool bWorldSpace) const;
	FTransform GetSectionTransformAtRandom(int32 InLevelId, bool bInstanced, bool bWorldSpace) const;
	FTransform GetSectionTransformAtRandom(const FModularLevel& InLevel, bool bInstanced, bool bWorldSpace) const;

	virtual bool IsBuildModeActivated() const override { return bBuildModeIsActivated; }
	virtual void SetBuildModeActivated(const bool bActivated) override { bBuildModeIsActivated = bActivated; }
	
	EModularSectionReloadMode GetReloadMode() const { return MeshConfiguration.ReloadMode; }
	void SetReloadMode(const EModularSectionReloadMode NewReloadMode) { MeshConfiguration.ReloadMode = NewReloadMode; }
	
	/**
	 * @return All modular sections of this modular build system actor, including both actor and instanced sections.
	 */
	TArray<FModularSectionBase*> GetAllSections();
	
	/**
	 * Returns all levels of this modular build system actor. Should be overridden by subclasses.
	 * @return All levels of this modular build system actor.
	 */
	virtual TArray<FModularLevel*> GetAllLevels() const override
	{
		unimplemented();
		return TArray<FModularLevel*>();
	};

	/**
	 * Returns modular level struct with specified id.
	 * Is used to reset modular level instanced static mesh component on build system reset.
	 * @param Id Modular level id.
	 * @return Modular level struct with specified id
	 *
	 * @note Should be overridden by subclasses.
	 */
	virtual FModularLevel* GetLevelWithId(int32 Id) const override
	{
		unimplemented();
		return nullptr;
	};
	
	/**
	 * Returns modular building generator.
	 * Should be overridden by subclasses.
	 */
	virtual TScriptInterface<IBuildingGeneratorInterface> GetGenerator() const override
	{
		unimplemented();
		return nullptr;
	};
	
	virtual void AttachActor(AActor* ToAttach, bool bWithRelativeTransform, bool bForceReattach = false) override;
	virtual TScriptInterface<IMBSPresetManagerInterface> GetPresetManager() const override
	{
		unimplemented();
		return nullptr;
	}
	virtual FTransform GetBuildSystemTransform() const override { return GetActorTransform(); }
	virtual USceneComponent* GetRoot() const override { return GetRootComponent(); }
	
	/**
	 * Replaces instanced static mesh component instance at index with single modular section
	 * @param InNewStaticMesh Static mesh to set to the modular section that will 
	 * @param InInstanceIndex
	 * @param InLevelId
	 * @param InComponent
	 * @param OutReplacedInstanceTransform 
	 * @return
	 * TODO: Fill documentation
	 */
	FModularSection* ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh, int32 InInstanceIndex, int32 InLevelId,
		UInstancedStaticMeshComponent* InComponent, FTransform& OutReplacedInstanceTransform);
	FModularSection* ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh, int32 InInstanceIndex, int32 InLevelId,
		FTransform& OutReplacedInstanceTransform);

	/**
 	 * Gets location shift depending on SectionPivotLocation. E.g., if section pivot location is set to BottomRight
 	 * then all modular sections of this modular build system actor will be shifted by DefaultSectionSize (400.f) on Y-axis.
 	 * @param PivotLocation Section pivot location. If not provided - section pivot location of this MBS actor will be used.
 	 * @return Location shift vector.
 	 */
	FVector GetLocationShiftFromSectionPivot(
		EModularSectionPivotLocation PivotLocation = EModularSectionPivotLocation::Default) const;

protected:
	bool CanReload(const FTransform& CurrentTransform) const;
	
	/**
	 * Resets current modified level, e.g. level with id equal to ModifiedLevelId
	 * May be overridden by subclasses.
	 * TODO: Remove or implement
	 */
	virtual void ResetCurrentModifiedLevel();
	void ResetSingleLevel(int32 LevelId);
	
	/**
	 * Ends initialization of this modular build system actor.
	 */
	virtual void EndInit();

	/**
	 * Collects data from the modular levels and updates the build stats of this actor.
	 * Should be overridden by subclasses.
	 */
	virtual void CollectStats();
	
	/**
	 * Can be overriden to add logic on update of this modular build system actor's transform bounds.
	 * @see TransformBounds
	 */
	virtual void OnUpdateTransformBounds() {};
	
	/**
	 * Can be overriden to add logic on any change of bounds transforms (on move, rotate, e.t.c)
	 * @see TransformBounds
	 */
	virtual void OnBoundsTransformChange() {};

	/**
	 * Can be overriden to apply stretch of modular sections after their generation is succeeded.
	 */
	virtual void ApplyStretch() {};

	/**
	 * Toggles visibility of a single modular level.
	 * @param InLevel Level that should be updated.
	 * @param bVisible New visibility.
	 */
	void SetLevelVisibility(const FModularLevel& InLevel, bool bVisible) const;
	void SetLevelVisibility(const TArray<FModularLevel>& InLevels, bool bVisible) const;
	
	void UpdateMeshes(FModularLevel& InLevel);
	void UpdateMeshes(TArray<FModularLevel>& InLevels);

private:
	void RemoveSectionsAfterIndex(int32 Index, int32 LevelId);
	void ResetInstancedSectionOfLevel(int32 LevelId);
	void ClearInstancedSectionOfLevel(int32 LevelId);
	
	/**
	 * Reloads all section static meshes
	 * @param InLevelId 
	 */
	void ReloadMeshes(int32 InLevelId = FModularLevel::InvalidLevelId);
	
	/**
	 * Initializes modular sections.
	 * @param Args Initialization arguments.
	 * @return Initialized modular sections.
	 */
	TArray<FModularSection> InitModularSections(const FInitModularSectionsArgs& Args);

	/**
	 * Updates existing modular sections using initialization arguments.
	 * @param Args Initialization arguments.
	 */
	void UpdateModularSections(const FInitModularSectionsArgs& Args);
	
	/**
	 * Returns this modular build system actor transform adjusted by modular sections pivot location.
	 * @param PivotLocation (Optional) Section pivot location.
	 * If not provided - section pivot location of this MBS actor will be used.
	 * @return This modular build system actor transform adjusted by modular sections pivot location.
	 */
	FTransform GetAdjustedBuildSystemActorTransform(
		EModularSectionPivotLocation PivotLocation = EModularSectionPivotLocation::Default) const;

	/**
	 * Adjust existing sections using provided spline component.
	 * @param InSpline Pointer to spline component.
	 * TODO: Implement
	 */
	void AdjustSectionsUsingSpline(USplineComponent* InSpline);
	
	/**
	 * Initializes instanced modular sections.
	 * @param Args Initialization args.
	 * @return Array of initialized instanced modular sections.
	 */
	TArray<FModularSectionInstanced> InitInstancedModularSections(const FInitModularSectionsArgs& Args);

	/**
	 * Updates existing instanced modular sections using initialization arguments.
	 * @param Args Initialization arguments.
	 */
	void UpdateInstancedModularSections(const FInitModularSectionsArgs& Args);
	
	/**
	 * Append or update initialized sections to target array of section (sections/actor sections/instanced sections)
	 * @param Args Initialization arguments
	 * TODO: Rename to InitSections
	 */
	void AppendSections(const FInitModularSectionsArgs& Args);
	
	int32 GetFirstIndexOfSectionWithLevelId(const int32 InLevelId) const;
	int32 GetLastIndexOfSectionWithLevelId(const int32 InLevelId) const;
	
	void UpdateTransformBounds(const FPropertyChangedEvent& PropertyChangedEvent);
	void UpdateTransformBounds(const FPropertyChangedChainEvent& PropertyChangedEvent);

	// Friends
	friend struct FModularLevel;
	friend struct FMBSSections;
	friend class MBS::FActorDetails;
	friend class MBS::FModularLevelInitializer;
	friend class MBS::FModularLevelObserver;
	
};
