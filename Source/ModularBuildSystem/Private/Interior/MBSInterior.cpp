// Fill out your copyright notice in the Description page of Project Settings.


#include "Interior/MBSInterior.h"
#include "Interior/MBSInteriorGenerator.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"

#include "Engine/StaticMeshActor.h"

void UMBSInterior::GenerateInterior(TScriptInterface<IModularBuildSystemInterface> InBuildSystem)
{
	UE_LOG(LogMBSInterior, Log, TEXT("%s: Generating interior..."), *GetName());
	if (Generator)
	{
		UE_LOG(LogMBSInterior, Log, TEXT("%s: Generating interior using %s generator."), *GetName(), *Generator->GetName());
		if (Generator->SetBuildSystemPtr(InBuildSystem))
		{
			// Clear old interior
			ResetInterior();
			
			// Generate new actors
			const FGeneratedInterior GeneratedInterior = IInteriorGeneratorInterface::Execute_Generate(Generator);

			// Save generated interior actors in array
			SaveInterior(GeneratedInterior);

			// Attach all spawned actors to modular build system actor
			for (const auto& Actor : InteriorActors)
			{
				InBuildSystem->AttachActor(Actor, false);
			}
		}
		else
		{
			UE_LOG(LogMBSInterior, Error, TEXT("%s: Build system pointer was not set. Cast failed."), *GetName());
		}
	}
	else
	{
		UE_LOG(LogMBSInterior, Error, TEXT("%s: Can't generate interior due to Generator was nullptr."), *GetName());
	}
}

void UMBSInterior::ResetInterior()
{
	UE_LOG(LogMBSInterior, Log, TEXT("%s: Resetting interior..."), *GetName());
	for (auto& InteriorActor : InteriorActors)
	{
		InteriorActor->Destroy();
		InteriorActor = nullptr;
	}
	InteriorActors.Empty();

	// Clear rooms
	for (auto& Room : Rooms)
	{
		Room.ResetBoundWalls();
	}
	Rooms.Empty();
}

void UMBSInterior::UpdateInterior()
{
	UE_LOG(LogMBSInterior, Log, TEXT("%s: Updating interior..."), *GetName());
	if (Generator)
	{
		IInteriorGeneratorInterface::Execute_Update(Generator);
	}
}

void UMBSInterior::ApplyInteriorPreset()
{
	UE_LOG(LogMBSInterior, Log, TEXT("%s: Applying interior..."), *GetName());
	if (Generator)
	{
		IInteriorGeneratorInterface::Execute_ApplyPresets(Generator);
	}
	else
	{
		UE_LOG(LogMBSInterior, Error, TEXT("%s: Generator is nullptr"), *GetName());
	}
}

void UMBSInterior::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();
		UE_LOG(LogMBSInterior, Log, TEXT("%s: Property=%s"), *GetName(), *PropertyName.ToString());

		if (bUpdateOnPropertyChange)
		{
			UpdateInterior();
		}

		// TODO: Currently works only if Generator has valid BS pointer set previously through GenerateInterior method
		if (bRegenerateOnPropertyChange && Generator && Generator->GetBuildSystemPtr())
		{
			GenerateInterior(Generator->GetBuildSystemPtr());
		}
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UMBSInterior::SaveInterior(const FGeneratedInterior& Generated)
{
	for (const auto& InteriorLevel : Generated.InteriorLevels)
	{
		InteriorActors.Append(InteriorLevel.StaticMeshActors);
		InteriorActors.Append(InteriorLevel.Actors);
		Rooms.Append(InteriorLevel.Rooms);
	}
}
