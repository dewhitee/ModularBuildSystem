// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSRoom.h"
#include "UObject/NoExportTypes.h"
#include "MBSInterior.generated.h"

class IModularBuildSystemInterface;
class IInteriorGeneratorInterface;
class AModularBuildSystemActor;
class UMBSInteriorGenerator;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, meta = (DisplayName = "Interior"))
class MODULARBUILDSYSTEM_API UMBSInterior final : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly, Instanced, Category=Interior)
	TObjectPtr<UMBSInteriorGenerator> Generator;

	UPROPERTY(EditInstanceOnly, Category=Interior)
	bool bUpdateOnPropertyChange;

	UPROPERTY(EditInstanceOnly, Category=Interior)
	bool bRegenerateOnPropertyChange;

	/**
	 * Should be accessed through getter by build system actor to decide if this Interior object should be updated.
	 * @see ShouldRegenerateOnBuildSystemUpdate
	 */
	UPROPERTY(EditInstanceOnly, Category=Interior)
	bool bRegenerateOnBuildSystemUpdate;
	
	UPROPERTY(VisibleAnywhere, Category=Interior)
	TArray<AActor*> InteriorActors;

	UPROPERTY(VisibleAnywhere, Category=Interior)
	TArray<FMBSRoom> Rooms;

public:
	/**
	 * Generates interior for provided modular build system actor.
	 */
	UFUNCTION(BlueprintCallable, Category=Interior)
	void GenerateInterior(TScriptInterface<IModularBuildSystemInterface> InBuildSystem);

	/**
	 * Clears and destroys all interior actors and rooms.
	 */
	UFUNCTION(BlueprintCallable, Category=Interior)
	void ResetInterior();

	UFUNCTION(BlueprintCallable, Category=Interior)
	void UpdateInterior();
	
	UFUNCTION(BlueprintCallable, Category=Interior)
	void ApplyInteriorPreset();

	UFUNCTION(BlueprintCallable, Category=Interior)
	TArray<AActor*> GetInteriorActors() const { return InteriorActors; };

	UFUNCTION(BlueprintCallable, Category=Interior)
	UMBSInteriorGenerator* GetGenerator() const { return Generator; }

	bool ShouldRegenerateOnBuildSystemUpdate() const { return bRegenerateOnBuildSystemUpdate; }
	TArray<FMBSRoom> GetRooms() const { return Rooms; }

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

private:
	void SaveInterior(const struct FGeneratedInterior& Generated);
	
};
