// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularLevelObserver.h"

#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularLevel.h"

MBS::FModularLevelObserver::FModularLevelObserver()
	: BuildSystem(nullptr)
	, ModifiedLevelId(FModularLevel::InvalidLevelId)
{
}

MBS::FModularLevelObserver::FModularLevelObserver(TWeakInterfacePtr<IModularBuildSystemInterface> InBuildSystem)
	: BuildSystem(InBuildSystem)
	, ModifiedLevelId(FModularLevel::InvalidLevelId)
{
}

MBS::FModularLevelObserver::~FModularLevelObserver()
{
}

bool MBS::FModularLevelObserver::OnLevelUpdate(const FName& InStructName, const FName& InCheckedName, FModularLevel& InLevel,
	TFunction<void(FModularLevel&)> InFunction)
{
	if (InStructName == InCheckedName)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: %s parameter changed. ModifiedLevelId = %d"),
			*BuildSystem.GetObject()->GetName(), *InStructName.ToString(), InLevel.GetId());
		ModifiedLevelId = InLevel.GetId();

		if (InFunction != nullptr)
		{
			InFunction(InLevel);
		}
		return true;
	}
	return false;
}

bool MBS::FModularLevelObserver::OnLevelUpdate(const FName& InStructName, const FName& InCheckedName,
	TArray<FModularLevel>& InLevels, int32 InModifiedIndex, TFunction<void(TArray<FModularLevel>&)> InFunction)
{
	if (InStructName == InCheckedName)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s parameter changed."), *InStructName.ToString());
		if (InLevels.IsValidIndex(InModifiedIndex))
		{
			UE_LOG(LogMBS, Verbose, TEXT("%s: InModifiedIndex=%d, ModifiedLevelId=%d"),
				*BuildSystem.GetObject()->GetName(), InModifiedIndex, InLevels[InModifiedIndex].GetId());
			ModifiedLevelId = InLevels[InModifiedIndex].GetId();
		}

		if (InFunction != nullptr)
		{
			InFunction(InLevels);
		}
		return true;
	}
	return false;
}

bool MBS::FModularLevelObserver::OnLevelsUpdate(const FName& InStructName, TMap<FName, FModularLevel*> InCheckedLevels,
	TMap<FName, TArray<FModularLevel>*> InCheckedLevelArrays, int32 InModifiedIndex,
	TFunction<void(FModularLevel&)> InFunction, TFunction<void(TArray<FModularLevel>&)> InArrayFunction,
	TFunction<void()> InNoLevelUpdatedFunction)
{
	for (auto& CheckedLevel : InCheckedLevels)
	{
		if (OnLevelUpdate(InStructName, CheckedLevel.Key, *CheckedLevel.Value, InFunction))
		{
			return true;
		}
	}

	for (auto& CheckedLevelArray : InCheckedLevelArrays)
	{
		if (OnLevelUpdate(InStructName, CheckedLevelArray.Key, *CheckedLevelArray.Value, InModifiedIndex, InArrayFunction))
		{
			return true;
		}
	}

	OnNoLevelsUpdate(InNoLevelUpdatedFunction);
	return false;
}

void MBS::FModularLevelObserver::OnNoLevelsUpdate(TFunction<void()> InFunction)
{
	ModifiedLevelId = FModularLevel::InvalidLevelId;
	UE_LOG(LogMBS, Verbose, TEXT("%s: No levels were updated."), *BuildSystem.GetObject()->GetName());

	if (InFunction != nullptr)
	{
		InFunction();
	}
}
