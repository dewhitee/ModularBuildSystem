// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "TunnelRooftopGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Tunnel rooftop")
class MODULARBUILDSYSTEM_API UTunnelRooftopGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()
	
public:
	UTunnelRooftopGeneratorProperty()
	{
		Data.DebugPropertyName = "Tunnel rooftop";
	}
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category=Generator)
	TObjectPtr<UMBSTransformSolver> Solver;
};
