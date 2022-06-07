// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelRooftopTransformSolver.h"

FTransform UTunnelRooftopTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	In(Args);
	
	constexpr float DefaultSectionSize = UModularSectionResolution::DefaultSectionSize;
	if (Args.InPreviousLevelResolution)
	{
		Args.OutLocation.X += Args.InPreviousLevelResolution->GetY();
		Args.OutLocation.Y += Args.InPreviousLevelResolution->GetY();
	}
	
	Args.OutLocation.X += Args.InResolution->GetX() * GetAdjustedIndex() - DefaultSectionSize;
	Args.OutLocation.Y += Args.InResolution->GetY() * GetCurrentRow();
	Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
	
	return Out(Args);
}
