// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemListBase.h"
#include "ModularBuildSystemActorList.generated.h"

class UModularSectionResolution;

USTRUCT(BlueprintType)
struct FModularBuildSystemActorListElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<TSubclassOf<AActor>> ActorClasses;
};

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UModularBuildSystemActorList : public UModularBuildSystemListBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TMap<UModularSectionResolution*, FModularBuildSystemActorListElement> Elements;

	TSubclassOf<AActor> GetActorClass(int32 AtIndex, UModularSectionResolution* Resolution) const;
	virtual int32 GetLength(UModularSectionResolution* Resolution) const override;
};
