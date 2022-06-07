// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "TunnelRoofGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Tunnel roof")
class MODULARBUILDSYSTEM_API UTunnelRoofGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UTunnelRoofGeneratorProperty()
	{
		Data.DebugPropertyName = "Tunnel roof";
	}
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category=Generator)
	TObjectPtr<UMBSTransformSolver> Solver;
};
