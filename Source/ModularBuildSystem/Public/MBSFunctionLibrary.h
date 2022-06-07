// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildStats.h"
#include "ModularSection.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MBSFunctionLibrary.generated.h"

class UMBSGeneratorProperty;
class AModularBuildSystemActor;
struct FModularSection;
struct FModularLevel;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr const TCHAR* DefaultRootName = TEXT("Root");
	
public:
	/**
 	 * @return Level with Id from specified Levels array.
 	 */
	static FModularLevel* GetLevelWithIdChecked(const TArray<FModularLevel>& Levels, int32 Id);

	static void OffsetSections(const TArray<FModularSection*>& InSections, FVector Offset, bool bRelative = true);
	static void ScaleSections(const TArray<FModularSection*>& InSections, FVector Offset, bool bRelative = true);
	static void ForEachBuildSystem(void (*InFunction)(TActorIterator<AModularBuildSystemActor> It));

	static void ForEachLevel(const TArray<FModularLevel*>& InLevels, TFunction<void(FModularLevel&)> InFunction);
	static void ForEachLevel(const TArray<FModularLevel*>& InLevels, TFunction<void(const FModularSectionInitializer&)> InFunction);
	static void ForEachLevel(AModularBuildSystemActor* BS, const TArray<FModularLevel*>& InLevels, void(AModularBuildSystemActor::* InFunction)(FModularLevel&));
	static void ForEachLevel(AModularBuildSystemActor* BS, const TArray<FModularLevel*>& InLevels, void(AModularBuildSystemActor::* InFunction)(const FModularSectionInitializer&));

	static void InitSectionFromProperty(AModularBuildSystemActor* BS, const UMBSGeneratorProperty* Property, bool bRandomIndex,
		int32 DefaultIndex, const FTransform& AtTransform, int32 LevelId, bool bAddToSections, bool bWithRelativeTransform);

	static UPackage* CreatePackageChecked(const FString& PackagePath, FString AssetName, const UClass* Class);

	static UWorld* GetTestWorld();
	
	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	static UTexture2D* GetMeshAssetThumbnail(UStaticMesh* InStaticMesh);
	
	UFUNCTION(BlueprintCallable, Category=MBS)
	static void ReleaseMeshThumbnail(UTexture2D* InThumbnailTexture);
	
	/**
	 * @brief Long operation. Fixes transforms of all build systems modular sections
	 */
	UFUNCTION(CallInEditor, Category = "Manager|LongOperations")
	static void FixAllBuildSystemsTransform(const UWorld* World);
	
	UFUNCTION(CallInEditor, Category = "Manager")
	static void AllToBuildModeOn(const UWorld* World);

	UFUNCTION(CallInEditor, Category = "Manager")
	static void AllToBuildModeOff(const UWorld* World);

	UFUNCTION(CallInEditor, Category = "Manager")
	static void AllToSectionReloadModeNone(const UWorld* World);

	UFUNCTION(CallInEditor, Category = "Manager|LongOperations")
	static void RegenerateAllBuildSystems(const UWorld* World);

	UFUNCTION(CallInEditor, Category = "Manager|LongOperations")
	static void MergeAllBuildSystems(const UWorld* World);

	UFUNCTION(CallInEditor, Category = "Manager|LongOperations")
	static void UnmergeAllBuildSystems(const UWorld* World);

	static FString GetDisplayName(const UObject* InObject);
	
	template<class T>
	static FString GetDisplayName(TScriptInterface<T> InInterface)
	{
		return GetDisplayName(InInterface.GetObject());
	}

	/**
 	 * Calculates new transform relative to provided build system actor transform.
 	 * @param BuildSystem
 	 * @param BuildStats
 	 * @param InIndex Index of a modular section.
 	 * @param ActorTransform Modular build system actor transform.
 	 * @param Args Initialization arguments.
 	 * @param bOutShouldBeSkipped If true - then this modular section initialization should be skipped. This is useful when custom shape 
 	 * object is provided that do not need to spawn modular section at the specified index.
 	 * @return New transform relative to the build system actor.
 	 * TODO: Add missing parameter description
 	 */
	static FTransform CalculateNewTransform(AModularBuildSystemActor* BuildSystem, const FModularBuildStats& BuildStats,
		int32 InIndex, const FTransform& ActorTransform, const FInitModularSectionsArgs& Args, bool& bOutShouldBeSkipped);

	/**
 	 * Returns box bounds of a modular level interior.
 	 * @param BuildSystem
 	 * @param InLevel Level to retrieve box bounds from.
 	 * @return Box bounds of a modular level interior.
 	 * TODO: Implement this method
 	 */
	static FBox GetModularLevelInteriorBox(const AModularBuildSystemActor* BuildSystem, const FModularLevel* InLevel);

	/**
	 * Fixes root component if it is not DefaultSceneRoot. Should trigger only once.
	 * Might be removed later if RootComponent will be constructed through CreateDefaultSubobject in ModularBuildSystemActor constructor. 
	 * However, this will cause all build system actors that were spawned before to be corrupted, 
	 * e.g their modular section static mesh actors will have wrong transforms (zeroed).
	 */
	static void FixRootComponent(AActor* Actor);

	struct FLevelsArrayEntry
	{
		FLevelsArrayEntry(const TArray<FModularLevel>* Array) : Levels(Array) {}
		const TArray<FModularLevel>* Levels;
	};
	static TArray<FModularLevel*> GetAllLevelsWrapper(const TArray<const FModularLevel*>& SingleEntries,
		const TArray<FLevelsArrayEntry>& ArrayEntries);
	
	static FModularLevel* GetLevelWithIdWrapper(const TArray<const FModularLevel*>& SingleEntries,
		const TArray<FLevelsArrayEntry>& ArrayEntries, int32 Id);

#if WITH_EDITOR
	static void LogComponents(const AActor* Actor);
	static void AddBillboard(AActor* Actor);
#endif
};
