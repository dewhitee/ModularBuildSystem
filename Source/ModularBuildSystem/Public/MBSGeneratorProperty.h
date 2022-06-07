// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorBase.h"
#include "UObject/NoExportTypes.h"
#include "MBSGeneratorProperty.generated.h"

class UModularBuildSystemGenerator;
class AModularBuildSystemActor;

namespace MBS
{
	/**
	 * Struct to check validity of chosen generator properties.
	 */
	struct FGenPropertyValidator
	{
		TMap<UMBSGeneratorProperty*, FString> Properties;
		FText Text;
		TArray<FText> Errors;
		
		FGenPropertyValidator(const TMap<UMBSGeneratorProperty*, FString>& GenProperties, const FText& ValidationText)
			: Properties(GenProperties), Text(ValidationText) {}
		FGenPropertyValidator(const TMap<UMBSGeneratorProperty*, FString>& GetProperties, const FString& ValidationStr)
			: Properties(GetProperties), Text(FText::FromString(ValidationStr)) {}
		bool Validate();
	};
}

USTRUCT(BlueprintType)
struct FMBSGeneratorPropertyInitArgs
{
	GENERATED_BODY()

	FMBSGeneratorPropertyInitArgs() {}
	
	TObjectPtr<UModularBuildSystemGenerator> Generator = nullptr;
	TObjectPtr<AModularBuildSystemActor> BuildSystem = nullptr;
	int32 AtIndex = -1;
	mutable int32 LevelIndex = -1;
	FString InLevelName = FString();
	TArray<int32>* InIndices = nullptr;
	TArray<FTransform>* InTransforms = nullptr;
	TArray<int32>* OutIndices = nullptr;
};

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MODULARBUILDSYSTEM_API UMBSGeneratorProperty : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Property, meta=(DisplayPriority=1))
	bool bSupported = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Property, meta=(DisplayPriority=1, EditCondition="bSupported"))
	bool bEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Property, meta=(DisplayPriority=1, EditCondition="bSupported"))
	FMBSGeneratorPropertyData Data;

	UPROPERTY(EditAnywhere, Category="Mesh", meta=(ClampMin=0, ClampMax=16))
	int32 MeshIndex;
	
	// TODO: Properties are not supposed to generate something by themselves, so this should be removed in the future,
	// or the purpose of this class must be changed.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Generator)
	bool Init(UPARAM(Ref) FMBSGeneratorPropertyInitArgs& Args);
	virtual bool Init_Implementation(FMBSGeneratorPropertyInitArgs& Args)
	{
		return true;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Generator)
	bool IsSet() const;
	virtual bool IsSet_Implementation() const
	{
		return Data.IsSet();
	}

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
};
