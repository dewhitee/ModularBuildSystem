// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemGenerator.h"
#include "TunnelBuildSystemGenerator.generated.h"

class UTunnelEntranceGeneratorProperty;
class UTunnelBasementGeneratorProperty;
class UTunnelWallGeneratorProperty;
class UTunnelRoofGeneratorProperty;
class UTunnelRooftopGeneratorProperty;
class ATunnelBuildSystemActor;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API UTunnelBuildSystemGenerator : public UModularBuildSystemGenerator
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ATunnelBuildSystemActor> BuildSystemPtr;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UTunnelBasementGeneratorProperty> TunnelBasement;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UTunnelWallGeneratorProperty> TunnelWall;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UTunnelRoofGeneratorProperty> TunnelRoof;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UTunnelRooftopGeneratorProperty> TunnelRooftop;

	UPROPERTY(EditAnywhere, Instanced, Category=Generator)
	TObjectPtr<UTunnelEntranceGeneratorProperty> Entrance;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Generator, meta=(AllowPrivateAccess=true))
	FIntPoint Bounds;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Generator, meta=(AllowPrivateAccess=true))
	int32 LevelCount;
	
public:
	UTunnelBuildSystemGenerator();

	virtual FGeneratedModularSections Generate_Implementation() override;
	virtual bool SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystemPtr) override;
	virtual TScriptInterface<IModularBuildSystemInterface> GetBuildSystemPtr() const override;
	virtual bool CheckLists() const override;
	virtual void PrepareBuildSystem() const override;
	virtual bool CanGenerate() const override;

	FIntPoint GetBounds() const		{ return Bounds; }
	void SetBounds(FIntPoint Value) { Bounds = Value; }
	
	int32 GetLevelCount() const		{ return LevelCount; }
	void SetLevelCount(int32 Value) { LevelCount = Value; }
};
