// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interior/MBSInteriorGenerator.h"
#include "HouseInteriorGenerator.generated.h"

class UHouseStairsGeneratorProperty;
class UMBSTreemap;
class UHouseDoorGeneratorProperty;
class UHouseEntranceGeneratorProperty;
class UHouseWallGeneratorProperty;
class UMBSLSystem;
class AHouseBuildSystemActor;

/**
 * Derive from this class to make your own house interior generators.
 */
UCLASS()
class MODULARBUILDSYSTEM_API UHouseInteriorGenerator : public UMBSInteriorGenerator
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Generator")
	FMBSInteriorGeneratorModularLevelData Floors;

	UPROPERTY(EditInstanceOnly, Category = "Generator")
	bool bUseBasementAsFloor = false;

	//UPROPERTY(EditInstanceOnly, Category = "Generator")
	//EMBSInteriorPlacementMode PlacementMode = EMBSInteriorPlacementMode::Default;

	UPROPERTY(EditAnywhere, Category = "Generator")
	bool bWithStairs;
	
	UPROPERTY(EditAnywhere, Category = "Generator")
	FMBSGeneratorPropertyData Stairs;
	
	UPROPERTY(EditAnywhere, Category = "Generator")
	FMBSGeneratorProperty_Old Ladders;

	UPROPERTY(EditAnywhere, Category = "Generator")
	FMBSGeneratorProperty_Old Furnace;
	FTransform FurnaceTransform;

	UPROPERTY(EditAnywhere, Category = "Generator")
	FMBSGeneratorProperty_Old FurnaceChimney;

	UPROPERTY(EditAnywhere, Instanced, Category="L-System")
	TObjectPtr<UMBSLSystem> InnerWallsLSystem;

	UPROPERTY(EditAnywhere, Instanced, Category="Treemap")
	TObjectPtr<UMBSTreemap> InnerWallsTreemap;

	UPROPERTY(EditAnywhere, Instanced, Category="Properties")
	TObjectPtr<UHouseWallGeneratorProperty> InnerWalls;

	UPROPERTY(EditAnywhere, Instanced, Category="Properties")
	TObjectPtr<UHouseEntranceGeneratorProperty> Entrances;

	UPROPERTY(EditAnywhere, Instanced, Category="Properties")
	TObjectPtr<UHouseDoorGeneratorProperty> Doors;

	UPROPERTY(EditAnywhere, Instanced, Category="L-System")
	TObjectPtr<UMBSLSystem> ItemsLSystem;

	UPROPERTY()
	TObjectPtr<AHouseBuildSystemActor> BuildSystemPtr;

public:
	virtual FGeneratedInterior Generate_Implementation() override;
	virtual void Update_Implementation() override;
	virtual void ApplyPresets_Implementation() override;
	virtual bool SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystem) override;
	virtual TScriptInterface<IModularBuildSystemInterface> GetBuildSystemPtr() const override;

protected:
	virtual bool CheckBounds() const override;
	virtual bool CheckLists() const override;
	virtual bool CheckResolutions() const override;
	virtual bool CanGenerate() const override;
	virtual FTransform CalculateNewTransform(const FMBSRoom& InRoom) const override;
	virtual void Prepare() override;
	
	void CreateRooms(int32 InFloorIndex, const FTransform& InFloorTransform, FGeneratedInterior& OutGenerated);
	void FillRooms(const FModularLevel& InLevel, int32 InFloorIndex, FGeneratedInterior& OutGenerated);

	void AddStairs(int32 InFloorIndex, FGeneratedInterior& OutGenerated) const;
	void AddLadders(int32 InFloorIndex, const FModularLevel& CurrentLevel, FGeneratedInterior& OutGenerated) const;
	void AddFurnace(int32 InFloorIndex, const FModularLevel& CurrentLevel, FGeneratedInterior& OutGenerated,
		FTransform& OutFurnaceTransform) const;
	void AddFurnaceChimney(int32 InFloorIndex, const FModularLevel& CurrentLevel, FGeneratedInterior& OutGenerated,
		const FTransform& InFurnaceTransform) const;
	FModularLevel* GetCurrentModularLevel(const int32 Index) const;
	
private:
	// TODO: Rename or implement inline and remove
	bool AddNewInteriorActor(AActor* Actor, const FInteriorLevel& InteriorLevel, const FMBSRoom& Room) const;
	
	template<class T>
	void AddNewInteriorActorChecked(T* Actor, const FInteriorLevel& InteriorLevel, const FMBSRoom& Room,
		TArray<T*>& InteriorActors, bool bEnableCollision) const
	{
		Actor->SetActorEnableCollision(bEnableCollision);
		if (AddNewInteriorActor(Actor, InteriorLevel, Room))
		{
			InteriorActors.Add(Actor);
		}
	}

	void CreateSingleRoom(FMBSRoom& Room, const FModularLevel& CurrentLevel, int32 InFloorIndex, FBox BSBounds) const;
	void FilterRoomBoundWalls(TArray<FTransform>& BoundWallTransforms, const FMBSRoom& Room, const FModularLevel& CurrentLevel) const;
};
