// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interior/MBSRoom.h"
#include "UObject/Interface.h"
#include "InteriorGeneratorInterface.generated.h"

class IModularBuildSystemInterface;
class ASkeletalMeshActor;

/**
 * Structure that holds all interior elements in a form of static, skeletal and other actor arrays,
 * as well as array of all generated rooms of a single modular level of a building.
 */
USTRUCT(BlueprintType)
struct FInteriorLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InteriorGenerator")
	TArray<AStaticMeshActor*> StaticMeshActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InteriorGenerator")
	TArray<ASkeletalMeshActor*> SkeletalMeshActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InteriorGenerator")
	TArray<AActor*> Actors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InteriorGenerator")
	TArray<FMBSRoom> Rooms;
};

/**
 * Structure that holds the array of generated interior levels of a single modular building.
 */
USTRUCT(BlueprintType)
struct FGeneratedInterior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteriorGenerator")
	TArray<FInteriorLevel> InteriorLevels;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UInteriorGeneratorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that should be implemented by modular building interior generators.
 */
class MODULARBUILDSYSTEM_API IInteriorGeneratorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Generates interior for modular build system actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteriorGenerator")
	FGeneratedInterior Generate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteriorGenerator")
	void Update();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteriorGenerator")
	void ApplyPresets();
};
