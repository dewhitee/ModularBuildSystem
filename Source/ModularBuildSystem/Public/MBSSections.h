// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemInterface.h"
#include "ModularSection.h"
#include "MBSSections.generated.h"

class AModularBuildSystemActor;

/**
 * Structure that holds all sections of a single modular build system actor, and provides methods to manipulate them.
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FMBSSections
{
	GENERATED_BODY()
	
private:	
	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, /*DuplicateTransient, */Category = "ModularBuildSystem")
	TArray<FModularSection> Static;

	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, /*DuplicateTransient, */Category = "ModularBuildSystem")
	TArray<FModularSectionActor> Actor;

	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, /*DuplicateTransient, */Category = "ModularBuildSystem")
	TArray<FModularSectionInstanced> Instanced;

	UPROPERTY()
	TScriptInterface<IModularBuildSystemInterface> BS = nullptr;
	
	FActorSpawnParameters SectionSpawnParams;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, /*DuplicateTransient, */Category = "ModularBuildSystem")
	int32 TotalInstanceCount = 0;
#endif

	UPROPERTY()
	bool bWasReset = false;

public:
	FMBSSections() {}
	FMBSSections(TScriptInterface<IModularBuildSystemInterface> InBuildSystemActor);
	
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

	TArray<FModularSection*> GetStaticSectionsOfLevel(const FModularLevel& InLevel) const;
	TArray<FModularSection*> GetStaticSectionsOfLevel(const int32 InLevelId) const;

	TArray<FModularSectionActor*> GetActorSectionsOfLevel(const FModularLevel& InLevel) const;
	TArray<FModularSectionActor*> GetActorSectionsOfLevel(const int32 InLevelId) const;

	FModularSectionInstanced* GetInstancedSectionOfLevel(const FModularLevel& InLevel) const;
	FModularSectionInstanced* GetInstancedSectionOfLevel(const int32 InLevelId) const;

	TArray<FModularSection> GetStatic() const { return Static; }
	TArray<FModularSectionActor> GetActor() const { return Actor; }
	TArray<FModularSectionInstanced> GetInstanced() const { return Instanced; }
	TArray<FModularSectionBase*> GetAll();

	void SelectSections(const FModularLevel& InLevel) const;
	void SelectSections(const TArray<FModularLevel>& InLevels) const;
	void UnselectSections(const FModularLevel& InLevel) const;
	void UnselectSections(const TArray<FModularLevel>& InLevels) const;
	
	void SelectAll();
	void UnselectAll();

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
 	 */
	void OffsetLevel(const FModularLevel& Level, FVector Offset, bool bUpdateZMultiplier = false) const;
	void OffsetSection(const FModularLevel& Level, int32 SectionIndex, FVector Offset, bool bRelative = true) const;
	
	void ResetSingleLevel(int32 LevelId);

	void Add(const FModularSection& InStatic) { Static.Add(InStatic); }
	void Add(const FModularSectionActor& InActor) { Actor.Add(InActor); }
	void Add(const FModularSectionInstanced& InInstanced) { Instanced.Add(InInstanced); }

	void Append(const TArray<FModularSection>& InStaticSections) { Static.Append(InStaticSections); }
	void Append(const TArray<FModularSectionActor>& InActorSections) { Actor.Append(InActorSections); }
	void Append(const TArray<FModularSectionInstanced>& InInstancedSections) { Instanced.Append(InInstancedSections); }
	
	bool IsAnyEmpty() const { return Static.IsEmpty() || Actor.IsEmpty() || Instanced.IsEmpty(); }
	bool IsAnyNotEmpty() const { return !Static.IsEmpty() || !Actor.IsEmpty() || !Instanced.IsEmpty(); }

	void EmptyStatic()		{ Static.Empty(); }
	void EmptyActor()		{ Actor.Empty(); }
	void EmptyInstanced()	{ Instanced.Empty(); }

	int32 GetFirstIndexOfSectionWithLevelId(const int32 InLevelId) const;
	int32 GetLastIndexOfSectionWithLevelId(const int32 InLevelId) const;

	void RemoveSectionsOfLevel(int32 LevelId);
	
	void RemoveSectionsAfterIndex(int32 Index, int32 LevelId);
	void RemoveActorSectionsAfterIndex(int32 Index, int32 LevelId);
	void RemoveInstancedSectionsAfterIndex(int32 Index, int32 LevelId);
	void RemoveInstancedSectionInstancesAfterIndex(int32 Index, int32 LevelId);

	void SetVisibility(bool bVisible);
	void SetVisibility(const FModularLevel& InLevel, bool bVisible) const;

	/**
	 * Reloads all section static meshes
	 * @param InLevelId 
	 */
	void ReloadMeshes(int32 InLevelId);

	/**
	 * Clears invalid sections and destroys all specified sections and/or section actors
	 * @param bResetSections 
	 * @param bResetActorSections 
	 * @param bResetInstancedSections
	 */
	void Reset(bool bResetSections, bool bResetActorSections, bool bResetInstancedSections);
	
	void ResetInstancedSectionOfLevel(int32 LevelId);
	void ClearInstancedSectionOfLevel(int32 LevelId);

	/**
	 * Removes all sections, actor sections and instanced sections that has nullptr static mesh, 
	 * actor or instanced static mesh component respectively.
	 */
	void ClearInvalidSections();

	/**
 	 * Initializes single modular section at the specified relative (to the build system actor) transform with specified static mesh.
 	 * @param InStaticMesh 
 	 * @param InTransform Relative to the modular build system actor transform. 
 	 * @param InLevelId 
 	 * @param bAddToSections 
 	 * @param bWithRelativeTransform 
 	 * @return
 	 * TODO: Add description to params and return value
 	 */
	FModularSection InitStatic(UStaticMesh* InStaticMesh, const FTransform& InTransform, int32 InLevelId,
		bool bAddToSections, bool bWithRelativeTransform);

	/**
	 * Initializes single modular section actor at the specified relative (to the build system actor) transform.
	 * @param InTransform Relative to the modular build system actor transform. 
	 * @param InLevelId 
	 * @param InClass
	 * @param bAddToActorSections
	 * @param bWithRelativeTransform
	 *
	 * TODO: Add description and put InClass parameter as the first (more convenient with above methods)
	 * @return 
	 */
	FModularSectionActor InitActor(const FTransform& InTransform, int32 InLevelId,
		TSubclassOf<AActor> InClass, bool bAddToActorSections, bool bWithRelativeTransform);

	/**
  	 * Initializes and adds new instanced static mesh component instance at the specified relative (to the build system actor) transform.
  	 * @param InLevelId 
  	 * @param bAddToInstancedSections 
  	 * @param InInstancedStaticMeshComponent
  	 * @return
  	 * TODO: complete documentation
  	 */
	FModularSectionInstanced InitInstanced(int32 InLevelId, bool bAddToInstancedSections, 
		UInstancedStaticMeshComponent* InInstancedStaticMeshComponent);

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
 	 * Adds new instance to instanced modular section's instanced static mesh component.
 	 * @param InTransform 
 	 * @param bWithRelativeTransform
 	 * @param InInstancedStaticMeshComponent 
 	 */
	void AddNewInstance(const FTransform& InTransform, bool bWithRelativeTransform,
		UInstancedStaticMeshComponent* InInstancedStaticMeshComponent);

	/**
	 * 
	 * @param InLevel 
	 * @param InEachElement 
	 * @param InMesh 
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
		
	void UpdateInstanced();
	/**
 	 * Updates instance count of instanced modular section.
 	 * @param InSection Instanced modular section to retrieve instance count from.
 	 */
	void UpdateInstanceCount(FModularSectionInstanced& InSection);
	
	AStaticMeshActor* SpawnNewSectionStaticMeshActor(const FTransform& InTransform, const FActorSpawnParameters& SpawnParams) const;
	AActor* SpawnNewSectionActor(const FTransform& InTransform, TSubclassOf<AActor> InClass, const FActorSpawnParameters& SpawnParams) const;

	bool WasReset() const { return bWasReset; }
	void SetWasReset(bool bValue) { bWasReset = bValue; }
	
};
