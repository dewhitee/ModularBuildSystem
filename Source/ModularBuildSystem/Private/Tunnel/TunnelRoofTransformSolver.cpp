// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelRoofTransformSolver.h"

FTransform UTunnelRoofTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
	In(Args);
	const int32 AdjustedIndex = GetAdjustedIndex();
	const int32 CurrentRow = GetCurrentRow();
	// TODO: Implement
	return Out(Args);
}
