// Fill out your copyright notice in the Description page of Project Settings.


#include "Preset/MBSPresetData.h"

#include "EditorReimportHandler.h"

FMBSPresetDataActions::FMBSPresetDataActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FMBSPresetDataActions::GetName() const
{
	return FText::FromString("MBS Preset Data");
}

UClass* FMBSPresetDataActions::GetSupportedClass() const
{
	return UMBSPresetData::StaticClass();
}

FColor FMBSPresetDataActions::GetTypeColor() const
{
	return FColor(115, 10, 255);
}

uint32 FMBSPresetDataActions::GetCategories()
{
	return AssetCategory;
}

bool FMBSPresetDataActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

void FMBSPresetDataActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto PresetDataImports = GetTypedWeakObjectPtrs<UMBSPresetData>(InObjects);
	MenuBuilder.AddMenuEntry(
		FText::FromString("Reimport"),
		FText::FromString("Reimports MBS preset data from file."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FMBSPresetDataActions::ExecuteReimport, PresetDataImports),
			FCanExecuteAction()));
}

void FMBSPresetDataActions::ExecuteReimport(TArray<TWeakObjectPtr<UMBSPresetData>> Objects)
{
	for (auto It = Objects.CreateConstIterator(); It; ++It)
	{
		UMBSPresetData* Object = It->Get();
		if (Object)
		{
			FReimportManager::Instance()->Reimport(Object, true);
		}
	}
	
}
