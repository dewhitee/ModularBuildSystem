// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "ModularLevel.generated.h"

namespace MBS
{
class FModularLevelBuilder;
}

class IModularBuildSystemInterface;
class UModularBuildSystemMeshList;
class UModularSectionResolution;
class AModularBuildSystemActor;
class UModularLevelShape;

/**
 * Single level of a building. All modular sections of modular build system actor are associated with
 * one of the modular levels.
 *
 * This struct should be used to separate distinct parts on a MBS actor. For example, House actor would have levels such as:
 * Basement, Floors (array of levels), Walls (array of levels), Roof, e.t.c
 * 
 * @note There can be multiple levels on the same height (on the same floor). This is useful when adding modular building level selection functionality,
 * or when there is a clear visual or logical distinction between sections (modules) on a specific level of a building.
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FModularLevel
{
	GENERATED_BODY()
	
	/**
	 * Level id that should be assigned to the invalid or not yet initialized levels.
	 */
	static constexpr int32 InvalidLevelId = -1;
	
	/**
	 * Pointer to instanced static mesh component associated with this modular level. Currently can only be set from Generators.
	 * TODO: Should be removed in favour of instanced sections
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent = nullptr;
	
private:
	/**
	 * Modular section initializer of this modular level. Is used to initialize new modular sections or
	 * instanced modular sections.
	 */
	UPROPERTY(EditAnywhere, Category = "ModularBuildSystem", meta=(EditCondition="!bSkipThisLevel"))
	FModularSectionInitializer Initializer;
	
	/**
	 * Custom shape of a modular level. Currently can only be set from Generators.
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem", meta=(DisplayThumbnail=false))
	TObjectPtr<UModularLevelShape> CustomShape = nullptr;

	/**
	 * Unique (within a system) id used in reload of a specific level's sections.
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	int32 Id = InvalidLevelId;

	/**
	 * Level/floor of a building. Currently only used for debug.
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	uint8 LevelIndex = 0u;

	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	float ZMultiplier = 0.f;

	/**
	 * Set this to true in case you want to skip initialization of this building layer.
	 */
	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem")
	bool bSkipThisLevel = false;

	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem", meta = (EditCondition = "bSkipThisLevel"))
	EModularSectionSkipMode SkipMode = EModularSectionSkipMode::Hide;

	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem")
	EModularSectionPivotLocation PivotLocation = EModularSectionPivotLocation::Default;

	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem")
	TObjectPtr<UMBSTransformSolver> Solver = nullptr;

	/**
	 * If true - this modular level will not be affected by reload.
	 * TODO: more doc
	 */
	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem")
	bool bLock = false;

	UPROPERTY(EditInstanceOnly, Category = "ModularBuildSystem")
	bool bAffectsNextZMultiplier = true;
	
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	bool bUpdated = false;
	
#if WITH_EDITORONLY_DATA
	/**
	 * A unique name for debug only.
	 */
	UPROPERTY(VisibleAnywhere, Category = "ModularBuildSystem")
	FName Name = {};
#endif
	
	static constexpr uint8 MaxLevelIndex = /*16u*/UINT8_MAX;
	static constexpr float MaxLevelZMultiplier = /*16.f*/UINT8_MAX;
	
public:
	/**
	 * Initializes this modular level, set up modular sections (or instanced modular sections, depending on MeshConfiguration
	 * of modular build system actor).
	 * @param InFallbackDebugName Fallback name of a level (For log only).
	 * @param InSystem This level owner modular build system actor.
	 * @param InNewId New level id to set.
	 * @param InNewLevelIndex New level index to set.
	 * @param InShiftCount Total count of level shifts performed. Will be modified if this level need to be skipped.
	 * @param InNewZMultiplier New height multiplier of a modular level.
	 * @param InPreviousLevelResolution Pointer to a previous level resolution (resolution of a level that is under the current level).
	 * Can be left nullptr if there is no modular level under this.
	 * @param InAppendFunction Function that should be used to append initialized building sections to the array of build system sections.
	 * @return True if modular level (including ) was successfully initialized.
	 */
	bool Init(const FName& InFallbackDebugName, AModularBuildSystemActor* InSystem, int32 InNewId, uint8 InNewLevelIndex,
		uint8& InShiftCount, float InNewZMultiplier, UModularSectionResolution* InPreviousLevelResolution,
		TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction,
		void (AModularBuildSystemActor::* InAppendMethod)(const FInitModularSectionsArgs&));

	int32 GetId() const { return Id; }
	const FModularSectionInitializer& GetInitializer() const { return Initializer; }
	UModularLevelShape* GetShape() const { return CustomShape; }
	float GetZMultiplier() const { return ZMultiplier; }
	EModularSectionPivotLocation GetPivotLocation() const { return PivotLocation; }
	UMBSTransformSolver* GetSolver() const { return Solver; }

	/**
	 * Checks if this modular level's Initializer has valid Resolution and StaticMesh, total count is more than 0
	 * and if this level should not be skipped.
	 * @return True if modular section resolution and static mesh properties are not null, total count is more than 0
	 * and if this modular level should not be skipped.
	 */
	bool IsValid() const;
	
	/**
	 * Gets Z-axis multiplier for next (above) modular level.
	 * @param PreviousMultiplier Previous Z (up axis) multiplier. Might be the Z multiplier of this modular level or
	 * modular level below this one.
	 * @return Z-axis multiplier for next level.
	 */
	float GetZMultiplierForNextLevel(float PreviousMultiplier) const;

	/**
	 * Gets world transform of this modular level.
	 * @param InBuildSystem Build system actor this modular level is associated with.
	 * @return World transform of this modular level.
	 */
	FTransform GetWorldTransform(const IModularBuildSystemInterface* InBuildSystem) const;
	
	/**
	 * Gets world transform of this modular level.
	 * @param InBuildSystemTransform World transform of build system actor this modular level is associated with.
	 * @return World transform of this modular level.
	 */
	FTransform GetWorldTransform(FTransform InBuildSystemTransform) const;

	/**
	 * Returns this modular level transform relative to the InBuildSystem actor transform.
	 * @param InBuildSystem This modular level owner - modular build system actor.
	 * @return Transform relative to the InBuildSystem actor transform.
	 */
	FTransform GetRelativeTransform(const IModularBuildSystemInterface* InBuildSystem) const;

	/**
	 * Returns this modular level transform relative to the InBuildSystemTransform 
	 * (supposed to be the transform of modular build system actor).
	 * @param InBuildSystemTransform Modular build system actor transform.
	 * @return Transform relative to the InBuildSystemTransform (modular build system actor transform).
	 */
	FTransform GetRelativeTransform(const FTransform& InBuildSystemTransform) const;

	/**
	 * Gets resolution vector of this modular level's modular section resolution object.
	 * @return Resolution integer vector.
	 */
	FIntVector GetResolutionVec() const;

	/**
	 * Gets debug name of this modular level.
	 * @return Debug name of this level.
	 */
	FString GetName() const;

	/**
	 * Accessor of bIsUpdated.
	 * @see bIsUpdated
	 */
	bool IsUpdated() const;

	/**
	 * Sets all data of this modular level to it's defaults but keeping it's debug name.
	 * @see Name
	 */
	void Invalidate();

	// TODO: Move to the separate Builder class to not bloat this one?
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

	static bool IsValidLevelId(const int32 InLevelId) { return InLevelId != InvalidLevelId; }
	bool OverrideLevelId(int32 NewId);

private:
	void OverrideLevelIndex(uint8 NewLevelIndex, uint8 MinLevel = 0u, uint8 MaxLevel = MaxLevelIndex);
	void OverrideLevelZMultiplier(float NewZMultiplier, float Min = 0.f, float Max = MaxLevelZMultiplier);

	inline void MarkUpdated(bool bCondition);

	bool MustBeSkippedWithShift() const;

	void CreateNewInstancedStaticMeshComponent(TSubclassOf<UInstancedStaticMeshComponent> ComponentClass,
		TScriptInterface<IModularBuildSystemInterface> InBuildSystem, UStaticMesh* InStaticMesh);

	static void InitAppend(TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction,
		void (AModularBuildSystemActor::* InAppendMethod)(const FInitModularSectionsArgs&),
		AModularBuildSystemActor* InSystem, const FInitModularSectionsArgs& InArgs);

	friend class MBS::FModularLevelBuilder;
};

