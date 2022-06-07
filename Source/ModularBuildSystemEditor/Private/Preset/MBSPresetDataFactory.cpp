// Fill out your copyright notice in the Description page of Project Settings.


#include "Preset/MBSPresetDataFactory.h"

#include "Preset/MBSPresetData.h"
#include "ModularBuildSystemEditor.h"
#include "EditorFramework/AssetImportData.h"

UMBSPresetDataFactory::UMBSPresetDataFactory()
{
	Formats.Add(TEXT("mbspreset;MBS Preset Data"));
	SupportedClass = UMBSPresetData::StaticClass();
	bCreateNew = false;		// Turned off for import
	bEditAfterNew = false;	// Turned off for import
	bEditorImport = true;
	bText = true;
}

UObject* UMBSPresetDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMBSPresetData>(InParent, InClass, InName, Flags | RF_Transactional);
}

UObject* UMBSPresetDataFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn)
{
	UE_LOG(LogMBSEditor, Warning, TEXT("FactoryCreateText"));
	const UImportSubsystem* ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>();
	ImportSubsystem->OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	if (InClass != UMBSPresetData::StaticClass() || FCString::Stricmp(Type, TEXT("mbspreset")) != 0)
	{
		return nullptr;
	}

	UMBSPresetData* Data = CastChecked<UMBSPresetData>(NewObject<UMBSPresetData>(InParent, InName, Flags));
	CreateDataFromText(Data, Buffer, BufferEnd);

	// Save the source file path
	Data->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, Data->GetOutermost());
	ImportSubsystem->OnAssetPostImport.Broadcast(this, Data);

	return Data;
}

void UMBSPresetDataFactory::CreateDataFromText(UMBSPresetData* Data, const TCHAR* Buffer, const TCHAR* BufferEnd)
{
	Data->Name = Buffer;
}
