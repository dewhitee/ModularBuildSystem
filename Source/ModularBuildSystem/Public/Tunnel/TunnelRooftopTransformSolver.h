// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Solver/MBSTransformSolver.h"
#include "TunnelRooftopTransformSolver.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UTunnelRooftopTransformSolver : public UMBSTransformSolver
{
	GENERATED_BODY()

public:
	virtual FTransform GetNextTransform_Implementation(FNextTransformArgs& Args) override;
};
