// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularSection.h"
#include "ModularBuildSystem.h"
#include "List/ModularBuildSystemMeshList.h"
#include "ModularLevel.h"
#include "ModularSectionResolution.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"

UStaticMesh* FModularSectionInitializer::GetStaticMesh() const
{
	return StaticMesh;
}

float FModularSectionInitializer::GetZMultiplier() const
{
	return Resolution ? Resolution->GetHeightAsMultiplier() : -1.f;
}

int32 FModularSectionInitializer::GetAdjustedTotalCount() const
{
	return TotalCount - SkippedCount;
}

FModularBuildStats FModularSectionInitializer::GetBuildStats() const
{
	const int32 MaxTotalRows = TotalCount > 0 && MaxInRow > 0 ? TotalCount / MaxInRow : 0;
	return FModularBuildStats(
		FIntPoint(MaxInRow, MaxTotalRows),
		TotalCount,
		MaxTotalRows,
		MaxInRow);
}

void FModularSectionInitializer::SetFromMeshList()
{
	UE_LOG(LogModularSection, Verbose, TEXT("(Initializer) Setting mesh from list."));
	if (MeshList)
	{
		StaticMesh = MeshList->GetMesh(MeshListIndex, Resolution);
	}
	else if (TotalCount > 0)
	{
		UE_LOG(LogModularSection, Error, TEXT("(Initializer) MeshList was nullptr when trying to set static mesh from mesh list."));
	}
}

void FModularSectionInitializer::SetMeshList(UModularBuildSystemMeshList* InMeshList)
{
	MeshList = InMeshList;
}

void FModularSectionInitializer::SetMeshListIndex(int32 InIndex)
{
	MeshListIndex = InIndex;
}

void FModularSectionInitializer::SetResolution(UModularSectionResolution* InResolution)
{
	Resolution = InResolution;
}

void FModularSectionInitializer::SetTotalCount(int32 InTotalCount)
{
	TotalCount = InTotalCount;
}

void FModularSectionInitializer::SetMaxInRow(int32 InMaxInRow)
{
	MaxInRow = InMaxInRow;
}

void FModularSectionInitializer::SetSkippedCount(int32 InSkippedCount)
{
	SkippedCount = InSkippedCount;
}

bool FModularSectionBase::IsInLevel(const int32 InLevelId) const
{
	return InLevelId == LevelId;
}

bool FModularSectionBase::IsInLevel(const FModularLevel& InLevel) const
{
	return LevelId == InLevel.GetId();
}

bool FModularSectionBase::IsInLevel(const TArray<FModularLevel>& InLevels) const
{
	for (const auto& Level : InLevels)
	{
		if (IsInLevel(Level))
		{
			return true;
		}
	}
	return false;
}

void FModularSectionBase::SetActorLocation(AActor* OfActor, FVector NewLocation, bool bRelative)
{
	check(OfActor);
	bRelative
		? OfActor->SetActorRelativeLocation(NewLocation)
		: OfActor->SetActorLocation(NewLocation);
}

void FModularSectionBase::SetActorTransform(AActor* OfActor, const FTransform& NewTransform, bool bRelative)
{
	check(OfActor);
	bRelative
		? OfActor->SetActorRelativeTransform(NewTransform)
		: OfActor->SetActorTransform(NewTransform);
}

void FModularSectionBase::OffsetActor(AActor* OfActor, FVector Offset, bool bRelative)
{
	check(OfActor);
	bRelative
		? OfActor->AddActorLocalOffset(Offset)
		: OfActor->AddActorWorldOffset(Offset);
}

void FModularSectionBase::ScaleActor(AActor* OfActor, FVector ScaleOffset, bool bRelative)
{
	check(OfActor);
	bRelative
		? OfActor->SetActorRelativeScale3D(1.f * ScaleOffset)
		: OfActor->SetActorScale3D(1.f * ScaleOffset);
}

void FModularSection::Offset(FVector Offset, bool bRelative, int32 Index)
{
	OffsetActor(StaticMesh, Offset, bRelative);
}

void FModularSection::SetLocation(FVector NewLocation, bool bRelative, int32 Index)
{
	SetActorLocation(StaticMesh, NewLocation, bRelative);
}

FTransform FModularSection::GetTransform(int32 Index) const
{
	check(StaticMesh);
	return StaticMesh->GetActorTransform();
}

void FModularSection::SetTransform(const FTransform& NewTransform, bool bRelative, int32 Index)
{
	SetActorTransform(StaticMesh, NewTransform, bRelative);
}

bool FModularSection::IsValid() const
{
	return GetLevelId() != FModularLevel::InvalidLevelId && StaticMesh != nullptr;
}

FVector FModularSection::GetLocation(int32 Index) const
{
	check(StaticMesh);
	return StaticMesh->GetActorLocation();
}

void FModularSection::Scale(FVector ScaleOffset, bool bRelative, int32 Index)
{
	ScaleActor(StaticMesh, ScaleOffset, bRelative);
}

void FModularSection::SetPivot(FVector PivotOffset, int32 Index)
{
	check(StaticMesh);
	//UE_LOG(LogMBS, Log, TEXT("Pivot offset was set to %s"), *PivotOffset.ToCompactString());
	StaticMesh->SetPivotOffset(PivotOffset);
}

FString FModularSection::GetName() const
{
	check(StaticMesh);
	return StaticMesh->GetName();
}

FVector FModularSection::GetForwardVector(int32 Index) const
{
	return StaticMesh->GetActorForwardVector();
}

FVector FModularSection::GetRightVector(int32 Index) const
{
	return StaticMesh->GetActorRightVector();
}

FVector FModularSection::GetUpVector(int32 Index) const
{
	return StaticMesh->GetActorUpVector();
}

void FModularSection::Reset()
{
	check(StaticMesh);
	StaticMesh->Destroy();
	StaticMesh = nullptr;
}

void FModularSection::SetMesh(UStaticMesh* NewStaticMesh) const
{
	check(StaticMesh);
	StaticMesh->GetStaticMeshComponent()->SetStaticMesh(NewStaticMesh);
}

FVector FModularSectionActor::GetLocation(int32 Index) const
{
	check(Actor);
	return Actor->GetActorLocation();
}

void FModularSectionActor::SetLocation(FVector NewLocation, bool bRelative, int32 Index)
{
	SetActorLocation(Actor, NewLocation, bRelative);
}

FTransform FModularSectionActor::GetTransform(int32 Index) const
{
	check(Actor);
	return Actor->GetActorTransform();
}

void FModularSectionActor::SetTransform(const FTransform& NewTransform, bool bRelative, int32 Index)
{
	SetActorTransform(Actor, NewTransform, bRelative);
}

void FModularSectionActor::Offset(FVector Offset, bool bRelative, int32 Index)
{
	OffsetActor(Actor, Offset, bRelative);
}

void FModularSectionActor::Scale(FVector ScaleOffset, bool bRelative, int32 Index)
{
	ScaleActor(Actor, ScaleOffset, bRelative);
}

void FModularSectionActor::SetPivot(FVector PivotOffset, int32 Index)
{
	check(Actor);
	Actor->SetPivotOffset(PivotOffset);
}

FString FModularSectionActor::GetName() const
{
	check(Actor);
	return Actor->GetName();
}

bool FModularSectionActor::IsValid() const
{
	return GetLevelId() != FModularLevel::InvalidLevelId && Actor != nullptr;
}

FVector FModularSectionActor::GetForwardVector(int32 Index) const
{
	check(Actor);
	return Actor->GetActorForwardVector();
}

FVector FModularSectionActor::GetRightVector(int32 Index) const
{
	check(Actor);
	return Actor->GetActorRightVector();
}

FVector FModularSectionActor::GetUpVector(int32 Index) const
{
	check(Actor);
	return Actor->GetActorUpVector();
}

void FModularSectionActor::Reset()
{
	check(Actor);
	Actor->Destroy();
	Actor = nullptr;
}

void FModularSectionActor::SetMesh(UStaticMesh* NewStaticMesh) const
{
	unimplemented();
}

FModularSectionInstanced::FModularSectionInstanced(int32 InLevelId,
	UInstancedStaticMeshComponent* InInstancedStaticMeshComponent, int32 InInstanceCount)
		: Super(InLevelId)
		, InstancedStaticMeshComponent(InInstancedStaticMeshComponent)
		, InstanceCount(InInstanceCount)
{
	InstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InstancedStaticMeshComponent->SetGenerateOverlapEvents(false);
}

FTransform FModularSectionInstanced::GetSectionTransform(int32 AtIndex) const
{
	if (InstancedStaticMeshComponent)
	{
		FTransform OutTransform;
		InstancedStaticMeshComponent->GetInstanceTransform(AtIndex, OutTransform);
		return OutTransform;
	}
	UE_LOG(LogModularSection, Error, TEXT("%s: InstancedStaticMeshComponent was nullptr on GetSectionTransform"), *GetName());
	return {};
}

FString FModularSectionInstanced::GetSectionName(int32 AtIndex) const
{
	if (InstancedStaticMeshComponent)
	{
		if (AtIndex == -1)
		{
			return InstancedStaticMeshComponent->GetName();
		}
		return FString::Printf(TEXT("%s_%d"), *InstancedStaticMeshComponent->GetName(), AtIndex);
	}
	UE_LOG(LogModularSection, Error, TEXT("%s: InstancedStaticMeshComponent was nullptr on GetSectionName"), *GetName());
	return {};
}

bool FModularSectionInstanced::IsValid() const
{
	return GetLevelId() != FModularLevel::InvalidLevelId && InstancedStaticMeshComponent != nullptr;
}

FVector FModularSectionInstanced::GetLocation(int32 Index) const
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->GetInstanceTransform(Index, InstanceTransform);
	return InstanceTransform.GetLocation();
}

void FModularSectionInstanced::SetLocation(FVector NewLocation, bool bRelative, int32 Index)
{
	check(InstancedStaticMeshComponent);
	FTransform NewInstanceTransform = GetTransform(Index);
	NewInstanceTransform.SetLocation(NewLocation);
	InstancedStaticMeshComponent->UpdateInstanceTransform(Index, NewInstanceTransform, bRelative, Index == GetInstanceCount() - 1);
}

FTransform FModularSectionInstanced::GetTransform(int32 Index) const
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->GetInstanceTransform(Index, InstanceTransform);
	return InstanceTransform;
}

void FModularSectionInstanced::SetTransform(const FTransform& NewTransform, bool bRelative, int32 Index)
{
	check(InstancedStaticMeshComponent);
	InstancedStaticMeshComponent->UpdateInstanceTransform(Index, NewTransform, bRelative, Index == GetInstanceCount() - 1);
}

void FModularSectionInstanced::Offset(FVector Offset, bool bRelative, int32 Index)
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform = GetTransform(Index);
	InstanceTransform.AddToTranslation(Offset);
	InstancedStaticMeshComponent->UpdateInstanceTransform(Index, InstanceTransform, bRelative, Index == GetInstanceCount() - 1);
}

void FModularSectionInstanced::Scale(FVector ScaleOffset, bool bRelative, int32 Index)
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform = GetTransform(Index);
	InstanceTransform.SetScale3D(ScaleOffset);
	InstancedStaticMeshComponent->UpdateInstanceTransform(Index, InstanceTransform, bRelative, Index == GetInstanceCount() - 1);
}

void FModularSectionInstanced::SetPivot(FVector PivotOffset, int32 Index)
{
	// Set pivot is not supported for instanced modular sections
	UE_LOG(LogModularSection, Verbose, TEXT("%s: SetPivot is not supported for instanced modular sections."), *GetName());
}

FString FModularSectionInstanced::GetName() const
{
	return GetSectionName();
}

FVector FModularSectionInstanced::GetForwardVector(int32 Index) const
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->GetInstanceTransform(Index, InstanceTransform);
	return InstanceTransform.Rotator().Vector();
}

FVector FModularSectionInstanced::GetRightVector(int32 Index) const
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->GetInstanceTransform(Index, InstanceTransform);
	return FRotationMatrix(InstanceTransform.Rotator()).GetScaledAxis(EAxis::Y);
}

FVector FModularSectionInstanced::GetUpVector(int32 Index) const
{
	check(InstancedStaticMeshComponent);
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->GetInstanceTransform(Index, InstanceTransform);
	return FRotationMatrix(InstanceTransform.Rotator()).GetScaledAxis(EAxis::Z);
}

void FModularSectionInstanced::Reset()
{
	check(InstancedStaticMeshComponent);
	InstancedStaticMeshComponent->ClearInstances();
	InstancedStaticMeshComponent->UnregisterComponent();
	InstancedStaticMeshComponent->DestroyComponent();
	InstancedStaticMeshComponent = nullptr;
}

void FModularSectionInstanced::SetMesh(UStaticMesh* NewStaticMesh) const
{
	check(InstancedStaticMeshComponent);
	InstancedStaticMeshComponent->SetStaticMesh(NewStaticMesh);
}

int32 FModularSectionInstanced::GetInstanceCount() const
{
	check(InstancedStaticMeshComponent);
	return InstancedStaticMeshComponent->GetInstanceCount();
}
