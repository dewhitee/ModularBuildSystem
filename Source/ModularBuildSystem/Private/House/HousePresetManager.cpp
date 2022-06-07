// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HousePresetManager.h"

#include "MBSFunctionLibrary.h"
#include "ModularBuildSystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "House/HouseBuildSystemGenerator.h"
#include "House/HouseBuildSystemPreset.h"
#include "House/HouseBuildSystemPresetList.h"

void UHousePresetManager::SavePreset_Implementation()
{
#if WITH_EDITOR
	if (!BS.IsValid())
	{
		BS = Cast<AHouseBuildSystemActor>(GetOuter());
		check(BS.IsValid());
	}
	
	UE_LOG(LogMBS, Warning, TEXT("%s: Saving result..."), *BS->GetActorLabel());
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Creating MBS/SavedResults in Content folder of a game
	AssetRegistryModule.Get().AddPath(MBS::FPaths::SavedResultsDir);
	AssetRegistryModule.Get().AddPath(MBS::FPaths::PresetsDir);

	const FString SavedFileName = BS->GetActorLabel(); // generate new file name

	const FString AssetName = TEXT("SM_MBS_PRESET_") + FPackageName::GetShortName(BS->GetActorLabel());
	const FString PackagePath = FString::Printf(TEXT("%s%s"), MBS::FPaths::PresetsDir, *AssetName);

	UPackage* Package = UMBSFunctionLibrary::CreatePackageChecked(PackagePath, AssetName, UHouseBuildSystemPreset::StaticClass());

	UHouseBuildSystemPreset* NewAsset = NewObject<UHouseBuildSystemPreset>(Package, 
		UHouseBuildSystemPreset::StaticClass(), *SavedFileName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

	NewAsset->SaveProperties(BS.Get());

	FAssetRegistryModule::AssetCreated(NewAsset);
	NewAsset->MarkPackageDirty();
	Package->SetDirtyFlag(true);
	NewAsset->PostEditChange();
	NewAsset->AddToRoot();

	// Set newly create asset as the current Preset
	Preset = NewAsset;
#endif
}

void UHousePresetManager::LoadPreset_Implementation()
{
#if WITH_EDITOR
	if (PresetList && PresetList->Presets.IsValidIndex(PresetSlider))
	{
		Preset = PresetList->Presets[PresetSlider];
	}

	if (Preset)
	{
		Preset->LoadProperties(BS.Get());
		if (BS->Generator && IBuildingGeneratorInterface::Execute_Generate(BS->Generator).Succeeded())
		{
			BS->TransformBounds.SetBounds(
				FIntVector(BS->GetBuildStats().GetBoundsVector()),
				BS->GetStretchManager().GetScaleCoefficientsSwappedXY());
			BS->ApplyStretch();
		}
	}
#endif
}

bool UHousePresetManager::CanEditChange(const FProperty* InProperty) const
{
	const bool Parent = UObject::CanEditChange(InProperty);
	if (BS.IsValid())
	{
		return Parent && !BS->IsBuildModeActivated();
	}
	return Parent;
}

void UHousePresetManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, PresetManager->PresetSlider))
		{
			LoadPreset();
			BS->ResetBuildSystem();
			BS->Init();
		}
	}
	UObject::PostEditChangeProperty(PropertyChangedEvent);
}
