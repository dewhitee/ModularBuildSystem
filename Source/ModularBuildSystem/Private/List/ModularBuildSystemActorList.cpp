// Fill out your copyright notice in the Description page of Project Settings.


#include "List/ModularBuildSystemActorList.h"
#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"

TSubclassOf<AActor> UModularBuildSystemActorList::GetActorClass(int32 AtIndex, UModularSectionResolution* Resolution) const
{
	if (!Resolution)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Resolution was nullptr when trying to get actor class; AtIndex=%d"),
			*GetName(), AtIndex);
		return nullptr;
	}

	if (Elements.IsEmpty())
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Elements map was empty when trying to get actor class; AtIndex=%d, Resolution=%s"),
			*GetName(), AtIndex, *Resolution->GetName());
		return nullptr;
	}
	
	const FModularBuildSystemActorListElement* Element = Elements.Find(Resolution);
	if (Element && Element->ActorClasses.IsValidIndex(AtIndex))
	{
		return Element->ActorClasses[AtIndex];
	}
	UE_LOG(LogMBS, Error, TEXT("%s: Actor class is not found"), *GetName());
	return nullptr;
}

int32 UModularBuildSystemActorList::GetLength(UModularSectionResolution* Resolution) const
{
	return Resolution && Elements.Contains(Resolution)
		? Elements[Resolution].ActorClasses.Num()
		: Elements.Num();
}
