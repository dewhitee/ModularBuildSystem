// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "UObject/NoExportTypes.h"
#include "ModularLevelShape.generated.h"

class AModularBuildSystemActor;

UENUM(BlueprintType)
enum class EMBSSectionDirection : uint8
{
	Front,
	Right,
	Back,
	Left
};

USTRUCT(BlueprintType, Blueprintable)
struct FMBSShapeTransformArgs
{
	GENERATED_BODY()

	FMBSShapeTransformArgs();
	FMBSShapeTransformArgs(const int32 Index, const int32 AlreadySkippedCount, const FIntPoint Bounds,
		const FModularSectionInitializer* Initializer, AModularBuildSystemActor* BuildSystem, int32 LevelId,
		FTransform* AdjustedTransform, bool* bShouldBeSkipped)
		: InIndex(Index)
		, InMaxInRow(Initializer->GetMaxInRow())
		, InAlreadySkippedCount(AlreadySkippedCount)
		, InBounds(Bounds)
		, InInitializer(Initializer)
		, InBuildSystem(BuildSystem)
		, InLevelId(LevelId)
		, OutAdjustedTransform(AdjustedTransform)
		, bOutShouldBeSkipped(bShouldBeSkipped) {};

	mutable int32 InIndex;
	mutable int32 InMaxInRow; // Copy of InInitializer's MaxInRow
	
	// Read-only, does not need to be modified in ShapeTransform method
	int32 InAlreadySkippedCount;
	
	FIntPoint InBounds;
	const FModularSectionInitializer* InInitializer;
	TObjectPtr<AModularBuildSystemActor> InBuildSystem;
	int32 InLevelId;
	
	FTransform* OutAdjustedTransform;
	bool* bOutShouldBeSkipped;
};

USTRUCT(BlueprintType, Blueprintable)
struct FMBSPlaceRemainingActorsArgs
{
	GENERATED_BODY()

	FMBSPlaceRemainingActorsArgs()
		: InCountToSpawn(0)
		, InLevelId(0)
		, InZMultiplier(0.f)
		, InInitializer(nullptr)
		, InBuildSystem()
		, InSkippedIndices(nullptr) {};
	
	FMBSPlaceRemainingActorsArgs(const FIntPoint Bounds, const int32 DefaultCountToSpawn, const int32 LevelId,
		float ZMultiplier, const FModularSectionInitializer* Initializer, AModularBuildSystemActor* BuildSystem,
		const TArray<int32>& SkippedIndices)
		: InBounds(Bounds)
		, InCountToSpawn(DefaultCountToSpawn)
		, InLevelId(LevelId)
		, InZMultiplier(ZMultiplier)
		, InInitializer(Initializer)
		, InBuildSystem(BuildSystem)
		, InSkippedIndices(&SkippedIndices) {};
	
	FIntPoint InBounds;
	mutable int32 InCountToSpawn;
	mutable int32 InLevelId;
	float InZMultiplier;
	const FModularSectionInitializer* InInitializer;
	TObjectPtr<AModularBuildSystemActor> InBuildSystem;
	const TArray<int32>* InSkippedIndices;
};

USTRUCT(BlueprintType)
struct FMBSUpdateSectionInitializerArgs
{
	GENERATED_BODY()

	FMBSUpdateSectionInitializerArgs()
		: Initializer(nullptr) {};
	
	FMBSUpdateSectionInitializerArgs(FModularSectionInitializer& Initializer)
		: Initializer(&Initializer) {};

	FModularSectionInitializer* Initializer;
};

/**
 * Abstract base class of all modular building level shapes.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class MODULARBUILDSYSTEM_API UModularLevelShape : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Adjusts provided transform to fit the target shape.
	 * 
	 * @note Use BreakShapeTransformArgs to get structure arguments.
	 * @note Use SetOutParams function to set out arguments, set by reference directly or use helper functions.
	 * 
	 * @see FMBSShapeTransformArgs
	 * @see AddLocationToOutTransform
	 * @see AddRotationToOutTransform
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	void ShapeTransform(const FMBSShapeTransformArgs& Args);
	virtual void ShapeTransform_Implementation(const FMBSShapeTransformArgs& Args);
	
	/**
	 * Spawns and places all remaining actors (if there is any) of a single modular level
	 * 
	 * @see FMBSPlaceRemainingActorsArgs
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	void PlaceRemainingActors(const FMBSPlaceRemainingActorsArgs& Args);
	virtual void PlaceRemainingActors_Implementation(const FMBSPlaceRemainingActorsArgs& Args);

	/**
	 * Updates necessary parameters of a provided modular section initializer struct to fit the shape.
	 * Supposed to be called before ShapeTransform or PlaceRemainingActors, to prepare necessary data.
	 *
	 * @note Experimental. This function may be removed later if PlaceRemainingActors will be sufficient.
	 * @see PlaceRemainingActors
	 * 
	 * @param Args List of arguments
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	void UpdateSectionInitializer(const FMBSUpdateSectionInitializerArgs& Args);
	virtual void UpdateSectionInitializer_Implementation(const FMBSUpdateSectionInitializerArgs& Args);

	/**
	 * Checks if all data that is necessary for the ShapeTransform method is set.
	 * @return True if object is valid for ShapeTransform method to run.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	bool IsDataPrepared() const;
	virtual bool IsDataPrepared_Implementation() const
	{
		unimplemented();
		return false;
	}

	/**
	 * Checks if all conditions are met for the ShapeTransform method to run successfully.
	 * @param Args List of arguments.
	 * @return True if all conditions are met.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	bool CanShapeTransform(const FMBSShapeTransformArgs& Args) const;
	virtual bool CanShapeTransform_Implementation(const FMBSShapeTransformArgs& Args) const
	{
		unimplemented();
		return false;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	int32 GetShapedMaxIndexX(const FIntPoint Bounds, int32 IndexY) const;
	virtual int32 GetShapedMaxIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const
	{
		ensureMsgf(false, TEXT("%s: GetShapedMaxIndexX is not overrided."), *GetName());
		return Bounds.X - 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	int32 GetShapedMaxIndexY(const FIntPoint Bounds, int32 IndexX) const;
	virtual int32 GetShapedMaxIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const
	{
		ensureMsgf(false, TEXT("%s: GetShapedMaxIndexY is not overrided."), *GetName());
		return Bounds.Y - 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	int32 GetShapedMinIndexX(const FIntPoint Bounds, int32 IndexY) const;
	virtual int32 GetShapedMinIndexX_Implementation(const FIntPoint Bounds, int32 IndexY) const
	{
		ensureMsgf(false, TEXT("%s: GetShapedMinIndexX is not overrided."), *GetName());
		return 0;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	int32 GetShapedMinIndexY(const FIntPoint Bounds, int32 IndexX) const;
	virtual int32 GetShapedMinIndexY_Implementation(const FIntPoint Bounds, int32 IndexX) const
	{
		ensureMsgf(false, TEXT("%s: GetShapedMinIndexY is not overrided."), *GetName());
		return 0;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	bool HasFrontIndicesInverted() const;
	virtual bool HasFrontIndicesInverted_Implementation() const
	{
		return false;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Shape)
	bool SkipsSections() const;
	virtual bool SkipsSections_Implementation() const
	{
		return false;
	}

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape, meta = (NativeBreakFunc))
	static void BreakShapeTransformArgs(const FMBSShapeTransformArgs& InArgs, int32& InIndex, FIntPoint& InBounds,
		FModularSectionInitializer& InInitializer, FTransform& OutAdjustedTransform, bool& bOutShouldBeSkipped);
	
	UFUNCTION(BlueprintCallable, Category=Shape)
	static void OffsetIndex(const FMBSShapeTransformArgs& InArgs, int32 Offset);

	UFUNCTION(BlueprintCallable, Category=Shape)
	static void OffsetMaxInRow(const FMBSShapeTransformArgs& InArgs, int32 Offset);

	UFUNCTION(BlueprintCallable, Category=Shape)
	static void SetOutParams(UPARAM(ref) FMBSShapeTransformArgs& InArgs, const FTransform& OutAdjustedTransform,
		const bool bOutShouldBeSkipped);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (CompactNodeTitle="Add"))
	static void AddLocationToOutTransform(const FMBSShapeTransformArgs& InArgs, const FVector Location);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (CompactNodeTitle="Add"))
	static void AddLocationXToOutTransform(const FMBSShapeTransformArgs& InArgs, const float X);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (CompactNodeTitle="Add"))
	static void AddLocationYToOutTransform(const FMBSShapeTransformArgs& InArgs, const float Y);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (CompactNodeTitle="Add"))
	static void AddLocationZToOutTransform(const FMBSShapeTransformArgs& InArgs, const float Z);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (CompactNodeTitle="Add"))
	static void AddRotationToOutTransform(UPARAM(ref) FMBSShapeTransformArgs& InArgs, const FRotator Rotation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsWallInMiddle(const FMBSShapeTransformArgs& InArgs);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsFrontSectionIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape, meta = (CompactNodeTitle="Is back section"))
	static bool IsBackDefaultSectionIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape, meta = (CompactNodeTitle="Is back section"))
	static bool IsBackWallSectionIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsRightSectionIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsRightSectionLastIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsRightSectionFirstIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsLeftSectionIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsLeftSectionLastIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static bool IsLeftSectionFirstIndex(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static void ExcludeAlreadySkipped(const FMBSShapeTransformArgs& InArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static int32 GetWallLeftFirstIndex();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Shape)
	static int32 GetWallRightFirstIndex(const FMBSShapeTransformArgs& InArgs);
	// TODO: Expose to blueprints
	static int32 GetWallRightFirstIndex(const FMBSPlaceRemainingActorsArgs& InArgs);
	static int32 GetWallRightLastIndex(const FMBSPlaceRemainingActorsArgs& InArgs);

	UFUNCTION(BlueprintCallable, Category=Shape, meta = (ExpandEnumAsExecs="Direction"))
	static void SwitchSectionDirection(const FMBSShapeTransformArgs& InArgs, EMBSSectionDirection& Direction,
		FMBSShapeTransformArgs& OutArgs);
	
};
