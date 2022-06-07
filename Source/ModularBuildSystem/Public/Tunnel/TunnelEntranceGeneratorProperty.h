// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "TunnelBuildSystemGenerator.h"
#include "TunnelEntranceGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Tunnel entrance")
class MODULARBUILDSYSTEM_API UTunnelEntranceGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UTunnelEntranceGeneratorProperty()
	{
		Data.DebugPropertyName = "Tunnel entrance";
		EntranceRoof.DebugPropertyName = "Tunnel entrance roof";
		EntranceRooftop.DebugPropertyName = "Tunnel entrance rooftop";
	}
	
	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UMBSTransformSolver> EntranceWallSolver;

	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UMBSTransformSolver> EntranceRoofSolver;

	UPROPERTY(EditAnywhere, Category=Generator)
	FMBSGeneratorPropertyData EntranceRoof;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator, meta=(EditCondition="bEnabled"))
	TObjectPtr<UMBSTransformSolver> EntranceRooftopSolver;

	UPROPERTY(EditAnywhere, Category=Generator)
	FMBSGeneratorPropertyData EntranceRooftop;

	bool CustomInit(UTunnelBuildSystemGenerator* Generator, ATunnelBuildSystemActor* BS);
};
