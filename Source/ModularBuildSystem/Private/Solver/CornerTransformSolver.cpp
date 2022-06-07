// Fill out your copyright notice in the Description page of Project Settings.


#include "Solver/CornerTransformSolver.h"
#include "ModularBuildSystem.h"

FTransform UCornerTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	In(Args);
	constexpr int32 DSS = UModularSectionResolution::DefaultSectionSize;
	
	// If we are on the back side in the current resolution - then we should be
	// on the back side on the max resolution (from the stats)
	if (IsFacingBack())
	{
		Args.OutLocation.X += (DSS * Args.InBuildStats.MaxCountInRow) - DSS * 2.f;
		if (IsFacingLeft())
		{
			Args.OutRotation.Yaw += 90.f;
		}

		if (IsFacingRight())
		{
			Args.OutRotation.Yaw -= 90.f;
		}
	}

	// Same with the right side
	if (IsFacingRight())
	{
		// TODO: With Bounds = (X=2, Y=1) this is not working yet. Fix
		// With such bounds BuildStats have MaxTotalRows property equals 2 and need to be 1
		UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: InBuildStats.MaxTotalRows = %d, InBuildStats.Bounds.Y = %d"),
			*GetName(), 
			Args.InBuildStats.MaxTotalRows, Args.InBuildStats.Bounds.Y);

		Args.OutLocation.Y += (DSS * Args.InBuildStats.Bounds.Y) - DSS;
	}
	else if (IsFacingFront() || IsFacingLeft())
	{
		Args.OutRotation.Yaw += 90.f;
		Args.OutLocation.Y -= DSS;
	}

	// -1.f as corners should be placed starting from the basement level.
	Args.OutLocation.Z += DSS * (Args.InLevelZMultiplier - 1.f);
	return Out(Args);
}
