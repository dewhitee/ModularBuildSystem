// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ModularBuildSystemPreset.generated.h"

class IModularBuildSystemInterface;
class AModularBuildSystemActor;

/**
 * 
 */
UCLASS(Abstract)
class MODULARBUILDSYSTEM_API UModularBuildSystemPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void SaveProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
	{
		unimplemented();
	}

	virtual void LoadProperties(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
	{
		unimplemented();
	}
};
