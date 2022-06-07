// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelWallTransformSolver.h"

FTransform UTunnelWallTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	In(Args);
	constexpr int32 DSS = UModularSectionResolution::DefaultSectionSize;

	// Left
	if (Args.InIndex < Args.InMaxInRow)
	{
		Args.OutRotation.Yaw -= 180.f;
		Args.OutLocation.X += (DSS * GetAdjustedIndex()) + DSS;
		Args.OutLocation.Y -= DSS;
	}
	// Right
	else
	{
		Args.OutLocation.X += (DSS * GetAdjustedIndex());
		Args.OutLocation.Y += (DSS * Args.InBuildStats.Bounds.Y) - DSS;
	}
	
	Args.OutLocation.Z += DSS * Args.InLevelZMultiplier;
	return Out(Args);
}
