// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSTreemap.h"
#include "Engine/DataAsset.h"
#include "MBSTreemapTreePreset.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSTreemapTreePreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Treemap, meta=(TitleProperty="Name={Name}, Parent={ParentName}, Size={Size}"))
	TArray<FMBSTreeNode> Tree;

	UPROPERTY(EditAnywhere, Category=Treemap)
	EMBSTreemapSizeValueMeasure Measure;
};
