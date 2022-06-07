// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorBase.h"
#include "MBSTextureGenerator.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSTextureGenerator : public UMBSGeneratorBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MBS)
	TScriptInterface<IModularBuildSystemInterface> BuildSystem;
	
public:
	virtual bool SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystemPtr) override;
	virtual TScriptInterface<IModularBuildSystemInterface> GetBuildSystemPtr() const override;
	
protected:
	virtual void LogGenerationSummary() const override;
	
};
