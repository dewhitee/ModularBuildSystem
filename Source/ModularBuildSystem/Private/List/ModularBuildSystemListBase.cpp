// Fill out your copyright notice in the Description page of Project Settings.


#include "List/ModularBuildSystemListBase.h"

int32 UModularBuildSystemListBase::GetMaxIndex(UModularSectionResolution* Resolution) const
{
	return FMath::Clamp(GetLength(Resolution) - 1, 0, INT32_MAX);
}
