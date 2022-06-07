// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemGenerator.h"
#include "HouseBuildSystemGenerator.generated.h"

struct FMBSLevelIdTransformPair;
class UHouseVegetationGeneratorProperty;
class UHouseRooftopGeneratorProperty;
class UHouseChimneyGeneratorProperty;
class UHouseRoofGeneratorProperty;
class UHouseCornerGeneratorProperty;
class UHouseStairsGeneratorProperty;
class UHouseDoorGeneratorProperty;
class UHouseEntranceGeneratorProperty;
class UHouseWallGeneratorProperty;
class UHouseFloorGeneratorProperty;
class UHouseBasementGeneratorProperty;
class UHouseWindowGeneratorProperty;
class AModularBuildSystemActor;
class AHouseBuildSystemActor;
class UModularBuildSystemActorList;
class UModularLevelShape;

/**
 * Base abstract class for house build system generators. Use this class to create your own house generators.
 * Contains functionality to generate elements related to the house.
 */
UCLASS(/*Abstract*/)
class MODULARBUILDSYSTEM_API UHouseBuildSystemGenerator : public UModularBuildSystemGenerator
{
	GENERATED_BODY()
	
public:
	UHouseBuildSystemGenerator();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Generator")
	FIntPoint Bounds;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Generator", meta=(ClampMin=0))
	int32 LevelCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Generator")
	TObjectPtr<UModularLevelShape> CustomLevelShape;

private:	
	// Basement
	UPROPERTY(EditAnywhere, Instanced, Category="Generator")
	TObjectPtr<UHouseBasementGeneratorProperty> Basement_New;
	
	// Floors
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	bool bCanHaveFloors;
	
	UPROPERTY(EditAnywhere, Instanced, Category="Generator|Floor")
	TObjectPtr<UHouseFloorGeneratorProperty> Floor_New;

	// Attic
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveAttic;
	// We don't need additional data here (yet)
	// TODO: Add Attic GenProperty

	// Walls
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveWalls;
	
	UPROPERTY(EditAnywhere, Instanced, Category="Generator", meta=(EditCondition="bCanHaveWalls", EditConditionHides))
	TObjectPtr<UHouseWallGeneratorProperty> Walls_New;

	// Entrances
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	bool bCanHaveEntrances;

	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveEntrances", EditConditionHides))
	TObjectPtr<UHouseEntranceGeneratorProperty> Entrances_New;

	// Doors
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveDoors;

	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveDoors", EditConditionHides))
	TObjectPtr<UHouseDoorGeneratorProperty> Doors_New;

	// Windows
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	bool bCanHaveWindows;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveWindows", EditConditionHides))
	TObjectPtr<UHouseWindowGeneratorProperty> Windows_New;

	// Stairs
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	bool bCanHaveStairs;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveStairs", EditConditionHides))
	TObjectPtr<UHouseStairsGeneratorProperty> Stairs_New;

	// Corners
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	bool bCanHaveCorners;

	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveCorners"))
	TObjectPtr<UHouseCornerGeneratorProperty> Corners_New2;
	
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveRoof;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveRoof"))
	TObjectPtr<UHouseRoofGeneratorProperty> Roof_New;

	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveRooftop;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveRooftop", EditConditionHides))
	TObjectPtr<UHouseRooftopGeneratorProperty> Rooftop_New;
	
	// Chimney
	// TODO: Move to subclass?
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveChimney;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveChimney", EditConditionHides))
	TObjectPtr<UHouseChimneyGeneratorProperty> Chimney_New;

	// Vegetation
	UPROPERTY(EditDefaultsOnly, Category="Generator")
	bool bCanHaveVegetation;
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category="Generator", meta=(EditCondition="bCanHaveVegetation", EditConditionHides))
	TObjectPtr<UHouseVegetationGeneratorProperty> Vegetation_New;

	UPROPERTY(EditAnywhere, Category="Generator")
	FMBSGeneratorProperty_Old RoofWindows;
	
	UPROPERTY(EditAnywhere, Category="Generator")
	FMBSGeneratorProperty_Old InterWallCorners;
	
	UPROPERTY()
	TObjectPtr<AHouseBuildSystemActor> BuildSystemPtr;

public:
	virtual FGeneratedModularSections Generate_Implementation() override;
	virtual bool SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystem) override;
	virtual TScriptInterface<IModularBuildSystemInterface> GetBuildSystemPtr() const override;

	bool CanHaveFloors() const { return bCanHaveFloors; }
	UHouseFloorGeneratorProperty* GetFloor() const { return Floor_New; }
	
protected:
	virtual void LogGenerationSummary() const override;

	virtual bool CheckProperties() const override;
	virtual bool CheckBounds() const override;
	virtual bool CheckLists() const override;
	virtual bool CheckResolutions() const override;

	virtual void PreGenerateClear() const override;
	virtual void PrepareBuildSystem() const override;
	
	virtual void SetEntrances(TArray<int32>& OutEntranceIndices, TArray<FTransform>& OutEntranceTransforms) const;
	virtual void SetDoors(const TArray<int32>& InEntranceIndices, const TArray<FTransform>& InEntranceTransforms) const;
	virtual void SetStairs(const TArray<int32>& InEntranceIndices, const TArray<FTransform>& InEntranceTransforms) const;
	//virtual void SetWindows(const TArray<int32>& InEntranceIndices, TArray<int32>& OutWindowIndices) const;
	virtual void SetChimney() const;
	virtual void SetVegetation() const;
	virtual void SetRoofWindows() const;
	virtual void SetInterWallCorners() const;
	virtual void SetFloorHoles(TArray<FMBSLevelIdTransformPair>& OutFloorHoleIdTransforms);
	virtual void SetFloorHoleDoor(const TArray<FMBSLevelIdTransformPair>& InFloorHoleIdTransforms) const;

	void PrintHeader(const FString& InPropertyName) const;
	
private:
	bool CheckEntrancesCount(const FString& InLevelName, const TArray<int32>& InEntranceIndices) const;
	//void AddNewWindowIndex(int32 AtIndex, int32 WallLevelIndex, TArray<int32>& OutWindowIndices) const;
	//void SetSingleWindow(int32 AtIndex, int32 WallLevelIndex, TArray<int32>& OutWindowIndices) const;
};
