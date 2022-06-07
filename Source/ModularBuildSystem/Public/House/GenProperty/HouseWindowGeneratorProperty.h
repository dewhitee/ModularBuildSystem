// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSGeneratorProperty.h"
#include "HouseWindowGeneratorProperty.generated.h"

/**
 * 
 */
UCLASS(DisplayName="House Windows")
class MODULARBUILDSYSTEM_API UHouseWindowGeneratorProperty : public UMBSGeneratorProperty
{
	GENERATED_BODY()

public:
	UHouseWindowGeneratorProperty()
	{
		Data.DebugPropertyName = "House Windows";
		WindowSectionData.DebugPropertyName = "House Window Section Data";
	}
	
	UPROPERTY(EditDefaultsOnly, Category=Window, meta=(DisplayPriority=2, EditCondition="bEnabled"))
	FMBSGeneratorPropertyData WindowSectionData;
	
	UPROPERTY(EditInstanceOnly, Category=Window, meta=(DisplayPriority=2, EditCondition="bEnabled"))
	int32 WindowCount;
	
	UPROPERTY(EditInstanceOnly, Category=Window, meta=(DisplayPriority=2, EditCondition="bEnabled"))
	int32 WindowCountPerLevel = 2;

	virtual bool Init_Implementation(FMBSGeneratorPropertyInitArgs& Args) override;
	virtual bool IsSet_Implementation() const override;

private:
	bool CheckEntrancesCount(FMBSGeneratorPropertyInitArgs& Args) const;
	void AddNewWindowIndex(FMBSGeneratorPropertyInitArgs& Args) const;
	void SetSingleWindow(FMBSGeneratorPropertyInitArgs& Args) const;
};
