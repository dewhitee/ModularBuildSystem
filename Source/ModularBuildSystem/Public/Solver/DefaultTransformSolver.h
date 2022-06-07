// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSTransformSolver.h"
#include "DefaultTransformSolver.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UDefaultTransformSolver final : public UMBSTransformSolver
{
	GENERATED_BODY()

public:
	virtual FTransform GetNextTransform_Implementation(FNextTransformArgs& Args) override;
};
