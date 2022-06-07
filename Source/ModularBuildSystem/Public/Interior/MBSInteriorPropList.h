// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "List/ModularBuildSystemListBase.h"
#include "MBSInteriorPropList.generated.h"

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UMBSInteriorPropList : public UModularBuildSystemListBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<UStaticMesh*> StaticMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<USkeletalMesh*> SkeletalMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<TSubclassOf<AActor>> Actors;
};
