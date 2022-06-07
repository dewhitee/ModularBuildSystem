// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSSections.h"

#include "MBSFunctionLibrary.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

FModularSection FMBSSections::InitStatic(UStaticMesh* InStaticMesh, const FTransform& InTransform,
	int32 InLevelId, bool bAddToSections, bool bWithRelativeTransform)
{
	check(InStaticMesh);
	check(FModularLevel::IsValidLevelId(InLevelId));
	
	FModularSection NewSection = FModularSection(InLevelId,
		SpawnNewSectionStaticMeshActor(InTransform, BS->GetSpawnConfiguration().SectionSpawnParams));

	UE_LOG(LogMBS, Verbose, TEXT("%s: %s - static mesh actor spawned at %s location"),
		*UMBSFunctionLibrary::GetDisplayName(BS), *NewSection.GetName(), *InTransform.GetLocation().ToCompactString());
	
	BS->AttachActor(NewSection.GetStaticMeshActor(), bWithRelativeTransform);
	NewSection.SetMesh(InStaticMesh);

	if (bAddToSections)
	{
		Static.Add(NewSection);
	}

#if WITH_EDITOR
	UE_LOG(LogMBS, Verbose, TEXT("%s: %s - modular section has been initialized with %s static mesh"),
		*UMBSFunctionLibrary::GetDisplayName(BS), *NewSection.GetStaticMeshActor()->GetActorLabel(), *InStaticMesh->GetName());
#endif
	
	return NewSection;
}

FModularSectionActor FMBSSections::InitActor(const FTransform& InTransform, int32 InLevelId,
	TSubclassOf<AActor> InClass, bool bAddToActorSections, bool bWithRelativeTransform)
{
	check(FModularLevel::IsValidLevelId(InLevelId));
	
	FModularSectionActor NewSectionActor = FModularSectionActor(InLevelId,
		SpawnNewSectionActor(InTransform, InClass, BS->GetSpawnConfiguration().SectionSpawnParams));
	BS->AttachActor(NewSectionActor.GetActor(), bWithRelativeTransform);

	if (bAddToActorSections)
	{
		Actor.Add(NewSectionActor);
	}
	
#if WITH_EDITOR	
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: %s - modular section actor (of %s class) has been initialized"),
		*UMBSFunctionLibrary::GetDisplayName(BS), *NewSectionActor.GetActor()->GetActorLabel(), *InClass->GetName());
#endif
	
	return NewSectionActor;
}

FModularSectionInstanced FMBSSections::InitInstanced(int32 InLevelId, bool bAddToInstancedSections,
	UInstancedStaticMeshComponent* InInstancedStaticMeshComponent)
{
	check(FModularLevel::IsValidLevelId(InLevelId));
	FModularSectionInstanced NewSection = FModularSectionInstanced(InLevelId, InInstancedStaticMeshComponent);

	if (bAddToInstancedSections)
	{
		Instanced.Add(NewSection);
	}

	return NewSection;
}

FModularSection* FMBSSections::ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh, int32 InInstanceIndex,
	int32 InLevelId, UInstancedStaticMeshComponent* InComponent, FTransform& OutReplacedInstanceTransform)
{
	check(InComponent);
	if (InComponent->GetInstanceTransform(InInstanceIndex, OutReplacedInstanceTransform))
	{
		InComponent->RemoveInstance(InInstanceIndex);
	}
	else
	{
		UE_LOG(LogMBSSection, Error, TEXT("%s: Can't replace with non instanced section. InInstanceIndex = %d, InstanceCount = %d"),
			*UMBSFunctionLibrary::GetDisplayName(BS), InInstanceIndex, InComponent->GetInstanceCount());
		return nullptr;
	}

	const int32 NewSectionIndex = Static.Add(InitStatic(InNewStaticMesh, OutReplacedInstanceTransform,
		InLevelId, false, true));
	return &Static[NewSectionIndex];
}

FModularSection* FMBSSections::ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh, int32 InInstanceIndex,
	int32 InLevelId, FTransform& OutReplacedInstanceTransform)
{
	if (!FModularLevel::IsValidLevelId(InLevelId))
	{
		return nullptr;
	}

	if (const FModularSectionInstanced* Section = this->GetInstancedSectionOfLevel(InLevelId))
	{
		return ReplaceWithNonInstancedSection(
			InNewStaticMesh, InInstanceIndex, InLevelId, Section->GetISMC(), OutReplacedInstanceTransform);
	}

	return nullptr;
}

void FMBSSections::AddNewInstance(const FTransform& InTransform, bool bWithRelativeTransform,
	UInstancedStaticMeshComponent* InInstancedStaticMeshComponent)
{
	if (InInstancedStaticMeshComponent)
	{
		InInstancedStaticMeshComponent->AddInstance(InTransform, !bWithRelativeTransform);
	}
#if WITH_EDITOR
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Can't init modular section with InstancedStaticMeshes of HierarchicalInstancedStaticMeshes mesh configuration type when"
			" InInstancedStaticMeshComponent is nullptr."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}
#endif
}

void FMBSSections::SetMeshForEach(const FModularLevel& InLevel, int32 InEachElement, UStaticMesh* InMesh)
{
	check(InMesh);

	UE_LOG(LogMBSSection, Verbose, TEXT("%s: Updating each %d section mesh of level with Id=%d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), InEachElement, InLevel.GetId());
	const int32 SectionsLength = Static.Num();

	TArray<int32> Indices;
	Indices.Reserve(SectionsLength);

	for (int32 i = 0; i < SectionsLength; i++)
	{
		if (Static[i].GetLevelId() == InLevel.GetId())
		{
			Indices.Add(i);
		}
	}

	const int32 IndicesCount = Indices.Num();
	for (int32 i = 0; i < IndicesCount; i++)
	{
		if (i % InEachElement == 0)
		{
			Static[Indices[i]].SetMesh(InMesh);
		}
	}
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: %d sections were updated."), *UMBSFunctionLibrary::GetDisplayName(BS), IndicesCount);
}

void FMBSSections::SetMeshAt(const FModularLevel& InLevel, int32 InElement, int32 InRow, UStaticMesh* InMesh)
{
	check(BS);
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: SetMeshAt InElement=%d InRow=%d"), *UMBSFunctionLibrary::GetDisplayName(BS), InElement, InRow);

	if (!InMesh)
	{
		UE_LOG(LogMBSSection, Error, TEXT("%s: InMesh was nullptr"), *UMBSFunctionLibrary::GetDisplayName(BS));
		return;
	}

	if (const FModularSection* Section = GetSectionAt(InLevel, InElement, InRow))
	{
		if (!Section->GetStaticMeshActor())
		{
			UE_LOG(LogMBSSection, Error, TEXT("%s: Section has been found (LevelId=%d) but it's StaticMesh was NULL"),
				*UMBSFunctionLibrary::GetDisplayName(BS), Section->GetLevelId());
		}
		else if (!Section->GetStaticMeshActor()->GetStaticMeshComponent())
		{
			UE_LOG(LogMBSSection, Error, TEXT("%s: Section has been found (LevelId=%d) but it's StaticMesh->GetStaticMeshComponent() returned NULL"),
				*UMBSFunctionLibrary::GetDisplayName(BS), Section->GetLevelId());
		}
		else
		{
			Section->SetMesh(InMesh);
		}
	}
}

void FMBSSections::SetMeshAt(const FModularLevel& InLevel, int32 InIndex, UStaticMesh* InMesh)
{
	check(BS);
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: SetMeshAt InIndex=%d"), *UMBSFunctionLibrary::GetDisplayName(BS), InIndex);
	if (!InMesh)
	{
		UE_LOG(LogMBSSection, Error, TEXT("%s: InMesh was nullptr"), *UMBSFunctionLibrary::GetDisplayName(BS));
		return;
	}

	if (const FModularSection* Section = GetSectionAt(InLevel, InIndex))
	{
		if (!Section->GetStaticMeshActor())
		{
			UE_LOG(LogMBSSection, Error, TEXT("%s: Section has been found (LevelId=%d) but it's StaticMesh was NULL"),
				*UMBSFunctionLibrary::GetDisplayName(BS), Section->GetLevelId());
		}
		else if (!Section->GetStaticMeshActor()->GetStaticMeshComponent())
		{
			UE_LOG(LogMBSSection, Error, TEXT("%s: Section has been found (LevelId=%d) but it's StaticMesh->GetStaticMeshComponent() returned NULL"),
				*UMBSFunctionLibrary::GetDisplayName(BS), Section->GetLevelId());
		}
		else
		{
			Section->SetMesh(InMesh);
		}
	}
}

void FMBSSections::UpdateInstanced()
{
}

void FMBSSections::UpdateInstanceCount(FModularSectionInstanced& InSection)
{
	if (ensure(InSection.GetISMC()))
	{
#if WITH_EDITOR
		const int32 CurrentInstanceCount = InSection.GetInstanceCount();
		TotalInstanceCount += CurrentInstanceCount - InSection.GetPreviousInstanceCount();
		InSection.SetPreviousInstanceCount(CurrentInstanceCount);
#else
		InSection.InstanceCount = InSection.InstancedStaticMeshComponent->GetInstanceCount();
#endif
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Can't update instance count because InSection.InstancedStaticMeshComponent is nullptr."),
			*UMBSFunctionLibrary::GetDisplayName(BS));
	}
}

AStaticMeshActor* FMBSSections::SpawnNewSectionStaticMeshActor(const FTransform& InTransform,
	const FActorSpawnParameters& SpawnParams) const
{
	return BS.GetObject()->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), InTransform, SpawnParams);
}

AActor* FMBSSections::SpawnNewSectionActor(const FTransform& InTransform, TSubclassOf<AActor> InClass,
	const FActorSpawnParameters& SpawnParams) const
{
	return BS.GetObject()->GetWorld()->SpawnActor<AActor>(InClass, InTransform, SpawnParams);
}

FMBSSections::FMBSSections(const TScriptInterface<IModularBuildSystemInterface> InBuildSystemActor)
{
	check(InBuildSystemActor);
	BS = InBuildSystemActor;
	Static		= BS->GetSections().Static;
	Actor		= BS->GetSections().Actor;
	Instanced	= BS->GetSections().Instanced;
}

FModularSection* FMBSSections::GetSectionAt(const FModularLevel& InLevel, int32 InIndex) const
{
	check(BS);
	TArray<FModularSection*> LevelSections = GetStaticSectionsOfLevel(InLevel);

	if (LevelSections.IsValidIndex(InIndex))
	{
		UE_LOG(LogMBSSection, Verbose, TEXT("%s: GetSectionAt: Index=%d is valid (LevelSections.Num()=%d)"),
			*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, LevelSections.Num());
		return LevelSections[InIndex];
	}
	UE_LOG(LogMBSSection, Error, TEXT("%s: Condition failed: LevelSections.IsValidIndex(%d). LevelSections.Num()=%d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, LevelSections.Num());

	return nullptr;
}

FModularSection* FMBSSections::GetSectionAt(const FModularLevel& InLevel, int32 InElement, int32 InRow) const
{
	check(BS);
	TArray<FModularSection*> LevelSections = GetStaticSectionsOfLevel(InLevel);

	const int32 Index = InElement + (InLevel.GetInitializer().GetMaxInRow() * InRow);
	UE_LOG(LogMBSSection, VeryVerbose, TEXT("%s: Index=%d (InElement=%d, InLevel.Initializer.MaxInRow=%d, InRow=%d)"), 
		*UMBSFunctionLibrary::GetDisplayName(BS), Index, InElement, InLevel.GetInitializer().GetMaxInRow(), InRow);
	if (LevelSections.IsValidIndex(Index))
	{
		return LevelSections[Index];
	}

	return nullptr;
}

FTransform FMBSSections::GetSectionTransformAt(int32 InLevelId, int32 InIndex, bool bInstanced, bool bWorldSpace) const
{
	check(BS);
	if (const FModularLevel* Level = BS->GetLevelWithId(InLevelId))
	{
		FTransform OutTransform = GetSectionTransformAt(*Level, InIndex, bInstanced, bWorldSpace);
		UE_LOG(LogMBSSection, Verbose, TEXT("%s: Section at %d index has location of %s"),
			*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, *OutTransform.GetLocation().ToCompactString());
		return OutTransform;
	}
	UE_LOG(LogMBSSection, Error, TEXT("%s: Level with %d not found on GetSectionTransformAt."),
		*UMBSFunctionLibrary::GetDisplayName(BS), InLevelId);
	return {};
}

FTransform FMBSSections::GetSectionTransformAt(const FModularLevel& InLevel, int32 InIndex, bool bInstanced,
	bool bWorldSpace) const
{
	check(BS);
	if (!bInstanced)
	{
		if (const FModularSection* Section = GetSectionAt(InLevel, InIndex))
		{
			if (bWorldSpace)
			{
				return Section->GetStaticMeshActor()->GetActorTransform();
			}
			return Section->GetStaticMeshActor()->GetActorTransform().GetRelativeTransform(BS->GetBuildSystemTransform());
		}
		UE_LOG(LogMBSSection, Error, TEXT("%s: Section at index=%d (LevelId=%d) not found."),
			*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, InLevel.GetId());
	}
	else
	{
		if (const FModularSectionInstanced* Section = GetInstancedSectionOfLevel(InLevel))
		{
			FTransform OutTransform;
			if (!Section->GetISMC()->GetInstanceTransform(InIndex, OutTransform, bWorldSpace))
			{
				UE_LOG(LogMBSSection, Error, TEXT("%s: Instance with index=%d (LevelId=%d) not found on GetInstanceTransform."),
					*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, InLevel.GetId());
			}
			return OutTransform;
		}
		UE_LOG(LogMBSSection, Error, TEXT("%s: Instance at index=%d (LevelId=%d) not found."),
			*UMBSFunctionLibrary::GetDisplayName(BS), InIndex, InLevel.GetId());
	}

	return FTransform();
}

TArray<FTransform> FMBSSections::GetSectionTransformAt(const FModularLevel& InLevel, TArray<int32> InIndices,
	bool bInstanced, bool bWorldSpace) const
{
	check(BS);
	TArray<FTransform> OutTransforms;
	for (const int32 Index : InIndices)
	{
		OutTransforms.Add(GetSectionTransformAt(InLevel, Index, bInstanced, bWorldSpace));
	}
	return OutTransforms;
}

FTransform FMBSSections::GetSectionTransformAtRandom(int32 InLevelId, bool bInstanced, bool bWorldSpace) const
{
	check(BS);
	if (const FModularLevel* Level = BS->GetLevelWithId(InLevelId))
	{
		const int32 Index = FMath::RandRange(0, Level->GetInitializer().GetAdjustedTotalCount() - 1);
		FTransform OutTransform = GetSectionTransformAt(*Level, Index, bInstanced, bWorldSpace);
		UE_LOG(LogMBSSection, Verbose, TEXT("%s: Section at %d index has location of %s"),
			*UMBSFunctionLibrary::GetDisplayName(BS), Index, *OutTransform.GetLocation().ToCompactString());
		return OutTransform;
	}
	UE_LOG(LogMBSSection, Error, TEXT("%s: Level with %d not found on GetSectionTransformAtRandom."),
		*UMBSFunctionLibrary::GetDisplayName(BS), InLevelId);
	return {};
}

FTransform FMBSSections::GetSectionTransformAtRandom(const FModularLevel& InLevel, bool bInstanced,
	bool bWorldSpace) const
{
	const int32 Index = FMath::RandRange(0, InLevel.GetInitializer().GetAdjustedTotalCount() - 1);
	return GetSectionTransformAt(InLevel, Index, bInstanced, bWorldSpace);
}

TArray<FModularSection*> FMBSSections::GetStaticSectionsOfLevel(const FModularLevel& InLevel) const
{
	return GetStaticSectionsOfLevel(InLevel.GetId());
}

TArray<FModularSection*> FMBSSections::GetStaticSectionsOfLevel(const int32 InLevelId) const
{
	//UE_LOG(LogBuildSystem, Log, TEXT("%s: Get sections of level"), *GetName());
	TArray<FModularSection*> OutSections;
	OutSections.Reserve(Static.Num());

	for (auto& Section : Static)
	{
		if (Section.GetLevelId() == InLevelId)
		{
			OutSections.Add(const_cast<FModularSection*>(&Section));
		}
	}

	return OutSections;
}

TArray<FModularSectionActor*> FMBSSections::GetActorSectionsOfLevel(const FModularLevel& InLevel) const
{
	return GetActorSectionsOfLevel(InLevel.GetId());
}

TArray<FModularSectionActor*> FMBSSections::GetActorSectionsOfLevel(const int32 InLevelId) const
{
	TArray<FModularSectionActor*> OutSections;
	OutSections.Reserve(Actor.Num());

	for (auto& Section : Actor)
	{
		if (Section.GetLevelId() == InLevelId)
		{
			OutSections.Add(const_cast<FModularSectionActor*>(&Section));
		}
	}

	return OutSections;
}

FModularSectionInstanced* FMBSSections::GetInstancedSectionOfLevel(const FModularLevel& InLevel) const
{
	return GetInstancedSectionOfLevel(InLevel.GetId());
}

FModularSectionInstanced* FMBSSections::GetInstancedSectionOfLevel(const int32 InLevelId) const
{
	//UE_LOG(LogBuildSystem, Log, TEXT("%s: Get instanced sections of level"), *GetName());
	FModularSectionInstanced* OutSection = nullptr;

	for (auto& Section : Instanced)
	{
		if (Section.GetLevelId() == InLevelId)
		{
			OutSection = const_cast<FModularSectionInstanced*>(&Section);
		}
	}

	if (!OutSection)
	{
		UE_LOG(LogMBSSection, Warning, TEXT("%s: GetInstancedSectionOfLevel: OutSection = nullptr. InstancedSections.Num()=%d"),
			*UMBSFunctionLibrary::GetDisplayName(BS), Instanced.Num());
	}
	
	return OutSection;
}

TArray<FModularSectionBase*> FMBSSections::GetAll()
{
	TArray<FModularSectionBase*> OutSections;
	for (int32 i = 0; i < Static.Num(); i++)
	{
		OutSections.Add(&Static[i]);
	}

	for (int32 i = 0; i < Actor.Num(); i++)
	{
		OutSections.Add(&Actor[i]);
	}

	for (int32 i = 0; i < Instanced.Num(); i++)
	{
		OutSections.Add(&Instanced[i]);
	}
	return OutSections;
}

void FMBSSections::SelectSections(const FModularLevel& InLevel) const
{
	UE_LOG(LogMBS, Log, TEXT("%s: Start single level selection."), *UMBSFunctionLibrary::GetDisplayName(BS));
	if (BS->GetMeshConfiguration().IsOfInstancedType())
	{
		if (const FModularSectionInstanced* Section = GetInstancedSectionOfLevel(InLevel); Section->IsValid())
		{
			Section->GetISMC()->SelectInstance(true, 0, Section->GetInstanceCount());
		}
		if (InLevel.InstancedStaticMeshComponent)
		{
			InLevel.InstancedStaticMeshComponent->SelectInstance(true, 0,
				InLevel.InstancedStaticMeshComponent->GetInstanceCount());
		}
	}
	else
	{
		for (const auto& Section : GetStaticSectionsOfLevel(InLevel))
		{
			if (Section->GetStaticMeshActor())
			{
				GEditor->SelectActor(Section->GetStaticMeshActor(), true, true, true);
			}
		}
	}
	UE_LOG(LogMBS, Log, TEXT("%s: Single level selection performed."), *UMBSFunctionLibrary::GetDisplayName(BS));
}

void FMBSSections::SelectSections(const TArray<FModularLevel>& InLevels) const
{
	for (const auto& Level : InLevels)
	{
		SelectSections(Level);
	}
}

void FMBSSections::UnselectSections(const FModularLevel& InLevel) const
{
	UE_LOG(LogMBS, Log, TEXT("%s: Unselecting single level."), *UMBSFunctionLibrary::GetDisplayName(BS));
	if (BS->GetMeshConfiguration().IsOfInstancedType())
	{
		if (InLevel.InstancedStaticMeshComponent)
		{
			InLevel.InstancedStaticMeshComponent->SelectInstance(false, 0,
				InLevel.InstancedStaticMeshComponent->GetInstanceCount());
		}
	}
	else
	{
		for (const auto& Section : GetStaticSectionsOfLevel(InLevel))
		{
			if (Section->GetStaticMeshActor())
			{
				GEditor->SelectActor(Section->GetStaticMeshActor(), false, true, true);
			}
		}
	}
	UE_LOG(LogMBS, Log, TEXT("%s: Single level was unselected."), *UMBSFunctionLibrary::GetDisplayName(BS));
}

void FMBSSections::UnselectSections(const TArray<FModularLevel>& InLevels) const
{
	for (const auto& Level : InLevels)
	{
		UnselectSections(Level);
	}
}

void FMBSSections::SelectAll()
{
	if (GEditor)
	{
		if (BS->GetMeshConfiguration().IsOfInstancedType())
		{
			for (const auto& Section : Instanced)
			{
				if (Section.GetISMC())
				{
					Section.GetISMC()->SelectInstance(true, 0, Section.GetInstanceCount());
				}
			}
		}
		else
		{
			for (const auto& Section : Static)
			{
				if (Section.GetStaticMeshActor())
				{
					UE_LOG(LogMBS, Verbose, TEXT("%s: Trying to select %s section."), *UMBSFunctionLibrary::GetDisplayName(BS), *Section.GetName());
					GEditor->SelectActor(Section.GetStaticMeshActor(), true, true, true);
				}
				else
				{
					UE_LOG(LogMBS, Error, TEXT("%s: Nothing to select."), *UMBSFunctionLibrary::GetDisplayName(BS));
				}
			}
		}
		UE_LOG(LogMBS, Log, TEXT("%s: Selection performed."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: GEditor was nullptr. Selection can't be performed."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}
}

void FMBSSections::UnselectAll()
{
	if (GEditor)
	{
		if (BS->GetMeshConfiguration().IsOfInstancedType())
		{
			for (const auto& Section : Instanced)
			{
				if (Section.GetISMC())
				{
					Section.GetISMC()->SelectInstance(false, 0, Section.GetInstanceCount());
				}
			}
		}
		else
		{
			for (const auto& Section : Static)
			{
				if (Section.GetStaticMeshActor())
				{
					UE_LOG(LogMBS, Verbose, TEXT("%s: Trying to unselect %s section."), *UMBSFunctionLibrary::GetDisplayName(BS), *Section.GetName());
					GEditor->SelectActor(Section.GetStaticMeshActor(), false, true, true);
				}
				else
				{
					UE_LOG(LogMBS, Error, TEXT("%s: Nothing to unselect."), *UMBSFunctionLibrary::GetDisplayName(BS));
				}
			}
		}
		UE_LOG(LogMBS, Log, TEXT("%s: Unselection performed."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s, GEditor was nullptr. Unselection can't be performed."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}
}

bool FMBSSections::IsValidSectionIndex(const FModularLevel& InLevel, int32 Index) const
{
	UE_LOG(LogMBS, Verbose, TEXT("%s: Index=%d"), *UMBSFunctionLibrary::GetDisplayName(BS), Index);
	return GetStaticSectionsOfLevel(InLevel).IsValidIndex(Index);
}

void FMBSSections::OffsetLevel(const FModularLevel& Level, FVector Offset, bool bUpdateZMultiplier) const
{
	if (BS->GetMeshConfiguration().IsOfInstancedType())
	{
		if (const FModularSectionInstanced* InstancedLevelSection = GetInstancedSectionOfLevel(Level))
		{
			InstancedLevelSection->GetISMC()->AddWorldOffset(Offset);
		}
	}
	else
	{
		for (const FModularSection* LevelSection : GetStaticSectionsOfLevel(Level))
		{
			LevelSection->GetStaticMeshActor()->AddActorWorldOffset(Offset);
		}
	}

	if (bUpdateZMultiplier)
	{
		// TODO: add implementation. Maybe move out from this function, as we only can update a single level's z multiplier here,
		// leaving all other levels above with the same Z multiplier as before.
		
	}
}

void FMBSSections::OffsetSection(const FModularLevel& Level, int32 SectionIndex, FVector Offset, bool bRelative) const
{
	if (FModularSection* Section = GetSectionAt(Level, SectionIndex))
	{
		Section->Offset(Offset, bRelative);
	}
}

void FMBSSections::ResetSingleLevel(int32 LevelId)
{
	if (!FModularLevel::IsValidLevelId(LevelId))
	{
		UE_LOG(LogMBSSection, Error, TEXT("%s: LevelId %d is invalid on ResetSingleLevel."), *UMBSFunctionLibrary::GetDisplayName(BS), LevelId);
	}

	for (auto& Section : Static)
	{
		if (!FModularLevel::IsValidLevelId(Section.GetLevelId()))
		{
			UE_LOG(LogMBSSection, Error, TEXT("%s: Section has invalid id bound"), *UMBSFunctionLibrary::GetDisplayName(BS));
		}

		if (Section.GetLevelId() == LevelId)
		{
			Section.Reset();
		}
	}
}

int32 FMBSSections::GetFirstIndexOfSectionWithLevelId(const int32 InLevelId) const
{
	check(FModularLevel::IsValidLevelId(InLevelId));
	for (int32 i = 0; i < Static.Num(); i++)
	{
		if (Static[i].IsInLevel(InLevelId))
		{
			return i;
		}
	}
	return -1;
}

int32 FMBSSections::GetLastIndexOfSectionWithLevelId(const int32 InLevelId) const
{
	check(FModularLevel::IsValidLevelId(InLevelId));
	for (int32 i = Static.Num() - 1; i > 0; i--)
	{
		if (Static[i].IsInLevel(InLevelId))
		{
			return i;
		}
	}
	return -1;
}

void FMBSSections::RemoveSectionsOfLevel(int32 LevelId)
{
	if (FModularLevel::IsValidLevelId(LevelId))
	{
		for (FModularSection& Section : Static)
		{
			if (Section.IsInLevel(LevelId))
			{
				Section.Reset();
			}
		}
		ClearInvalidSections();

		const FModularLevel* CurrentLevel = BS->GetLevelWithId(LevelId);
		check(CurrentLevel);
		
		UE_LOG(LogMBSSection, Log, TEXT("%s: Sections of %s level were removed."), *UMBSFunctionLibrary::GetDisplayName(BS), *CurrentLevel->GetName());
	}
}

void FMBSSections::RemoveSectionsAfterIndex(int32 Index, int32 LevelId)
{
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: RemoveSectionsAfterIndex | Section.Num()=%d"), *UMBSFunctionLibrary::GetDisplayName(BS), Static.Num());
	if (!Static.IsValidIndex(Index))
	{
		return;
	}
	
	// If LevelId is provided - then find the index in Sections array that corresponds to the first section
	// of a level with specified LevelId
	const int32 SectionsCount = Static.Num();
	if (FModularLevel::IsValidLevelId(LevelId))
	{
		// Starting from first index of a section with LevelId + Shift (Index)
		const int32 EndIndex = GetLastIndexOfSectionWithLevelId(LevelId);
		checkf(Static.IsValidIndex(EndIndex), TEXT("%d"), EndIndex);

		const int32 FirstIndex = GetFirstIndexOfSectionWithLevelId(LevelId);
		const int32 StartIndex = FirstIndex + Index;
		UE_LOG(LogMBSSection, Verbose, TEXT("%s: FirstIndex=%d, StartIndex=%d, EndIndex=%d"), *UMBSFunctionLibrary::GetDisplayName(BS), FirstIndex, StartIndex, EndIndex);
		
		checkf(Static.IsValidIndex(StartIndex), TEXT("%d"), StartIndex);
		checkf(StartIndex <= EndIndex, TEXT("%d <= %d"), StartIndex, EndIndex);
		
		for (int32 i = StartIndex; i <= EndIndex; i++)
		{
			if (Static[i].IsInLevel(LevelId))
			{
				Static[i].Reset();
			}
		}
	}
	else
	{
		for (int32 i = Index; i < SectionsCount; i++)
		{
			Static[i].Reset();
		}
	}
	ClearInvalidSections();
}

void FMBSSections::ResetInstancedSectionOfLevel(int32 LevelId)
{
	if (FModularLevel::IsValidLevelId(LevelId))
	{
		if (FModularSectionInstanced* InstancedSection = GetInstancedSectionOfLevel(LevelId))
		{
			TotalInstanceCount -= InstancedSection->GetInstanceCount();
			InstancedSection->Reset();
			if (FModularLevel* AssociatedLevel = BS->GetLevelWithId(LevelId))
			{
				AssociatedLevel->InstancedStaticMeshComponent = nullptr;
			}
			else
			{
				UE_LOG(LogMBSSection, Error, TEXT("%s: Level with Id = %d not found."), *UMBSFunctionLibrary::GetDisplayName(BS), LevelId);
			}
			// TODO: Clear invalid level - Should be in ClearInvalidSections
			Instanced.RemoveAll([&](const FModularSectionInstanced& Section) -> bool
			{
				return !Section.IsValid();
			});
		}
	}
}

void FMBSSections::ClearInstancedSectionOfLevel(int32 LevelId)
{
	if (FModularLevel::IsValidLevelId(LevelId))
	{
		if (const FModularSectionInstanced* InstancedSection = GetInstancedSectionOfLevel(LevelId))
		{
			InstancedSection->GetISMC()->ClearInstances();
		}
	}
}

void FMBSSections::ClearInvalidSections()
{
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: Clearing invalid sections (with StaticMesh == nullptr)"), *UMBSFunctionLibrary::GetDisplayName(BS));
	// Removing sections with NULL static mesh
	const int32 InitialCount = Static.Num();
	Static.RemoveAll([&](const FModularSection& Section) -> bool { return !Section.IsValid(); });

	const int32 NewCount = Static.Num();
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: %d invalid sections removed out of %d. Current sections count is %d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), InitialCount - NewCount, InitialCount, NewCount);

	const int32 InitialActorSectionCount = Actor.Num();
	Actor.RemoveAll([&](const FModularSectionActor& Section) -> bool { return !Section.IsValid(); });

	const int32 NewActorSectionCount = Actor.Num();
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: %d invalid actor sections removed out of %d. Current actor sections count is %d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), InitialActorSectionCount - NewActorSectionCount, InitialActorSectionCount, NewActorSectionCount);

	// TODO: Instanced static mesh components?
	const int32 InitialInstancedSectionCount = Instanced.Num();
	Instanced.RemoveAll([&](const FModularSectionInstanced& Section) -> bool { return !Section.IsValid(); });
	
	const int32 NewInstancedSectionCount = Instanced.Num();
	UE_LOG(LogMBSSection, Verbose, TEXT("%s: %d invalid instanced sections removed out of %d. Current instanced sections count is %d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), InitialInstancedSectionCount - NewInstancedSectionCount, InitialInstancedSectionCount, NewInstancedSectionCount);
}

void FMBSSections::RemoveActorSectionsAfterIndex(int32 Index, int32 LevelId)
{
	unimplemented();
	// TODO: Implement the same way as the RemoveSectionsAfterIndex
	if (Actor.IsValidIndex(Index))
	{
		for (int32 i = Index; i < Actor.Num(); i++)
		{
			Actor[i].Reset();
		}
		ClearInvalidSections();
	}
}

void FMBSSections::RemoveInstancedSectionsAfterIndex(int32 Index, int32 LevelId)
{
	unimplemented();
	// TODO: Implement the same way as the RemoveSectionsAfterIndex
	if (Instanced.IsValidIndex(Index))
	{
		for (int32 i = Index; i < Instanced.Num(); i++)
		{
			Instanced[i].Reset();
		}
		ClearInvalidSections();
	}
}

void FMBSSections::RemoveInstancedSectionInstancesAfterIndex(int32 Index, int32 LevelId)
{
	UE_LOG(LogMBSSection, Warning, TEXT("%s: RemoveInstancedSectionInstancesAfterIndex: Index=%d, LevelId=%d"),
		*UMBSFunctionLibrary::GetDisplayName(BS), Index, LevelId);
	if (FModularLevel::IsValidLevelId(LevelId))
	{
		const FModularSectionInstanced* InstancedSection = GetInstancedSectionOfLevel(LevelId);
		check(InstancedSection);
		check(InstancedSection->IsValid());

		// Get count of instances from Index to InstanceCount of ISMC
		const int32 InstanceCount = InstancedSection->GetInstanceCount();
		const int32 CountToRemove = InstanceCount - Index;
		UE_LOG(LogMBSSection, Warning, TEXT("%s: RemoveInstancedSectionInstancesAfterIndex: InstanceCount=%d, CountToRemove=%d"),
			*UMBSFunctionLibrary::GetDisplayName(BS), InstanceCount, CountToRemove);
		check(CountToRemove > 0);
		for (int32 i = 0; i < CountToRemove; i++)
		{
			InstancedSection->GetISMC()->RemoveInstance(InstanceCount - 1);
		}
	}
}

void FMBSSections::SetVisibility(bool bVisible)
{
	for (const auto& Section : Static)
	{
		Section.GetStaticMeshActor()->GetRootComponent()->SetVisibility(bVisible, true);
	}

	for (const auto& Section : Actor)
	{
		Section.GetActor()->GetRootComponent()->SetVisibility(bVisible, true);
	}

	for (const auto& Section : Instanced)
	{
		Section.GetISMC()->SetVisibility(bVisible, true);
	}
}

void FMBSSections::SetVisibility(const FModularLevel& InLevel, bool bVisible) const
{
	for (const auto& Section : GetStaticSectionsOfLevel(InLevel))
	{
		check(Section);
		Section->GetStaticMeshActor()->GetRootComponent()->SetVisibility(bVisible, true);
	}

	for (const auto& Section : GetActorSectionsOfLevel(InLevel))
	{
		check(Section);
		Section->GetActor()->GetRootComponent()->SetVisibility(bVisible, true);
	}

	if (BS->GetMeshConfiguration().IsOfInstancedType())
	{
		if (const FModularSectionInstanced* InstancedSection = GetInstancedSectionOfLevel(InLevel))
		{
			InstancedSection->GetISMC()->SetVisibility(bVisible, true);
		}
	}
}

void FMBSSections::ReloadMeshes(int32 InLevelId)
{
	UE_LOG(LogMBS, Log, TEXT("%s: Reloading meshes of sections with %d level id"), *UMBSFunctionLibrary::GetDisplayName(BS), InLevelId);
	if (!FModularLevel::IsValidLevelId(InLevelId))
	{
		return;
	}

	const FModularLevel* Level = BS->GetLevelWithId(InLevelId);
	if (!Level)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Level with Id=%d was nullptr on mesh reload."), *UMBSFunctionLibrary::GetDisplayName(BS), InLevelId);
	}
	
	for (const auto& Section : Static)
	{
		if (Section.GetLevelId() != InLevelId)
		{
			continue;
		}

		if (Section.IsValid())
		{
			Section.SetMesh(Level->GetInitializer().GetStaticMesh());
		}
		else
		{
			UE_LOG(LogMBS, Error, TEXT("%s: Error while mesh reload."), *UMBSFunctionLibrary::GetDisplayName(BS));
		}
	}
	UE_LOG(LogMBS, Log, TEXT("%s: Mesh reload completed"), *UMBSFunctionLibrary::GetDisplayName(BS));
}

void FMBSSections::Reset(bool bResetSections, bool bResetActorSections, bool bResetInstancedSections)
{
	// Removing sections with NULL static mesh
	ClearInvalidSections();
	
	if (bResetSections)
	{
		for (auto& Section : Static)
		{
			Section.Reset();
		}

		EmptyStatic();
		UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Sections array is now empty."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}

	if (bResetActorSections)
	{
		for (auto& Section : Actor)
		{
			Section.Reset();
		}

		EmptyActor();
		UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Actor sections array is now empty."), *UMBSFunctionLibrary::GetDisplayName(BS));
	}

	if (bResetInstancedSections)
	{
		for (auto& Section : Instanced)
		{
			Section.Reset();
		}

#if WITH_EDITOR
		TotalInstanceCount = 0;
#endif

		EmptyInstanced();
	}
}
