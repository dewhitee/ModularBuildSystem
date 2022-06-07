// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSectionResolution.h"
#include "UObject/NoExportTypes.h"
#include "MBSTransformSolver.generated.h"

/**
 * Base abstract class for transform solver objects.
 * 
 * @note Supposed to be used when generating non-rectangular shapes, where default snapping modes of
 * UModularSectionResolution class do not fit well.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MODULARBUILDSYSTEM_API UMBSTransformSolver : public UObject
{
	GENERATED_BODY()

protected:
	FNextTransformArgs* Arguments;
	
public:
	/**
	 * Calculate next transform based on the input arguments.
	 * @param Args Arguments
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=MBS)
	FTransform GetNextTransform(UPARAM(Ref) FNextTransformArgs& Args);
	virtual FTransform GetNextTransform_Implementation(FNextTransformArgs& Args)
	{
		unimplemented();
		return {};
	}

protected:
	/**
	 * Sets the Arguments property of this transform solver.
	 * @note This is necessary inside Blueprint implemented GetNextTransform methods, as it gives an ability to not pass
	 * FNextTransformArgs structure into every node.
	 * 
	 * @param Args Arguments passed in GetNextTransform method
	 */
	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true))
	void In(UPARAM(Ref) FNextTransformArgs& Args);
	
	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	FTransform Out();
	static FTransform Out(const FNextTransformArgs& Args);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="Index", HideSelfPin=true, DefaultToSelf))
	int32 GetIndex() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="MaxInRow", HideSelfPin=true, DefaultToSelf))
	int32 GetMaxInRow() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="MaxCount", HideSelfPin=true, DefaultToSelf))
	int32 GetMaxCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="LevelZMultiplier", HideSelfPin=true, DefaultToSelf))
	float GetLevelZMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="BuildStats", HideSelfPin=true, DefaultToSelf))
	FModularBuildStats GetBuildStats() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="PreviousLevelResolution", HideSelfPin=true, DefaultToSelf))
	UModularSectionResolution* GetPreviousLevelResolution() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="Resolution", HideSelfPin=true, DefaultToSelf))
	UModularSectionResolution* GetResolution() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="AdjustedIndex", HideSelfPin=true, DefaultToSelf))
	int32 GetAdjustedIndex() const;
	static int32 GetAdjustedIndex(int32 Index, int32 MaxInRow);
	int32 GetAdjustedIndex(const FModularBuildStats& BuildStats) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="MaxRow", HideSelfPin=true, DefaultToSelf))
	int32 GetMaxRow() const;
	static int32 GetMaxRow(int32 MaxCount, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="CurrentRow", HideSelfPin=true, DefaultToSelf))
	int32 GetCurrentRow() const;
	static int32 GetCurrentRow(int32 Index, int32 MaxInRow);
	int32 GetCurrentRow(const FModularBuildStats& BuildStats) const;
	static int32 GetWallCurrentRow(int32 Index, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	bool IsFacingFront() const;
	static bool IsFacingFront(int32 AdjustedIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	bool IsFacingBack() const;
	static bool IsFacingBack(int32 AdjustedIndex, int32 MaxInRow);
	bool IsFacingBack(const FModularBuildStats& BuildStats) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	bool IsFacingLeft() const;
	static bool IsFacingLeft(int32 CurrentRow);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	bool IsFacingRight() const;
	static bool IsFacingRight(int32 CurrentRow, int32 MaxCount, int32 MaxInRow);
	bool IsFacingRight(const FModularBuildStats& BuildStats) const;

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Location", HideSelfPin=true, DefaultToSelf))
	void AddLocation(FVector ToAdd);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add X", HideSelfPin=true, DefaultToSelf))
	void AddLocationX(float X);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Y", HideSelfPin=true, DefaultToSelf))
	void AddLocationY(float Y);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Z", HideSelfPin=true, DefaultToSelf))
	void AddLocationZ(float Z);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Rotation", HideSelfPin=true, DefaultToSelf))
	void AddRotation(FRotator ToAdd);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add X (Roll)", HideSelfPin=true, DefaultToSelf))
	void AddRotationRoll(float Roll);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Y (Pitch)", HideSelfPin=true, DefaultToSelf))
	void AddRotationPitch(float Pitch);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle="Add Z (Yaw)", HideSelfPin=true, DefaultToSelf))
	void AddRotationYaw(float Yaw);

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	void Skip();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="DefaultSectionSize"))
	static float GetDefaultSectionSize();

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	void MirrorByX() const;

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	void MirrorByY() const;

	UFUNCTION(BlueprintCallable, Category=MBS, meta=(CompactNodeTitle, HideSelfPin=true, DefaultToSelf))
	void MirrorByZ() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="IndexEquals", HideSelfPin=true))
	bool IsIndexEquals(int32 Index) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="IndexEquals", HideSelfPin=true))
	bool IsIndexEqualsAnyOf(TArray<int32> Indices);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="AdjustedIndexEquals", HideSelfPin=true))
	bool IsAdjustedIndexEquals(int32 Index) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=MBS, meta=(CompactNodeTitle="AdjustedIndexEquals", HideSelfPin=true))
	bool IsAdjustedIndexEqualsAnyOf(TArray<int32> Indices);

private:
	inline bool CheckArgs() const;
};
