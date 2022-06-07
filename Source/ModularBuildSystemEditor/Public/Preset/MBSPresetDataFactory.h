// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MBSPresetDataFactory.generated.h"

class UMBSPresetData;
/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEMEDITOR_API UMBSPresetDataFactory : public UFactory
{
	GENERATED_BODY()
public:
	UMBSPresetDataFactory();
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, FFeedbackContext* Warn) override;
		
	virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
		FFeedbackContext* Warn) override;

protected:
	static void CreateDataFromText(UMBSPresetData* Data, const TCHAR* Buffer, const TCHAR* BufferEnd);
};
