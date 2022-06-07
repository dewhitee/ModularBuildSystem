// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSMerger.h"

#include "IMeshMergeUtilities.h"
#include "MBSFunctionLibrary.h"
#include "MeshMergeModule.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularBuildSystemGenerator.h"
#include "Timer.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"

void FMBSMerger::MergeIntoStaticMesh(TScriptInterface<IModularBuildSystemInterface> MBS)
{
	check(MBS);
	TIMER_OBJ(MBS.GetObject());

	const FString& Name = UMBSFunctionLibrary::GetDisplayName(MBS);
	UE_LOG(LogMBSMerger, Warning, TEXT("%s: Merging meshes into single static mesh."), *Name);

	if (bIsMerged)
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: Already merged."), *Name);
		return;
	}

	if (MBS->GetMeshConfiguration().IsOfInstancedType())
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Currently merge is not supported for Instanced mesh configuration type."), *Name);
		unimplemented();
		return;
	}

	// TODO: Update to ensure that Instanced sections are also merged
	const TArray<AActor*> ActorsToMerge = GetActorsToMerge(MBS);
	if (ActorsToMerge.IsEmpty())
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: No actors to merge. Aborting."), *Name);
		return;
	}
	
	UE_LOG(LogMBS, Warning, TEXT("%s: Ready to merge %d sections"), *Name, ActorsToMerge.Num());

	// Creating directories if necessary
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AddPath(MBS::FPaths::SavedResultsDir);
	AssetRegistryModule.Get().AddPath(MBS::FPaths::MergedDir);

	//const FString SavedFileName = Name;
	const FString AssetName = GetAssetName(Name);
	const FString PackagePath = FString::Printf(TEXT("%s%s"), MBS::FPaths::MergedDir, *AssetName);
	
	UPackage* Package = UMBSFunctionLibrary::CreatePackageChecked(PackagePath, AssetName, UStaticMesh::StaticClass());
	
	const TArray<UPrimitiveComponent*> PrimitiveComponents = GetPrimitiveComponents(MBS, ActorsToMerge);

	FMeshMergingSettings MergeSettings = FMeshMergingSettings();
	MergeSettings.bMergePhysicsData = true;

	if (bCreateLODOnMerge)
	{
		MergeSettings.LODSelectionType = EMeshLODSelectionType::CalculateLOD;
	}

	// Merging
	const IMeshMergeUtilities& Module = FModuleManager::Get()
		.LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities")
		.GetUtilities();
	
	TArray<UObject*> AssetsToSync;
	FVector MergedLocation;
	constexpr float ScreenSize = TNumericLimits<float>::Max();
	UWorld* World = MBS.GetObject()->GetWorld();
	
	Module.MergeComponentsToStaticMesh(PrimitiveComponents, World, MergeSettings, nullptr, Package,
		AssetName, AssetsToSync, MergedLocation, ScreenSize, false);
	UE_LOG(LogMBS, Log, TEXT("%s: Merged location = %s"), *Name, *MergedLocation.ToString());

	// Clear all sections and reset build system
	MBS->ResetBuildSystem(true, false, true, true);

	// Replace sections with newly created merged static mesh actor
	UE_LOG(LogMBS, Warning, TEXT("%s: Replace sections with newly created merged static mesh actor."), *Name);
	UStaticMesh* MergedMesh = nullptr;
	if (AssetsToSync.FindItemByClass(&MergedMesh))
	{
		const FActorSpawnParameters Params;
		const FRotator MergedActorRotation(ForceInit);

		MergedSectionsStaticMeshActor = World->SpawnActor<AStaticMeshActor>(MergedLocation, MergedActorRotation, Params);
		MergedSectionsStaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(MergedMesh);
		MBS->AttachActor(MergedSectionsStaticMeshActor, false);
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: AssetsToSync.FindItemByClass failed."), *Name);
	}

	bIsMerged = true;
	UE_LOG(LogMBS, Warning, TEXT("%s: Merge is performed. Check new asset at %s path."), *Name, MBS::FPaths::MergedDir);
}

void FMBSMerger::UnmergeIntoModularSections(TScriptInterface<IModularBuildSystemInterface> MBS)
{
	check(MBS);
	TIMER_OBJ(MBS.GetObject());
	UE_LOG(LogMBS, Warning, TEXT("%s: Unmerging static mesh actor back into modular sections."), *UMBSFunctionLibrary::GetDisplayName(MBS));

	if (!MergedSectionsStaticMeshActor)
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: Nothing to unmerge."), *UMBSFunctionLibrary::GetDisplayName(MBS));
		return;
	}

	MBS->ResetBuildSystem();
	
	if (UModularBuildSystemGenerator* Generator = Cast<UModularBuildSystemGenerator>(MBS->GetGenerator().GetObject()))
	{
		IBuildingGeneratorInterface::Execute_Generate(Generator);
	}
	else
	{
		MBS->Init();
	}

	bIsMerged = false;
}

void FMBSMerger::Reset(const TScriptInterface<IModularBuildSystemInterface> MBS)
{
	if (MergedSectionsStaticMeshActor)
	{
		MergedSectionsStaticMeshActor->Destroy();
		MergedSectionsStaticMeshActor = nullptr;
		UE_LOG(LogMBS, Verbose, TEXT("%s: MergedSectionsStaticMeshActor has been destroyed and nulled."),
			*UMBSFunctionLibrary::GetDisplayName(MBS));
	}
	else
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: MergedSectionsStaticMeshActor was nullptr."), *UMBSFunctionLibrary::GetDisplayName(MBS));
	}
}

TArray<AActor*> FMBSMerger::GetActorsToMerge(const TScriptInterface<IModularBuildSystemInterface> MBS) const
{
	const TArray<FModularSection>& Sections = MBS->GetSections().GetStatic();
	if (Sections.IsEmpty())
	{
		UE_LOG(LogMBS, Error, TEXT("%s: No sections is spawned so merge can't be performed - nothing to merge."), *UMBSFunctionLibrary::GetDisplayName(MBS));
		return {};
	}

	// Get actors to be merged (Sections + ActorSections)
	const TArray<FModularSectionActor>& ActorSections = MBS->GetSections().GetActor();
	TArray<AActor*> ActorsToMerge;
	ActorsToMerge.Reserve(Sections.Num() + (bMergeActorSections ? ActorSections.Num() : 0));

	for (auto& Section : Sections)
	{
		ActorsToMerge.Add(Section.GetStaticMeshActor());
	}

	if (bMergeActorSections)
	{
		for (auto& ActorSection : ActorSections)
		{
			ActorsToMerge.Add(ActorSection.GetActor());
		}
	}

	return ActorsToMerge;
}

TArray<UPrimitiveComponent*> FMBSMerger::GetPrimitiveComponents(const TScriptInterface<IModularBuildSystemInterface> MBS,
	const TArray<AActor*>& ActorsToMerge)
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	PrimitiveComponents.Reserve(ActorsToMerge.Num());
	for (const AActor* Actor : ActorsToMerge)
	{
		TInlineComponentArray<UPrimitiveComponent*> Primitives;
		Actor->GetComponents<UPrimitiveComponent>(Primitives);
		PrimitiveComponents.Append(Primitives);
	}

	// Handle instanced sections
	if (MBS->GetMeshConfiguration().IsOfInstancedType())
	{
		// TODO: Test this
		for (const auto& InstancedSection : MBS->GetSections().GetInstanced())
		{
			PrimitiveComponents.Add(InstancedSection.GetISMC());
		}
	}
	
	return PrimitiveComponents;
}

FString FMBSMerger::GetAssetName(const FString& Name) const
{
	if (NameSettings.bOverrideName)
	{
		return (NameSettings.bKeepPrefix ? TEXT("SM_MBS_MERGED_") : TEXT("")) + FPackageName::GetShortName(NameSettings.OverrideName);
	}
	return TEXT("SM_MBS_MERGED_") + FPackageName::GetShortName(Name);
}
