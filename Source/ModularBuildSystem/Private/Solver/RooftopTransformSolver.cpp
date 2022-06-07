// Fill out your copyright notice in the Description page of Project Settings.


#include "Solver/RooftopTransformSolver.h"

FTransform URooftopTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	if (Args.InPreviousLevelResolution)
	{
		Args.OutLocation.X += Args.InPreviousLevelResolution->GetY();
		Args.OutLocation.Y += Args.InPreviousLevelResolution->GetY();
	}

	const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);
	const int32 CurrentRow = GetCurrentRow(Args.InIndex, Args.InMaxInRow);
    
	Args.OutLocation.X += Args.InResolution->GetX() * AdjustedIndex;
	Args.OutLocation.Y += Args.InResolution->GetY() * CurrentRow;
	Args.OutLocation.Z += GetDefaultSectionSize() * Args.InLevelZMultiplier;
	
	return Out(Args);
}