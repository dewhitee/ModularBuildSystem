// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "TunnelWallGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Tunnel walls")
class MODULARBUILDSYSTEM_API UTunnelWallGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UTunnelWallGeneratorProperty()
	{
		Data.DebugPropertyName = "Tunnel walls";
	}
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category=Generator)
	TObjectPtr<UMBSTransformSolver> Solver;
};
