// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "UObject/NoExportTypes.h"
#include "MBSPresetData.generated.h"

class UMBSPresetData;

class FMBSPresetDataActions : public FAssetTypeActions_Base
{
	EAssetTypeCategories::Type AssetCategory;
	
public:
	FMBSPresetDataActions(EAssetTypeCategories::Type InAssetCategory);
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	void ExecuteReimport(TArray<TWeakObjectPtr<UMBSPresetData>> Objects);
};

/**
 * 
 */
UCLASS(Blueprintable)
class MODULARBUILDSYSTEMEDITOR_API UMBSPresetData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Properties)
	FString Name;

	UPROPERTY(EditAnywhere, Category=Properties)
	FName Version;
	
	UPROPERTY(EditAnywhere, Category=Properties)
	TObjectPtr<UStaticMesh> Mesh;

#if WITH_EDITORONLY_DATA	
	UPROPERTY(VisibleAnywhere, Category=Source)
	FString SourceFilePath;
#endif
};
