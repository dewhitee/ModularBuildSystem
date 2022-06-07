// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSContainers.generated.h"

class UModularBuildSystemMeshList;
class UModularSectionResolution;

USTRUCT(BlueprintType)
struct FMBSMeshListResolutionPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	TObjectPtr<UModularBuildSystemMeshList> MeshList = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	TObjectPtr<UModularSectionResolution> Resolution = nullptr;
};

USTRUCT(BlueprintType)
struct FMBSMeshListProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	TObjectPtr<UModularBuildSystemMeshList> MeshList = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	TObjectPtr<UModularSectionResolution> Resolution = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	int32 MeshListIndex = 0;
};

USTRUCT(BlueprintType)
struct FMBSLevelIdTransformPair
{
	GENERATED_BODY()

	FMBSLevelIdTransformPair();
	FMBSLevelIdTransformPair(const int32 InLevelId, const FTransform& InTransform);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	int32 LevelId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MBS)
	FTransform Transform;
	
};