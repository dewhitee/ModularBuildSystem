// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class IModularBuildSystemInterface;
class AModularBuildSystemActor;
struct FModularLevel;

namespace MBS
{

/**
 * 
 */
class MODULARBUILDSYSTEM_API FModularLevelObserver
{
	TWeakInterfacePtr<IModularBuildSystemInterface> BuildSystem;
	int32 ModifiedLevelId;
	
public:
	FModularLevelObserver();
	FModularLevelObserver(TWeakInterfacePtr<IModularBuildSystemInterface> InBuildSystem);
	~FModularLevelObserver();

	bool OnLevelUpdate(const FName& InStructName, const FName& InCheckedName, FModularLevel& InLevel,
	TFunction<void(FModularLevel&)> InFunction = nullptr);
	bool OnLevelUpdate(const FName& InStructName, const FName& InCheckedName, TArray<FModularLevel>& InLevels,
		int32 InModifiedIndex, TFunction<void(TArray<FModularLevel>&)> InFunction = nullptr);

	bool OnLevelsUpdate(const FName& InStructName, TMap<FName, FModularLevel*> InCheckedLevels, TMap<FName,
		TArray<FModularLevel>*> InCheckedLevelArrays, int32 InModifiedIndex,
		TFunction<void(FModularLevel&)> InFunction = nullptr,
		TFunction<void(TArray<FModularLevel>&)> InArrayFunction = nullptr, 
		TFunction<void()> InNoLevelUpdatedFunction = nullptr);

	void OnNoLevelsUpdate(TFunction<void()> InFunction = nullptr);

	int32 GetModifiedLevelId() const { return ModifiedLevelId; }
};

}
