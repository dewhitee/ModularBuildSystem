// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSContainers.h"
#include "ModularLevel.h"

FMBSLevelIdTransformPair::FMBSLevelIdTransformPair()
	: LevelId(FModularLevel::InvalidLevelId)
	, Transform(FTransform::Identity)
{
}

FMBSLevelIdTransformPair::FMBSLevelIdTransformPair(const int32 InLevelId, const FTransform& InTransform)
	: LevelId(InLevelId)
	, Transform(InTransform)
{
}
