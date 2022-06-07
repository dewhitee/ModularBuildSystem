// Fill out your copyright notice in the Description page of Project Settings.


#include "Solver/DefaultTransformSolver.h"

FTransform UDefaultTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	// Default location handling
	Args.OutLocation.X += Args.InResolution->GetX() * GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);	// Updates on every iteration
	Args.OutLocation.Y += Args.InResolution->GetY() * GetCurrentRow(Args.InIndex, Args.InMaxInRow);     // Updates on every row shift
	Args.OutLocation.Z += UModularSectionResolution::DefaultSectionSize * Args.InLevelZMultiplier;      // Updates on each level (floor) shift

	return Out(Args);
}
