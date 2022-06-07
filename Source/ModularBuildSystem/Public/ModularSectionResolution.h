// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildStats.h"
#include "Engine/DataAsset.h"
#include "ModularSectionResolution.generated.h"

struct FModularLevel;
class UModularSectionResolution;
class USplineComponent;
class UMBSTransformSolver;

UENUM(BlueprintType)
enum class EModularSectionResolutionSnapMode : uint8
{
	/**
	 * Default snapping. Is used for basements, floors and all other sections that don't require specific rotation or location adjustment.
	 */
	Default,

	/**
	 * Wall snapping. Distributes all sections on the outer boundary.
	 */
	Wall,

	/**
	 * Roof snapping. Distributes all sections on the outer boundary. Similar to Wall snapping.
	 */
	Roof,

	/**
	 * Rooftop snapping. Distributes all sections relatively to the roof underneath.
	 */
	Rooftop,

	/**
	 * Corner snapping. Distributes sections only on the corners of the building.
	 */
	Corner,

	/**
	 * Custom snapping. Can be used in case existing snapping modes (that form rectangular shape) do not fit the
	 * requirements of a target shape.
	 *
	 * Supposed to be used when generating, for instance, circular or non-rectangular shape, as default snapping modes
	 * would result in too many redundant sections.
	 */
	Custom,

};

UENUM(BlueprintType)
enum class EMBSResolutionAlgorithmGeneration : uint8
{
	// Legacy. Works well for cuboid (default) shape, but is hard to shape in custom forms.
	Gen2,
	
	// New algorithm that has better placement flow, more predictable and easier to shape in custom form than Gen2.
	Gen3
};

// TODO: Remove
UENUM(BlueprintType)
enum class EModularSectionRelativePosition : uint8
{
	Front,
	Right,
	Back,
	Left,
};

/**
 * Structure that holds arguments passed to the GetNext*Transform methods.
 */
USTRUCT(BlueprintType)
struct FNextTransformArgs
{
	GENERATED_BODY()

	FNextTransformArgs();

	FNextTransformArgs(const FTransform& Transform, int32 Index, int32 MaxInRow, int32 MaxCount, float LevelZMultiplier,
		const FModularBuildStats& BuildStats, const UModularSectionResolution* PreviousLevelResolution,
		const UModularSectionResolution* Resolution, UMBSTransformSolver* Solver);

	FNextTransformArgs(const FTransform& Transform, int32 Index, const FModularLevel& Level,
		const FModularBuildStats& BuildStats, const UModularSectionResolution* PreviousLevelResolution,
		USplineComponent* Spline);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FTransform InTransform = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	int32 InIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	int32 InMaxInRow = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	int32 InMaxCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	float InLevelZMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FModularBuildStats InBuildStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	const UModularSectionResolution* InPreviousLevelResolution = nullptr;

	/**
	 * Const pointer to the Resolution used in the calculation.
	 *
	 * @note Is required by UMBSTransformSolver objects.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	const UModularSectionResolution* InResolution = nullptr;

	/**
	 * Currently not in use.
	 * TODO: Maybe remove later, as it is handled only inside UModularSectionResolution class GetNextTransform method.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	UMBSTransformSolver* InSolver = nullptr;

	/**
	 * Is used outside UModularSectionResolution in cases when there is no need to calculate transform at current index.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	bool bOutShouldBeSkipped = false;

	/**
	 * Currently not in use.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	TObjectPtr<USplineComponent> InSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FVector OutLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FRotator OutRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FVector OutScale = FVector::OneVector;

};

/**
 * TODO: Deprecate and remove.
 */
USTRUCT(BlueprintType)
struct FNearTransformArgs
{
	GENERATED_BODY()

	FNearTransformArgs() {}
	FNearTransformArgs(float LevelZMultiplier, FVector Location, FRotator Rotation)
		: InLevelZMultiplier(LevelZMultiplier)
		, OutLocation(Location)
		, OutRotation(Rotation) { }

	UPROPERTY(EditAnywhere)
	float InLevelZMultiplier = 0.f;

	UPROPERTY(EditAnywhere)
	FVector OutLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FRotator OutRotation = FRotator::ZeroRotator;
};

/**
 * Modular section resolution is a UDataAsset class that is used to calculate transforms for new modular sections
 * of modular build system actor levels in a rectangular shape, and holds the target building section resolution.
 * @note If new size of a section is required then new subclass of modular section resolution should be created.
 * @note There can be a lot of modular section resolution in a single project if non-standard sizes are used.
 * 
 * Newly created subclasses should be named like this:
 * SR_[SnapModeName]_[XxYxZ]
 * For example: 
 *		SR_Default_400x400x400, for a default section resolution with uniform size 400x400x400 centimeters
 *		SR_Wall_400x100x400, for a wall section resolution with size 400x100x400 centimeters
 *		e.t.c
 * 
 * In the future there possibly will be an option to avoid modular section resolutions altogether
 * and use size of static meshes directly as Mesh->GetBounds().GetBox().GetSize().
 *
 * TODO: Add ability to use custom SnapMode to reduce amount of modular section resolution DataAssets created.
 * TODO: Users of UModularSectionResolution objects should be able to not use the default SnapMode.
 *
 * @note Use UMBSTransformSolver objects to calculate new transforms for new modular build systems.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MODULARBUILDSYSTEM_API UModularSectionResolution : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/**
	 * Default uniform modular section size.
	 * Default value is 400.f, which means 400x400x400 centimeters section.
	 */
	static constexpr float DefaultSectionSize	= 400.f;
	
	/**
	 * Value that should be used to describe building sections of undefined/invalid/unknown size.
	 */
	static constexpr float UndefinedSectionSize = -1.f;

	/**
	 * Generation of section placement resolution algorithm.
	 */
	static constexpr EMBSResolutionAlgorithmGeneration PlacementGeneration = EMBSResolutionAlgorithmGeneration::Gen3;

protected:
	/**
	 * Resolution of a section in centimeters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	FIntVector Resolution;
	
	/**
	 * Snap mode defines how new transform is calculated for each section on their initialization.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	EModularSectionResolutionSnapMode SnapMode;

public:
	/**
	 * @brief Calculates next modular section transform based on the Index and other data provided. Uses Resolution
	 * integer vector in order to properly shift transform relatively to modular build system actor (InTransform).
	 * @param InTransform Modular build system actor world transform.
	 * @param InIndex Current index.
	 * @param MaxInRow Max count of modular sections in a row.
	 * @param InMaxCount Total max count of sections in a (current) modular level.
	 * @param InLevelZMultiplier Modular level Z-axis multiplier.
	 * @param InStats Overall stats of a modular build system actor.
	 * @param InSolver Solver object. Will be used instead of default snap mode if provided.
	 * @param InPreviousLevelResolution Previous modular level modular section resolution.
	 * @return Next modular section transform.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	FTransform GetNextTransform(const FTransform& InTransform, int32 InIndex, int32 MaxInRow, int32 InMaxCount,
		float InLevelZMultiplier, const FModularBuildStats& InStats, UMBSTransformSolver* InSolver, 
		const UModularSectionResolution* InPreviousLevelResolution) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextDefaultTransform(FNextTransformArgs& Args) const;

	/**
	 * Deprecated. Use GetNextWallTransform method instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem", meta = (DeprecatedFunction))
	void GetNextWallTransform_DEPRECATED(FNextTransformArgs& Args) const;

	/**
	 * Deprecated. Use GetNextWallTransform method instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextWallTransform_Gen2(FNextTransformArgs& Args) const;
	
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextWallTransform(FNextTransformArgs& Args) const;

	/**
	 * Deprecated. Use GetNextRoofTransform method instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem", meta = (DeprecatedFunction))
	void GetNextRoofTransform_DEPRECATED(FNextTransformArgs& Args) const;

	/**
	 * Deprecated. Use GetNextRoofTransform instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextRoofTransform_Gen2(FNextTransformArgs& Args) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextRoofTransform(FNextTransformArgs& Args) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextRooftopTransform(FNextTransformArgs& Args) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNextCornerTransform(FNextTransformArgs& Args) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	FTransform GetNearTransform(const FTransform& InTransform, float InLevelZMultiplier,
		EModularSectionRelativePosition InSectionRelativePosition) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	void GetNearFrontTransform(FNearTransformArgs& Args) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	static FTransform GetTransformShifted(const FTransform& Initial, const FVector Shift, bool bTranslationOnly);

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	FName GetResolutionName() const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	float GetHeightAsMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	FIntPoint GetBounds(int32 MaxInRow, int32 TotalCount) const;

	/**
	 * @return Resolution integer vector.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	FIntVector GetValue() const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	EModularSectionResolutionSnapMode GetSnapMode() const;

	UFUNCTION(BlueprintCallable, Category=Resolution, meta=(CompactNodeTitle="X"))
	int32 GetX() const { return Resolution.X; }

	UFUNCTION(BlueprintCallable, Category=Resolution, meta=(CompactNodeTitle="Y"))
	int32 GetY() const { return Resolution.Y; }

	UFUNCTION(BlueprintCallable, Category=Resolution, meta=(CompactNodeTitle="Z"))
	int32 GetZ() const { return Resolution.Z; }

	// TODO: Use this for Auto resolution calculation mode
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static FIntVector GetResolutionFromMesh(UStaticMesh* InMesh);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingFront(int32 Index, int32 MaxInRow, int32 TotalCount);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingRight(int32 Index, int32 MaxInRow, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingRightLast(int32 Index, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingRightFirst(int32 Index, int32 MaxInRow, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingBack(int32 Index, int32 MaxInRow, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingLeft(int32 Index, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingLeftLast(int32 Index, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallFacingLeftFirst(int32 Index);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsWallInMiddle(int32 Index, int32 MaxInRow, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsDefaultFacingFront(int32 Index, int32 MaxInRow);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static bool IsDefaultFacingBack(int32 Index, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static int32 GetWallCurrentRow(int32 Index, int32 MaxInRow);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static int32 GetWallLeftFirstIndex();

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static int32 GetWallLeftLastIndex(int32 MaxInRow);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static int32 GetWallRightFirstIndex(int32 MaxInRow, int32 MaxCount);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static int32 GetWallRightLastIndex(int32 MaxCount);
	
	UFUNCTION(BlueprintCallable, Category=Resolution)
	static TArray<int32> GetWallFrontIndices(int32 MaxInRow, int32 MaxCount);

	UFUNCTION(BlueprintCallable, Category=Resolution)
	static TArray<int32> GetWallBackIndices(int32 MaxInRow, int32 MaxCount);
	
private:
	static constexpr int32 GetMaxRow(int32 MaxCount, int32 MaxInRow);
	static constexpr int32 GetWallMaxRow(int32 MaxCount, int32 MaxInRow);

	static constexpr int32 GetAdjustedIndex(int32 Index, int32 MaxInRow);
	static constexpr int32 GetAdjustedIndex(int32 Index, const FModularBuildStats& BuildStats);

	static constexpr int32 GetCurrentRow(int32 Index, int32 MaxInRow);
	static constexpr int32 GetCurrentRow(int32 Index, const FModularBuildStats& BuildStats);

	static constexpr bool IsFacingFront(int32 AdjustedIndex);

	static constexpr bool IsFacingBack(int32 AdjustedIndex, int32 MaxInRow);
	static constexpr bool IsFacingBack(int32 AdjustedIndex, const FModularBuildStats& BuildStats);

	static constexpr bool IsFacingLeft(int32 CurrentRow);

	static constexpr bool IsFacingRight(int32 CurrentRow, int32 MaxCount, int32 MaxInRow);
	static constexpr bool IsFacingRight(int32 CurrentRow, const FModularBuildStats& BuildStats);

};
