// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "TunnelBasementGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Tunnel basement")
class MODULARBUILDSYSTEM_API UTunnelBasementGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UTunnelBasementGeneratorProperty()
	{
		Data.DebugPropertyName = "Tunnel basement";
	}
};
