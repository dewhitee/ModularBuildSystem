// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteriorGeneratorInterface.h"
#include "UObject/NoExportTypes.h"
#include "MBSGeneratorBase.h"
#include "MBSRoom.h"
#include "MBSInteriorGenerator.generated.h"

struct FModularLevel;
class UMBSInteriorPropList;
class UModularBuildSystemMeshList;
class ASkeletalMeshActor;

UENUM(BlueprintType)
enum class EMBSInteriorPlacementMode : uint8
{
	Default,
	UsingBoundingBox
};

USTRUCT(BlueprintType)
struct FMBSInteriorGeneratorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	bool bAdjustTransformIfOverlap = true;
	
	UPROPERTY(EditAnywhere, Category=Settings, meta = (EditCondition="bAdjustTransformIfOverlap"))
	bool bSkipIfStillOverlap = false;

	UPROPERTY(EditAnywhere, Category=Settings, meta = (ClampMin=1, ClampMax=64, EditCondition="bAdjustTransformIfOverlap"))
	int32 MaxAdjustTransformTryCount = 10;

	UPROPERTY(EditAnywhere, Category=Settings)
	bool bKeepBoundWallsSingle = false;

	UPROPERTY(EditAnywhere, Category=Settings, meta=(EditCondition="bKeepBoundWallsSingle"))
	bool bKeepTopAndRightBoundWalls = true;

	/**
 	 * Area in which interior actors can be spawned.
 	 */
	UPROPERTY(EditAnywhere, Category=Settings, meta=(ClampMin=0.52f, ClampMax=1))
	float InteriorAllowedAreaRatio = .9f;

	UPROPERTY(EditAnywhere, Category=Settings)
	FTransform InteriorAreaOffsetTransform = {};
};

/**
 * Do not use. Deprecated in favor of FInt32Range
 */
USTRUCT(BlueprintType)
struct FMBSInteriorPropCountRange_DEPRECATED
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MBS", meta = (ClampMin=0, ClampMax=256))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MBS", meta = (ClampMin=0, ClampMax=256))
	int32 MaxCount = 1;
};


USTRUCT(BlueprintType)
struct FMBSInteriorGeneratorModularLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generator")
	TObjectPtr<UMBSInteriorPropList> PropList = nullptr;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator")
	TMap<UStaticMesh*, FInt32Range> StaticMeshes;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator")
	TMap<USkeletalMesh*, FInt32Range> SkeletalMeshes;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator")
	TMap<TSubclassOf<AActor>, FInt32Range> Actors;
};

/**
 * This class handles creation of modular build system actor interior, including stairs, large and small prop actors.
 * 
 * @note Supposed to be extended by subclasses and used as a uproperty inside ModularBuildSystemActor derived classes or
 * MBSInterior subobjects of ModularBuildSystemActor instances.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class MODULARBUILDSYSTEM_API UMBSInteriorGenerator : public UMBSGeneratorBase, public IInteriorGeneratorInterface
{
	GENERATED_BODY()
	
protected:	
	UPROPERTY(EditAnywhere, Category=Generator)
	FMBSInteriorGeneratorSettings Settings;
	
public:
	// IInteriorGeneratorInterface
	virtual FGeneratedInterior Generate_Implementation() override;
	virtual void Update_Implementation() override;
	virtual void ApplyPresets_Implementation() override;
	
	/**
	 * Excludes level's outline (walls or roof, for example) from box extent.
	 * @param BoxToAdjust Box to adjust.
	 * @param LevelOutlineResolution Resolution of a level outline. E.g. exterior walls or roof.
	 */
	void AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust, const UModularSectionResolution* LevelOutlineResolution) const;
	void AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust, float AllowedAreaRatio, const UModularSectionResolution* LevelOutlineResolution) const;
	static void AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust, float AllowedAreaRatio, const FTransform& OffsetTransform, const UModularSectionResolution* LevelOutlineResolution);
	
protected:
	/**
	 * Calculates new transform for next interior actor in provided room.
	 * Should be overridden by subclasses.
	 * @param InRoom Current room to calculate new transform in.
	 * @return New calculated transform.
	 */
	virtual FTransform CalculateNewTransform(const FMBSRoom& InRoom) const
	{
		unimplemented();
		return {};
	};
	
	/**
	 * Prepares generator before generating new interior. Should be overridden by subclasses.
	 */
	virtual void Prepare()
	{
		unimplemented();
	};

	/**
	 * Adds static, skeletal and other actors from interior data prop list to interior data arrays if necessary.
	 * @param InInteriorData Interior data to prepare.
	 */
	static void PrepareInteriorData(FMBSInteriorGeneratorModularLevelData& InInteriorData);
	
	/**
	 * Empties interior data arrays.
	 * @param InInteriorData Interior data to clear.
	 */
	static void ClearInteriorData(FMBSInteriorGeneratorModularLevelData& InInteriorData);
	
	/**
	 * Checks if actor A overlaps with actor B.
	 * @param InteriorActorA Actor A.
	 * @param InteriorActorB Actor B.
	 * @return True if InteriorActorA overlaps with InteriorActorB.
	 */
	//[[deprecated]] bool IsInteriorActorOverlaps(AActor* InteriorActorA, AActor* InteriorActorB) const;
	
	/**
	 * Checks if InteriorActor overlaps with any actor from OtherActors array.
	 * @param InteriorActor Actor to check.
	 * @param OtherActors Array of actors to check for overlap with InteriorActor.
	 * @return True if InteriorActor overlaps with any actor from OtherActors array.
	 */
	bool IsInteriorActorOverlapsAny(AActor* InteriorActor, const TArray<AActor*> OtherActors) const;
	
	void AdjustInteriorActorTransform(AActor* InteriorActor, const FInteriorLevel& LevelInterior,
		const FMBSRoom& InRoom, int32 MaxTryCount, bool& bShouldSkip) const;
	
	void AdjustTransformIfOverlapsAny(AActor* ActorToAdjust, const TArray<AActor*> OtherActors, const FMBSRoom& InRoom,
		int32 MaxTryCount, bool& bStillOverlaps) const;
};
