// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularLevelInitializer.h"

#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularLevel.h"

MBS::FModularLevelInitializer::FModularLevelInitializer()
	: BuildSystem(nullptr)
	, LastLevelId(FModularLevel::InvalidLevelId)
{
}

MBS::FModularLevelInitializer::FModularLevelInitializer(TWeakInterfacePtr<IModularBuildSystemInterface> InBuildSystem)
	: BuildSystem(InBuildSystem)
	, LastLevelId(FModularLevel::InvalidLevelId)
{
}

MBS::FModularLevelInitializer::~FModularLevelInitializer()
{
}

void MBS::FModularLevelInitializer::InitSingleLevel(const FName& InLevelName, FModularLevel& InLevel, int32 InLevelIndex,
	uint8& InShiftCount, float InZMultiplier, UModularSectionResolution* InPreviousLevelResolution, float& OutZMultiplier,
	TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction)
{
	const bool bSuccess = InLevel.Init(
		InLevelName,
		Cast<AModularBuildSystemActor>(BuildSystem.GetObject()),
		LastLevelId + 1,
		InLevelIndex,
		InShiftCount,
		InZMultiplier,
		InPreviousLevelResolution,
		InAppendFunction,
		&AModularBuildSystemActor::AppendSections);
	
	PostInitSingleLevel(InLevelName, InLevel, InShiftCount, InZMultiplier, OutZMultiplier, bSuccess);
}

void MBS::FModularLevelInitializer::InitMultipleLevels(const FName& InLevelName, TArray<FModularLevel>& InLevels,
	uint8& InShiftCount, float InZMultiplier, UModularSectionResolution* InPreviousLevelResolution, 
	float& OutZMultiplier, int32& OutCount, TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction)
{
	OutCount = InLevels.Num() - InShiftCount;
	OutZMultiplier = InZMultiplier;
	
	UE_LOG(LogMBS, Verbose, TEXT("%s: (%s level) initializing multiple levels, InLevel.Num()=%d, OutCount=%d, InShiftCount=%d"),
		*BuildSystem.GetObject()->GetName(), *InLevelName.ToString(), InLevels.Num(), OutCount, InShiftCount);
	
	for (int32 i = 0; i < OutCount; i++)
	{
		InitSingleLevel(
			InLevelName,
			InLevels[i],
			i + 1 - InShiftCount,
			InShiftCount,
			InZMultiplier,
			InPreviousLevelResolution,
			OutZMultiplier,
			InAppendFunction);
		
		InZMultiplier = OutZMultiplier;
	}
}

void MBS::FModularLevelInitializer::PostInitSingleLevel(const FName& InLevelName, const FModularLevel& InLevel,
	const uint8 InShiftCount, float InZMultiplier, float& OutZMultiplier, const bool bSuccess) const
{
	OutZMultiplier = InLevel.GetZMultiplierForNextLevel(InZMultiplier);

	if (bSuccess)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: (%s level) InZMultiplier = %.2f, OutZMultiplier = %.2f, InShiftCount = %d, InLevel.Id = %d"),
			*BuildSystem.GetObject()->GetName(), *InLevelName.ToString(), InZMultiplier, OutZMultiplier, InShiftCount, InLevel.GetId());
	}
	else
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: %s level is not initialized."), *BuildSystem.GetObject()->GetName(), *InLevelName.ToString());
	}
}

void MBS::FModularLevelInitializer::FixLevelId(FModularLevel* InLevel)
{
	check(InLevel);
	if (!FModularLevel::IsValidLevelId(InLevel->GetId()))
	{
		InLevel->OverrideLevelId(++LastLevelId);
	}
}
