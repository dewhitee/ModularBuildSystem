// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSMerger.generated.h"

class IModularBuildSystemInterface;
class AModularBuildSystemActor;

USTRUCT(BlueprintType)
struct FMBSMergeNameSettings
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category=Merge)
	bool bOverrideName = false;
    
	UPROPERTY(EditInstanceOnly, Category=Merge, meta=(EditCondition=bOverrideName))
	bool bKeepPrefix = true;
    
	UPROPERTY(EditInstanceOnly, Category=Merge, meta=(EditCondition=bOverrideName))
	FString OverrideName = FString();
};

/**
 * Structure that manages merge and unmerge process of a MBS actor's modular sections.
 */
USTRUCT(BlueprintType)
struct MODULARBUILDSYSTEM_API FMBSMerger
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, AdvancedDisplay, /*DuplicateTransient, */Category=Merge)
	TObjectPtr<AStaticMeshActor> MergedSectionsStaticMeshActor;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, Category=Merge)
	bool bMergeActorSections = true;
	
	UPROPERTY(EditInstanceOnly, Category=Merge)
	bool bSaveMergedIfUniqueOnly = true;

	UPROPERTY(EditInstanceOnly, Category=Merge)
	bool bCreateLODOnMerge = true;

	UPROPERTY(EditInstanceOnly, Category=Merge)
	FMBSMergeNameSettings NameSettings;

	UPROPERTY(VisibleInstanceOnly, Category=Merge)
	bool bIsMerged = false;
#endif	

	void MergeIntoStaticMesh(TScriptInterface<IModularBuildSystemInterface> MBS);
	void UnmergeIntoModularSections(TScriptInterface<IModularBuildSystemInterface> MBS);
	void Reset(const TScriptInterface<IModularBuildSystemInterface> MBS);

private:
	TArray<AActor*> GetActorsToMerge(const TScriptInterface<IModularBuildSystemInterface> MBS) const;
	static TArray<UPrimitiveComponent*> GetPrimitiveComponents(const TScriptInterface<IModularBuildSystemInterface> MBS, const TArray<AActor*>& ActorsToMerge);
	FString GetAssetName(const FString& Name) const;
};
