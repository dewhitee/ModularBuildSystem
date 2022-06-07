// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"
#include "UObject/Interface.h"
#include "BuildingGeneratorInterface.generated.h"

class IModularBuildSystemInterface;

/**
 * Structure that holds all generated static, actor and instanced sections of a single modular building.
 */
USTRUCT(BlueprintType)
struct FGeneratedModularSections
{
	GENERATED_BODY()

	FGeneratedModularSections() {}
	FGeneratedModularSections(const TArray<FModularSection> InSections,
		const TArray<FModularSectionActor> InActorSections, const TArray<FModularSectionInstanced> InInstancedSections);
	FGeneratedModularSections(const TScriptInterface<IModularBuildSystemInterface> MBS);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generator")
	TArray<FModularSection> Sections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generator")
	TArray<FModularSectionActor> ActorSections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generator")
	TArray<FModularSectionInstanced> InstancedSections;

	/**
	 * @return True if any section was generated on Generate() method call.
	 */
	bool Succeeded() const { return !Sections.IsEmpty() || !ActorSections.IsEmpty() || !InstancedSections.IsEmpty(); }
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UBuildingGeneratorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that should be implemented by modular building generators.
 */
class MODULARBUILDSYSTEM_API IBuildingGeneratorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Generates sections for a single build system.
	 * @return Structure holding all generated sections.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Generator")
	FGeneratedModularSections Generate();
};
