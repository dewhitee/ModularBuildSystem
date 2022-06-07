// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "MBSPresetDataFactory.h"
#include "Factories/Factory.h"
#include "MBSReimportPresetDataFactory.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEMEDITOR_API UMBSReimportPresetDataFactory : public UMBSPresetDataFactory, public FReimportHandler
{
	GENERATED_BODY()
public:
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
};
