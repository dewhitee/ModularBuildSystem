// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularBuildSystemGenerator.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"

FGeneratedModularSections::FGeneratedModularSections(const TArray<FModularSection> InSections,
	const TArray<FModularSectionActor> InActorSections, const TArray<FModularSectionInstanced> InInstancedSections)
	: Sections(InSections)
	, ActorSections(InActorSections)
	, InstancedSections(InInstancedSections)
{
}

FGeneratedModularSections::FGeneratedModularSections(const TScriptInterface<IModularBuildSystemInterface> MBS)
	: Sections(MBS->GetSections().GetStatic())
	, ActorSections(MBS->GetSections().GetActor())
	, InstancedSections(MBS->GetSections().GetInstanced())
{
}

FGeneratedModularSections UModularBuildSystemGenerator::Generate_Implementation()
{
	UE_LOG(LogGenerator, Log, TEXT("%s: === Generating ==="), *GetName());
	return FGeneratedModularSections();
}

void UModularBuildSystemGenerator::FinishGeneration_Implementation()
{
	LogGenerationSummary();
	UE_LOG(LogGenerator, Log, TEXT("%s: === End generation === "), *GetName());
}

bool UModularBuildSystemGenerator::CanGenerate() const
{
	return !GetBuildSystemPtr()->IsBuildModeActivated()
		&& CheckProperties()
		&& CheckBounds();
}

void UModularBuildSystemGenerator::PreGenerateClear() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: PreGenerateClear"), *GetName());
	const TScriptInterface<IModularBuildSystemInterface> MBS = GetBuildSystemPtr();
	check(MBS);
	
	// TODO: Handle smart execution mode
	if (MBS->GetSpawnConfiguration().ExecutionMode == EMBSExecutionMode::Smart)
	{
		UE_LOG(LogGenerator, Log, TEXT("%s: PreGenerateClear handle Smart execution mode"), *GetName());
		// TODO: Check other properties if they were modified
		// Only reset sections if bounds were updated.
		const bool bWereBoundsUpdated = MBS->GetTransformBounds().WasUpdated();
		
		UE_LOG(LogGenerator, Verbose, TEXT("%s: MBS->GetTransformBounds().WasUpdated() = %d"), *GetName(), static_cast<int32>(bWereBoundsUpdated));
		if (bWereBoundsUpdated)
		{
			// TODO: Reset only those sections that were modified
			// Update: Reset only in case when building bounds are less that they were before.
			MBS->ResetBuildSystem();
		}
		else
		{
			MBS->ResetBuildSystem(false, true, false, false);
		}
	}
	else
	{
		// Clearing
		MBS->ResetBuildSystem();
	}
}

bool UModularBuildSystemGenerator::PreGenerate(AModularBuildSystemActor* MBS) const
{
	checkf(MBS, TEXT("%s: BuildSystemPtr was nullptr! This should never happen"), *GetName());
	UE_LOG(LogGenerator, Log, TEXT("%s: PreGenerate"), *GetName());

	// Checks
	if (!CanGenerate())
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: Can't generate while conditions not met."), *GetName());
		return false;
	}
	
	// Clear
	PreGenerateClear();

	// Preparing all levels before initialization
	PrepareBuildSystem();

	// Initialize
	MBS->Init();
	
	return true;
}
