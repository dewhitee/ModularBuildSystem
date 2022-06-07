// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemPreset.h"
#include "ModularBuildSystemActor.h"
#include "HouseBuildSystemPreset.generated.h"

class UHouseBuildSystemGenerator;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MODULARBUILDSYSTEM_API UHouseBuildSystemPreset : public UModularBuildSystemPreset
{
	GENERATED_BODY()

public:
	virtual void SaveProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem) override;
	virtual void LoadProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "ModularBuildSystem")
	TObjectPtr<UHouseBuildSystemGenerator> Generator;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	FModularLevel Basement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	TArray<FModularLevel> Floors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	TArray<FModularLevel> Walls;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	TArray<FModularLevel> Corners;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	FModularLevel Roof;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ModularBuildSystem")
	FModularLevel Rooftop;
};
