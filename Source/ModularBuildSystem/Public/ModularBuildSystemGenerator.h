// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingGeneratorInterface.h"
#include "UObject/NoExportTypes.h"
#include "MBSGeneratorBase.h"
#include "ModularBuildSystemGenerator.generated.h"

class UModularBuildSystemMeshList;
class UModularBuildSystemListBase;
class UModularSectionResolution;

/**
 * This class handles construction of modular build system actor's modular sections.
 * Supposed to be extended by subclasses and used as a uproperty inside ModularBuildSystemActor derived classes.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class MODULARBUILDSYSTEM_API UModularBuildSystemGenerator : public UMBSGeneratorBase, public IBuildingGeneratorInterface
{
	GENERATED_BODY()
	
public:
	/**
	 * @copydoc IBuildingGeneratorInterface::Generate
	 * @note Requires that build system pointer from GetBuildSystemPtr() method is valid.
	 * @see GetBuildSystemPtr
	 */
	virtual FGeneratedModularSections Generate_Implementation() override;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Generator")
	void FinishGeneration();
	virtual void FinishGeneration_Implementation();

	virtual void PrepareBuildSystem() const
	{
		unimplemented();
	};

	virtual bool CanGenerate() const override;

	// TODO: Move to ModularBuildSystemActor class. Call before Generator->Generate() method to handle clearing of sections.
	virtual void PreGenerateClear() const;
	
	/**
	 * Checks if generation can be performed, and then pre-generates and initializes MBS actor
	 * @param MBS Pointer to the modular build system actor.
	 * @return True if generation can be performed.
	 */
	bool PreGenerate(AModularBuildSystemActor* MBS) const;
};
