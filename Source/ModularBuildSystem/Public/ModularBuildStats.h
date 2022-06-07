// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildStats.generated.h"

class UModularLevelShape;
/**
 * 
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FModularBuildStats
{
	GENERATED_BODY()

	FModularBuildStats() {}
	FModularBuildStats(FIntPoint InBounds, int32 InMaxTotalCount, int32 InMaxTotalRows, int32 InMaxCountInRow)
		: Bounds(InBounds)
		, MaxTotalCount(InMaxTotalCount)
		, MaxTotalRows(InMaxTotalRows)
		, MaxCountInRow(InMaxCountInRow) { }

	/**
	 * @brief Bounds of a modular building.
	 */
	UPROPERTY(VisibleAnywhere, Category = "BuildStats")
	FIntPoint Bounds = {};

	/**
	 * Total count of modular levels of modular build system actor.
	 */
	UPROPERTY(VisibleAnywhere, Category="BuildStats")
	int32 LevelCount = 0;

	/**
	 * @brief Largest total count of sections found in this modular build system levels.
	 */
	UPROPERTY(VisibleAnywhere, Category = "BuildStats")
	int32 MaxTotalCount = 0;

	/**
	 * @brief Largest total count of rows found in this modular build system levels.
	 */
	UPROPERTY(VisibleAnywhere, Category = "BuildStats")
	int32 MaxTotalRows = 0;

	/**
	 * @brief Largest count of sections found in row of a single modular level in this build system.
	 */
	UPROPERTY(VisibleAnywhere, Category = "BuildStats")
	int32 MaxCountInRow = 0;

	UPROPERTY(VisibleAnywhere, Category = "BuildStats")
	bool bInitialized = false;

	void Init(FModularBuildStats NewBuildStats = FModularBuildStats());
	void Clear();
	void Update(const FModularBuildStats& InBuildStats);
	void OverrideBounds(FIntPoint NewBounds);
	void OverrideLevelCount(int32 InLevelCount);

	FIntVector GetBoundsVector() const { return FIntVector(Bounds.X, Bounds.Y, LevelCount); }

	/**
	 * Gets max X-axis section index for provided Y-axis section index from bounds shaped with CustomShape object.
	 * If CustomShape is not provided this function will return BuildStats.X value.
	 * @param Shape Custom shape object (from a ModularLevel in most cases).
	 * @param IndexY Y-axis section index (RowIndex).
	 * @return Max X-axis section index.
	 */
	int32 GetMaxSectionIndexX(const UModularLevelShape* Shape, int32 IndexY) const;

	/**
	 * Gets max Y-axis section index for provided X-axis section index from bounds shaped with CustomShape object.
	 * If CustomShape is not provided this function will return BuildStats.Y value.
	 * @param Shape Custom shape object (from a ModularLevel in most cases).
	 * @param IndexX X-axis section index (RowIndex).
	 * @return Max Y-axis section index.
	 */
	int32 GetMaxSectionIndexY(const UModularLevelShape* Shape, int32 IndexX) const;

	/**
	 * Gets min X-axis section index for provided Y-axis section index from bounds shaped with Shape object.
	 * If Shape is not provided this function will return zero.
	 * @param Shape Custom shape object (from a ModularLevel in most cases).
	 * @param IndexY Y-axis section index (RowIndex).
	 * @return Min X-axis section index.
	 */
	int32 GetMinSectionIndexX(const UModularLevelShape* Shape, int32 IndexY) const;

	/**
	 * Gets min Y-axis section index for provided X-axis section index from bounds shaped with Shape object.
	 * If Shape is not provided this function will return zero.
	 * @param Shape Custom shape object (from a ModularLevel in most cases).
	 * @param IndexX X-axis section index (RowIndex).
	 * @return Min Y-axis section index.
	 */
	int32 GetMinSectionIndexY(const UModularLevelShape* Shape, int32 IndexX) const;
	
	FString ToString() const
	{
		return FString::Printf(TEXT("{Bounds=%s, LevelCount=%d, MaxTotalCount=%d, MaxTotalRows=%d, MaxCountInRow=%d, bInitialized=%s}"),
			*Bounds.ToString(), LevelCount, MaxTotalCount, MaxTotalRows, MaxCountInRow, bInitialized ? TEXT("true") : TEXT("false"));
	}
};
