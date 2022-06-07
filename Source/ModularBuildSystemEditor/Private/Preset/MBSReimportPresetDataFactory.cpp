// Fill out your copyright notice in the Description page of Project Settings.


#include "Preset/MBSReimportPresetDataFactory.h"

#include "Preset/MBSPresetData.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"

bool UMBSReimportPresetDataFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (const UMBSPresetData* Preset = Cast<UMBSPresetData>(Obj))
	{
		OutFilenames.Add(UAssetImportData::ResolveImportFilename(Preset->SourceFilePath, Preset->GetOutermost()));
		return true;
	}
	return false;
}

void UMBSReimportPresetDataFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UMBSPresetData* Data = Cast<UMBSPresetData>(Obj);
	if (Data && ensure(NewReimportPaths.Num() == 1))
	{
		Data->SourceFilePath = UAssetImportData::SanitizeImportFilename(NewReimportPaths[0], Data->GetOutermost());
	}
}

EReimportResult::Type UMBSReimportPresetDataFactory::Reimport(UObject* Obj)
{
	UMBSPresetData* Preset = Cast<UMBSPresetData>(Obj);
	if (!Preset)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = UAssetImportData::ResolveImportFilename(Preset->SourceFilePath, Preset->GetOutermost());
	if (!FPaths::GetExtension(Filename).Equals(TEXT("mbspreset")))
	{
		return EReimportResult::Failed;
	}

	CurrentFilename = Filename;
	FString FileData;
	if (FFileHelper::LoadFileToString(FileData, *CurrentFilename))
	{
		const TCHAR* Ptr = *FileData;
		Preset->Modify();
		Preset->MarkPackageDirty();
		
		CreateDataFromText(Preset, Ptr, Ptr + FileData.Len());

		// Save the source file path and timestamp
		Preset->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, Preset->GetOutermost());
	}

	return EReimportResult::Succeeded;
}
