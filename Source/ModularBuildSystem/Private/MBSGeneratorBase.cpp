// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSGeneratorBase.h"

#include "MBSGeneratorProperty.h"
#include "MBSIndexCalculation.h"
#include "ModularBuildSystem.h"
#include "List/ModularBuildSystemActorList.h"
#include "List/ModularBuildSystemMeshList.h"
#include "ModularLevel.h"

FMBSGeneratorPropertyData::FMBSGeneratorPropertyData()
	: PivotLocationOverride(EModularSectionPivotLocation::Default)
    , SnapModeOverride(EModularSectionResolutionSnapMode::Default)
{
}

UStaticMesh* FMBSGeneratorPropertyData::GetMesh(int32 AtIndex, UModularSectionResolution* OverrideResolution) const
{
	if (!MeshList)
	{
		UE_LOG(LogMBSProperty, Error, TEXT("%s.GetRandomMesh tried to access MeshList but MeshList == nullptr (bUseActorList=%s)"),
			*DebugPropertyName.ToString(), bUseActorList ? TEXT("true") : TEXT("false"));
		return nullptr;
	}
	return MeshList->GetMesh(AtIndex, OverrideResolution ? OverrideResolution : Resolution);
}

UStaticMesh* FMBSGeneratorPropertyData::GetRandomMesh(UModularSectionResolution* OverrideResolution) const
{
	UModularSectionResolution* ChosenResolution = OverrideResolution ? OverrideResolution : Resolution;
	if (!MeshList)
	{
		UE_LOG(LogMBSProperty, Error, TEXT("%s.GetRandomMesh tried to access MeshList but MeshList == nullptr (bUseActorList=%s)"),
			*DebugPropertyName.ToString(), bUseActorList ? TEXT("true") : TEXT("false"));
		return nullptr;
	}
	const int32 Index = FMath::RandRange(0, MeshList->GetMaxIndex(ChosenResolution));
	return MeshList->GetMesh(Index, ChosenResolution);
}

TSubclassOf<AActor> FMBSGeneratorPropertyData::GetActorClass(int32 AtIndex, UModularSectionResolution* OverrideResolution) const
{
	if (!ActorList)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s.GetActorClass() tried to access ActorList but ActorList == nullptr (bUseActorList=%s)"),
			*DebugPropertyName.ToString(), bUseActorList ? TEXT("true") : TEXT("false"));
		return {};
	}
	return ActorList->GetActorClass(AtIndex, OverrideResolution ? OverrideResolution : Resolution);
}

TSubclassOf<AActor> FMBSGeneratorPropertyData::GetRandomActorClass(UModularSectionResolution* OverrideResolution) const
{
	if (!ActorList)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s.GetRandomActorClass() tried to access ActorList but ActorList == nullptr (bUseActorList=%s)"),
			*DebugPropertyName.ToString(), bUseActorList ? TEXT("true") : TEXT("false"));
		return {};
	}
	UModularSectionResolution* ChosenResolution = OverrideResolution ? OverrideResolution : Resolution;
	const int32 Index = FMath::RandRange(0, ActorList->GetMaxIndex(ChosenResolution));
	return ActorList->GetActorClass(Index, ChosenResolution);
}

int32 FMBSGeneratorPropertyData::GetIndex(const FMBSIndexCalculationArgs& Args, int32 Default) const
{
	return IndexCalculation ? IndexCalculation->CalculateSingle(Args) : Default;
}

bool FMBSGeneratorPropertyData::IsSet() const
{
	if (bUseActorList && ActorList == nullptr)
	{
		UE_LOG(LogMBSProperty, Error, TEXT("%s.IsSet() resulted in false due to:\t bUseActorList == true && ActorList == nullptr"), *DebugPropertyName.ToString());
	}
	else if (!bUseActorList && MeshList == nullptr)
	{
		UE_LOG(LogMBSProperty, Error, TEXT("%s.IsSet() resulted in false due to:\t bUseActorList == false && MeshList == nullptr"), *DebugPropertyName.ToString());
	}
	else if (Resolution == nullptr)
	{
		UE_LOG(LogMBSProperty, Error, TEXT("%s.IsSet() resulted in false due to:\t Resolution == nullptr"), *DebugPropertyName.ToString());
	}
	return (bUseActorList ? ActorList != nullptr : MeshList != nullptr) && Resolution != nullptr;
}

void UMBSGeneratorBase::LogGenerationSummary() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: Generation summary: "), *GetName());
}

bool UMBSGeneratorBase::CheckProperties() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: Checking properties..."), *GetName());
	return true;
}

bool UMBSGeneratorBase::CheckBounds() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: Checking bounds..."), *GetName());
	return true;
}

void UMBSGeneratorBase::CheckList(const FName InMeshListName, const UModularBuildSystemListBase* InMeshList,
	const bool bIsSupported, bool& bOutResult) const
{
	if (bIsSupported && InMeshList == nullptr)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s: %s == nullptr."), *GetName(), *InMeshListName.ToString());
		bOutResult &= false;
	}
	else if (!bIsSupported)
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: %s not supported."), *GetName(), *InMeshListName.ToString());
	}
}

bool UMBSGeneratorBase::CheckLists() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: Checking mesh lists..."), *GetName());
	return true;
}

void UMBSGeneratorBase::CheckList(const TArray<FCheckListEntry>& CheckListEntries, bool& bOutResult) const
{
	for (auto& Entry : CheckListEntries)
	{
		CheckList(Entry.MeshListName, Entry.MeshList, Entry.bIsSupported, bOutResult);
	}
}

void UMBSGeneratorBase::CheckResolution(const FName InResolutionName, const UModularSectionResolution* InResolution,
	const bool bIsSupported, bool& bOutResult) const
{
	if (bIsSupported && InResolution == nullptr)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s: %s == nullptr."), *GetName(), *InResolutionName.ToString());
		bOutResult &= false;
	}
}

void UMBSGeneratorBase::CheckResolution(const TArray<FCheckResolutionEntry>& ResolutionEntries, bool& bOutResult) const
{
	for (auto& Entry : ResolutionEntries)
	{
		CheckResolution(Entry.ResolutionName, Entry.Resolution, Entry.bIsSupported, bOutResult);
	}
}

EModularSectionPivotLocation UMBSGeneratorBase::GetPivotLocation(const FMBSGeneratorPropertyData& LevelData,
	const FModularLevel& Level)
{
	return LevelData.bOverridePivotLocation ? LevelData.PivotLocationOverride : Level.GetPivotLocation();
}

bool UMBSGeneratorBase::CheckResolutions() const
{
	UE_LOG(LogGenerator, Log, TEXT("%s: Checking resolutions..."), *GetName());
	return true;
}

bool UMBSGeneratorBase::CanGenerate() const
{
	return CheckProperties() && CheckBounds();
}

void UMBSGeneratorBase::CheckPropertyFull(const FName InPropertyName, const UMBSGeneratorProperty* InProperty,
	const bool bSupported, bool& bOutResult) const
{
	CheckProperty(InPropertyName, InProperty, bSupported, bOutResult);
	if (InProperty)
	{
		InProperty->Data.bUseActorList
			? CheckList(*FString::Printf(TEXT("%s_ActorList"), *InPropertyName.ToString()), InProperty->Data.ActorList, bSupported, bOutResult)
			: CheckList(*FString::Printf(TEXT("%s_MeshList"), *InPropertyName.ToString()), InProperty->Data.MeshList, bSupported, bOutResult);
		
		CheckResolution(*FString::Printf(TEXT("%s_Resolution"), *InPropertyName.ToString()), InProperty->Data.Resolution, bSupported, bOutResult);
	}
	else if (bSupported)
	{
		UE_LOG(LogGenerator, Error, TEXT("%s: %s property is supported but not set."), *GetName(), *InPropertyName.ToString());
	}
}

void UMBSGeneratorBase::CheckProperty(const FName InPropertyName, const UMBSGeneratorProperty* InProperty,
	const bool bSupported, bool& bOutResult) const
{
	if (bSupported)
	{
		const bool bWasResultFalseAlready = !bOutResult;
		bOutResult &= InProperty != nullptr && InProperty->IsSet();
		if (!bOutResult)
		{
			if (InProperty == nullptr)
			{
				UE_LOG(LogGenerator, Error, TEXT("%s: Property %s is not valid."), *GetName(), *InPropertyName.ToString());
			}
			else if (!InProperty->IsSet())
			{
				UE_LOG(LogGenerator, Error, TEXT("%s: Property %s is not set."), *GetName(), *InPropertyName.ToString());
			}
			else if (!bWasResultFalseAlready)
			{
				UE_LOG(LogGenerator, Error, TEXT("%s: Check of %s property resulted in bOutResult=false."), *GetName(), *InPropertyName.ToString());
			}
		}
	}
	else
	{
		UE_LOG(LogGenerator, Warning, TEXT("%s: %s not supported"), *GetName(), *InPropertyName.ToString());
		bOutResult &= true;
	}
}

void UMBSGeneratorBase::CheckProperty(const TArray<FCheckPropertyEntry>& CheckPropertyEntries, bool& bOutResult) const
{
	for (auto& Entry : CheckPropertyEntries)
	{
		CheckPropertyFull(Entry.PropertyName, Entry.Property, Entry.bIsSupported, bOutResult);
	}
}
