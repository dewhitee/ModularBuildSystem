// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSTransformSolver.h"
#include "CornerTransformSolver.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UCornerTransformSolver final : public UMBSTransformSolver
{
	GENERATED_BODY()

public:
	virtual FTransform GetNextTransform_Implementation(FNextTransformArgs& Args) override;
};
