// Fill out your copyright notice in the Description page of Project Settings.


#include "List/ModularBuildSystemMeshList.h"
#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"
#include "Slate/SlateTextures.h"

int32 UModularBuildSystemMeshList::GetCountOfMeshesWithResolution(UModularSectionResolution* Resolution) const
{
	if (const FModularBuildSystemMeshListElement* Element = Elements.Find(Resolution))
	{
		return Element->StaticMeshes.Num();
	}
	UE_LOG(LogMBS, Error, TEXT("%s: Resolution not found"), *GetName());
	return 0;
}

const UStaticMesh* UModularBuildSystemMeshList::GetConstMesh(int32 AtIndex, const UModularSectionResolution* Resolution) const
{
	if (!Resolution)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Can't get mesh of a NULL Resolution"), *GetName());
		return nullptr;
	}

	UE_LOG(LogMBS, Verbose, TEXT("%s: Getting mesh at index %d with resolution %s"), *GetName(), AtIndex,
		*Resolution->GetName());

	const FModularBuildSystemMeshListElement* Element = Elements.Find(Resolution);
	if (Element && Element->StaticMeshes.IsValidIndex(AtIndex))
	{
		if (!Element->StaticMeshes[AtIndex])
		{
			UE_LOG(LogMBS, Error, TEXT("%s: Found mesh at index %d is nullptr"), *GetName(), AtIndex);
		}
		return const_cast<UStaticMesh*>(Element->StaticMeshes[AtIndex]);
	}
	UE_LOG(LogMBS, Error, TEXT("%s: Mesh not found (AtIndex=%d, Resolution=%s)"), *GetName(),
		AtIndex, Resolution ? *Resolution->GetName() : TEXT("nullptr"));
	return nullptr;
}

UStaticMesh* UModularBuildSystemMeshList::GetMesh(int32 AtIndex, UModularSectionResolution* Resolution) const
{
	return const_cast<UStaticMesh*>(GetConstMesh(AtIndex, Resolution));
}

int32 UModularBuildSystemMeshList::GetLength(UModularSectionResolution* Resolution) const
{
	return Resolution && Elements.Contains(Resolution)
		? Elements[Resolution].StaticMeshes.Num()
		: Elements.Num();
}
