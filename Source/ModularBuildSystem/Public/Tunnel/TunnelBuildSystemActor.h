// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemActor.h"
#include "TunnelBuildSystemActor.generated.h"

class USplineComponent;
class UTunnelBuildSystemGenerator;

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEM_API ATunnelBuildSystemActor : public AModularBuildSystemActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MBS, meta=(EditCondition="!bBuildModeIsActivated"))
	FModularLevel TunnelBasement;
	
	UPROPERTY(EditAnywhere, Category=MBS, meta=(EditCondition="!bBuildModeIsActivated"))
	TArray<FModularLevel> TunnelWalls;

	UPROPERTY(EditAnywhere, Category=MBS, meta=(EditCondition="!bBuildModeIsActivated"))
	FModularLevel TunnelRoof;

	UPROPERTY(EditAnywhere, Category=MBS, meta=(EditCondition="!bBuildModeIsActivated"))
	FModularLevel TunnelRooftop;

	UPROPERTY(EditInstanceOnly, Instanced, Category=Generator)
	TObjectPtr<UTunnelBuildSystemGenerator> Generator;

	UPROPERTY(EditAnywhere, Category=Generator)
	bool bUseSpline;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Generator, meta=(AllowPrivateAccess=true))
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditInstanceOnly, Category=Generator)
	bool bGenerateOnChange;
	
public:
	ATunnelBuildSystemActor();
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void Init() override;
	virtual TScriptInterface<IBuildingGeneratorInterface> GetGenerator() const override;
	virtual TArray<FModularLevel*> GetAllLevels() const override;
	virtual FModularLevel* GetLevelWithId(int32 Id) const override;
	virtual void CollectStats() override;
	virtual void OnUpdateTransformBounds() override;

private:
	//virtual void SetupSpline() override;

	friend class UTunnelEntranceGeneratorProperty;
	friend class UTunnelBuildSystemGenerator;
};
