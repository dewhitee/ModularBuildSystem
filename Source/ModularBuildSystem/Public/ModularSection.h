// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.generated.h"

struct FMBSSide;
struct FModularLevel;
class UMBSTransformSolver;
struct FModularSectionInitializer;
class UModularLevelShape;
class UModularSectionResolution;
class UModularBuildSystemMeshList;

UENUM(BlueprintType)
enum class EModularSectionPivotLocation : uint8
{
	Default,
	BottomRight,
	BottomLeft,

	// TODO: Implement
	TopRight,
	TopLeft,
	
	/**
	 * Automatically select pivot location based on the static mesh.
	 * @note Currently not supported.
	 * TODO: Implement
	 */
	Automatic
};

UENUM(BlueprintType)
enum class EModularSectionSkipMode : uint8
{
	// With this skip mode modular level will be only hidden, which may cause upper levels to levitate.
	Hide,

	// With this skip mode modular level will be hidden and all upper levels shifted to fit the level below.
	HideAndShift
};

UENUM(BlueprintType)
enum class EModularSectionReloadMode : uint8
{
	// Sections will not be reloaded automatically.
	None,

	// In this case all sections of this build system will be automatically reloaded on move or parameters change.
	All,

	// In this case only current level sections of this build system will be reloaded.
	CurrentLevelOnly,
};

/**
 * Base struct for modular sections.
 */
USTRUCT(BlueprintType)
struct FModularSectionBase
{
	GENERATED_BODY()

	FModularSectionBase() {}
	explicit FModularSectionBase(const int32 InLevelId) : LevelId(InLevelId) {}
	virtual ~FModularSectionBase() = default;

private:	
	/**
	 * A unique level id this modular section is associated with. Used for dynamic reloading of a section
	 * with new mesh.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem", meta=(AllowPrivateAccess=true))
	int32 LevelId = -1;

public:
	virtual bool IsValid() const { unimplemented(); return false; }
	virtual FVector GetLocation(int32 Index = 0) const { unimplemented(); return {}; }
	virtual void SetLocation(FVector NewLocation, bool bRelative = true, int32 Index = 0) { unimplemented(); }
	virtual FTransform GetTransform(int32 Index = 0) const { unimplemented(); return {}; }
	virtual void SetTransform(const FTransform& NewTransform, bool bRelative = true, int32 Index = 0) { unimplemented(); }
	virtual void Offset(FVector Offset, bool bRelative = true, int32 Index = 0) { unimplemented(); }
	virtual void Scale(FVector ScaleOffset, bool bRelative = true, int32 Index = 0) { unimplemented(); }
	virtual void SetPivot(FVector PivotOffset, int32 Index = 0) { unimplemented(); }
	virtual FString GetName() const { unimplemented(); return {}; }
	virtual FVector GetForwardVector(int32 Index = 0) const { unimplemented(); return {}; }
	virtual FVector GetRightVector(int32 Index = 0) const { unimplemented(); return {}; }
	virtual FVector GetUpVector(int32 Index = 0) const { unimplemented(); return {}; }
	virtual void Reset() { unimplemented(); }
	virtual void SetMesh(UStaticMesh* NewStaticMesh) const { unimplemented(); }

	int32 GetLevelId() const { return LevelId; }

	bool IsInLevel(const int32 InLevelId) const;
	bool IsInLevel(const FModularLevel& InLevel) const;
	bool IsInLevel(const TArray<FModularLevel>& InLevels) const;

	FString ToString() const
	{
		return FString::Printf(TEXT("{Name=%s, LevelId=%d}"), *GetName(), LevelId);
	}

protected:
	static void SetActorLocation(AActor* OfActor, FVector NewLocation, bool bRelative);
	static void SetActorTransform(AActor* OfActor, const FTransform& NewTransform, bool bRelative);
	static void OffsetActor(AActor* OfActor, FVector Offset, bool bRelative);
	static void ScaleActor(AActor* OfActor, FVector ScaleOffset, bool bRelative);
};

/**
 * Instanced modular section struct.
 */
USTRUCT(BlueprintType)
struct FModularSection : public FModularSectionBase
{
	GENERATED_BODY()

	FModularSection() {}
	FModularSection(int32 InLevelId, AStaticMeshActor* InStaticMeshActor)
		: Super(InLevelId)
		, StaticMesh(InStaticMeshActor) {}

private:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem", meta=(AllowPrivateAccess=true))
	TObjectPtr<AStaticMeshActor> StaticMesh = nullptr;

public:
	virtual bool IsValid() const override;
	virtual FVector GetLocation(int32 Index = 0) const override;
	virtual void SetLocation(FVector NewLocation, bool bRelative = true, int32 Index = 0) override;
	virtual FTransform GetTransform(int32 Index = 0) const override;
	virtual void SetTransform(const FTransform& NewTransform, bool bRelative = true, int32 Index = 0) override;
	virtual void Offset(FVector Offset, bool bRelative = true, int32 Index = 0) override;
	virtual void Scale(FVector ScaleOffset, bool bRelative = true, int32 Index = 0) override;
	virtual void SetPivot(FVector PivotOffset, int32 Index = 0) override;
	virtual FString GetName() const override;
	virtual FVector GetForwardVector(int32 Index = 0) const override;
	virtual FVector GetRightVector(int32 Index) const override;
	virtual FVector GetUpVector(int32 Index) const override;
	virtual void Reset() override;
	virtual void SetMesh(UStaticMesh* NewStaticMesh) const override;

	AStaticMeshActor* GetStaticMeshActor() const { return StaticMesh; }
};

/**
 * Actor modular section struct.
 */
USTRUCT(BlueprintType)
struct FModularSectionActor : public FModularSectionBase
{
	GENERATED_BODY()

	FModularSectionActor() {}
	FModularSectionActor(int32 InLevelId, AActor* InActor)
		: Super(InLevelId)
		, Actor(InActor) {}

private:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem", meta=(AllowPrivateAccess=true))
	TObjectPtr<AActor> Actor = nullptr;

public:
	virtual bool IsValid() const override;
	virtual FVector GetLocation(int32 Index = 0) const override;
	virtual void SetLocation(FVector NewLocation, bool bRelative = true, int32 Index = 0) override;
	virtual FTransform GetTransform(int32 Index = 0) const override;
	virtual void SetTransform(const FTransform& NewTransform, bool bRelative = true, int32 Index = 0) override;
	virtual void Offset(FVector Offset, bool bRelative = true, int32 Index = 0) override;
	virtual void Scale(FVector ScaleOffset, bool bRelative = true, int32 Index = 0) override;
	virtual void SetPivot(FVector PivotOffset, int32 Index = 0) override;
	virtual FString GetName() const override;
	virtual FVector GetForwardVector(int32 Index) const override;
	virtual FVector GetRightVector(int32 Index) const override;
	virtual FVector GetUpVector(int32 Index) const override;
	virtual void Reset() override;
	virtual void SetMesh(UStaticMesh* NewStaticMesh) const override;

	AActor* GetActor() const { return Actor; }
};

/**
 * Instanced modular section struct that is supposed to be used in case MeshConfiguration.Type of
 * modular build system actor is set to Instanced or HierarchicalInstanced.
 */
USTRUCT(BlueprintType)
struct FModularSectionInstanced : public FModularSectionBase
{
	GENERATED_BODY()

	FModularSectionInstanced() {}
	FModularSectionInstanced(int32 InLevelId, UInstancedStaticMeshComponent* InInstancedStaticMeshComponent, int32 InInstanceCount = 0);

private:	
	/**
	 * Pointer to instanced static mesh component that this instanced modular section is associated with.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem", meta=(AllowPrivateAccess=true))
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent = nullptr;

	/**
	 * Total instance count of the instanced static mesh component. Is used only for debug.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem", meta=(AllowPrivateAccess=true))
	int32 InstanceCount = 0;

public:	
	/**
	 * Returns transform of a instanced static mesh component instance at specified index.
	 * @param AtIndex Instance index.
	 * @return Transform of a instanced static mesh component instance at specified index.
	 */
	FTransform GetSectionTransform(int32 AtIndex) const;

	/**
	 * Returns name of a instanced static mesh component instance at specified index.
	 * @param AtIndex Instance index. Will be the same as GetName() if equals -1. 
	 */
	FString GetSectionName(int32 AtIndex = -1) const;

	/**
	 * Returns true if instanced static mesh component pointer is valid.
	 */
	virtual bool IsValid() const override;
	virtual FVector GetLocation(int32 Index) const override;
	virtual void SetLocation(FVector NewLocation, bool bRelative, int32 Index) override;
	virtual FTransform GetTransform(int32 Index) const override;
	virtual void SetTransform(const FTransform& NewTransform, bool bRelative, int32 Index) override;
	virtual void Offset(FVector Offset, bool bRelative, int32 Index) override;
	virtual void Scale(FVector ScaleOffset, bool bRelative, int32 Index) override;
	virtual void SetPivot(FVector PivotOffset, int32 Index) override;
	virtual FString GetName() const override;
	virtual FVector GetForwardVector(int32 Index) const override;
	virtual FVector GetRightVector(int32 Index) const override;
	virtual FVector GetUpVector(int32 Index) const override;
	virtual void Reset() override;
	virtual void SetMesh(UStaticMesh* NewStaticMesh) const override;

	UInstancedStaticMeshComponent* GetISMC() const { return InstancedStaticMeshComponent; }
	int32 GetInstanceCount() const;

	// TODO: For debug purposes only. Remove later.
	int32 GetPreviousInstanceCount() const { return InstanceCount; }
	void SetPreviousInstanceCount(int32 Value) { InstanceCount = Value; }
};

/**
 * Initializer of all modular sections associated with a single modular level.
 */
USTRUCT(BlueprintType)
struct FModularSectionInitializer
{
	GENERATED_BODY()

private:	
	/**
	 * Static mesh that will be set to all newly created modular sections associated with a level that has
	 * this initializer.
	 */
	UPROPERTY(VisibleAnywhere, Category="ModularBuildSystem")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	/**
	 * You can provide mesh list data asset to be able to choose StaticMeshes using a MeshListIndex slider below.
	 */
	UPROPERTY(EditInstanceOnly, Category="ModularBuildSystem")
	TObjectPtr<UModularBuildSystemMeshList> MeshList = nullptr;

	UPROPERTY(EditInstanceOnly, Category="ModularBuildSystem", meta=(EditCondition="MeshList != nullptr", ClampMin=0, ClampMax=16))
	int32 MeshListIndex = 0;

	UPROPERTY(EditAnywhere, Category="ModularBuildSystem")
	TObjectPtr<UModularSectionResolution> Resolution = nullptr;

	UPROPERTY(EditAnywhere, Category="ModularBuildSystem", meta=(ClampMin=0, ClampMax=64))
	int32 TotalCount = 0;

	UPROPERTY(EditAnywhere, Category="ModularBuildSystem", meta=(ClampMin=1, ClampMax=8))
	int32 MaxInRow = 1;

	/**
	 * Count of skipped sections after last initialization call.
	 */
	UPROPERTY(VisibleAnywhere, Category="ModularBuildSystem")
	int32 SkippedCount = 0;

public:	
	struct FModularBuildStats GetBuildStats() const;
	UStaticMesh* GetStaticMesh() const;
	
	int32 GetTotalCount() const { return TotalCount; }
	void AddTotalCount(int32 Delta) { TotalCount += Delta; }
	int32 GetMaxInRow() const { return MaxInRow; }
	UModularSectionResolution* GetResolution() const { return Resolution; }
	UModularBuildSystemMeshList* GetMeshList() const { return MeshList; }
	int32 GetMeshListIndex() const { return MeshListIndex; }
	
	/**
	 * Gets this initializer's Z-axis multiplier, which is a value that is used for next modular levels to shift
	 * their modular sections location correctly by Z-axis.
	 * 
	 * @note This is not the same as ZMultiplier of a ModularLevel - it's ZMultiplier value is a multiplier relative
	 * to the modular build system actor location and is used to determine the location Z value of a modular section
	 * relatively to the modular build system actor.
	 * @see FModularLevel
	 *
	 * @note If DefaultModularSectionSize equals 400.f (400 centimeters), and Resolution.Z has a value of 200.f
	 * then this function will return a value of 0.5f
	 * 
	 * @note If Resolution property is not set - this function will return value of -1f
	 * 
	 * @return Z-axis multiplier for this initializer from the modular section resolution Z-value.
	 */
	float GetZMultiplier() const;

	/**
	 * @return TotalCount minus SkippedCount
	 */
	int32 GetAdjustedTotalCount() const;

	void SetFromMeshList();
	void SetMeshList(UModularBuildSystemMeshList* InMeshList);
	void SetMeshListIndex(int32 InIndex);
	void SetResolution(UModularSectionResolution* InResolution);
	void SetTotalCount(int32 InTotalCount);
	void SetMaxInRow(int32 InMaxInRow);
	void SetSkippedCount(int32 InSkippedCount);
};

/**
 * Modular sections initializer method arguments struct.
 */
struct FInitModularSectionsArgs
{
	FInitModularSectionsArgs(FModularSectionInitializer& InInitializer, int32 LevelId, float LevelZMultiplier,
		UModularSectionResolution* PreviousLevelResolution, UModularLevelShape* Shape,
		UInstancedStaticMeshComponent* Component, EModularSectionPivotLocation PivotLocationOverride,
		UMBSTransformSolver* Solver)
		: Initializer(InInitializer)
		, InLevelId(LevelId)
		, InLevelZMultiplier(LevelZMultiplier)
		, InPreviousLevelResolution(PreviousLevelResolution)
		, InShape(Shape)
		, InInstancedStaticMeshComponent(Component)
		, InPivotLocationOverride(PivotLocationOverride)
		, InSolver(Solver) {}

	FModularSectionInitializer& Initializer;
	int32 InLevelId;
	float InLevelZMultiplier;
	UModularSectionResolution* InPreviousLevelResolution;
	UModularLevelShape* InShape;
	UInstancedStaticMeshComponent* InInstancedStaticMeshComponent;
	EModularSectionPivotLocation InPivotLocationOverride;
	UMBSTransformSolver* InSolver;
	
	mutable TArray<int32> OutSkippedIndices;
};
