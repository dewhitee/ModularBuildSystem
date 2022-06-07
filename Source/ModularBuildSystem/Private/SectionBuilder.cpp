// Fill out your copyright notice in the Description page of Project Settings.


#include "SectionBuilder.h"

#include "ModularBuildSystemActor.h"

MBS::FSectionBuilder::FSectionBuilder(AModularBuildSystemActor* InBuildSystem, const FModularLevel* InLevel)
	: BuildSystem(InBuildSystem), LevelPtr(InLevel)
{
}

MBS::FSectionBuilder::~FSectionBuilder()
{
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Mesh(UStaticMesh* InMesh)
{
	MeshPtr = InMesh;
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Mesh(UStaticMesh* InMesh1, UStaticMesh* InMesh2)
{
	MeshPtr = InMesh1;
	OtherMeshPtr = InMesh2;
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Actor(TSubclassOf<AActor> InClass)
{
	Class = InClass;
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Of(const FModularLevel* InLevel)
{
	LevelPtr = InLevel;
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::At(const FTransform& InTransform)
{
	if (!Transforms.IsSet())
	{
		Transforms = TArray<FTransform>();
	}
	Transforms->Add(InTransform);
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::At(const TArray<FTransform>& InTransforms)
{
	if (!Transforms.IsSet())
	{
		Transforms = TArray<FTransform>();
	}
	Transforms->Append(InTransforms);
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::At(const TArray<int32> InIndices)
{
	if (!SectionIndices.IsSet())
	{
		SectionIndices = TArray<int32>();
	}
	SectionIndices->Append(InIndices);
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::AtInstanced(const int32 InIndex, bool bInstanced)
{
	bSearchInstanced = bInstanced;
	return At(InIndex);
}

MBS::FSectionBuilder& MBS::FSectionBuilder::AtInstanced(const TArray<int32> InIndices, bool bInstanced)
{
	bSearchInstanced = bInstanced;
	return At(InIndices);
}

MBS::FSectionBuilder& MBS::FSectionBuilder::MoveBy(const FVector Location)
{
	for (FTransform& Transform : Transforms.GetValue())
	{
		Transform.AddToTranslation(Location);
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::At(const int32 InIndex)
{
	if (!SectionIndices.IsSet())
	{
		SectionIndices = TArray<int32>();
	}
	SectionIndices->Add(InIndex);
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Rotate(const FRotator Rotator)
{
	for (FTransform& Transform : Transforms.GetValue())
	{
		Transform.SetRotation(FQuat(Rotator));
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::RotatePitch(float DeltaPitch)
{
	for (auto& Transform : Transforms.GetValue())
	{
		FRotator CurrentRotator = FRotator(Transform.GetRotation());
		CurrentRotator.Pitch += DeltaPitch;
		Transform.SetRotation(CurrentRotator.Quaternion());
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::RotateYaw(float DeltaYaw)
{
	for (auto& Transform : Transforms.GetValue())
	{
		FRotator CurrentRotator = FRotator(Transform.GetRotation());
		CurrentRotator.Yaw += DeltaYaw;
		Transform.SetRotation(CurrentRotator.Quaternion());
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::RotateRoll(float DeltaRoll)
{
	for (auto& Transform : Transforms.GetValue())
	{
		FRotator CurrentRotator = FRotator(Transform.GetRotation());
		CurrentRotator.Roll += DeltaRoll;
		Transform.SetRotation(CurrentRotator.Quaternion());
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Spawn(bool bRelative)
{
	if (!BuildSystem.Get() || !LevelPtr)
	{
		return *this;
	}
	
	if (GetMesh())
	{
		if (Transforms.IsSet())
		{
			if (BuildSystem->IsOfInstancedMeshConfigurationType())
			{
				for (auto& Transform : Transforms.GetValue())
				{
					BuildSystem->AddNewInstance(Transform, bRelative, LevelPtr->InstancedStaticMeshComponent);
					SpawnedTransforms.Add(Transform);
				}
			}
			else
			{
				for (auto& Transform : Transforms.GetValue())
				{
					BuildSystem->InitModularSection(GetMesh(), Transform, LevelPtr->GetId(), true, bRelative);
					SpawnedTransforms.Add(Transform);
				}
			}
		}
		else if (SectionIndices.IsSet())
		{
			if (BuildSystem->IsOfInstancedMeshConfigurationType())
			{
				for (const int32 Index : SectionIndices.GetValue())
				{
					const FTransform Transform = BuildSystem->GetSectionTransformAt(LevelPtr->GetId(), Index, true, !bRelative);
					BuildSystem->AddNewInstance(Transform, bRelative, LevelPtr->InstancedStaticMeshComponent);
				}
			}
			else
			{
				for (const int32 Index : SectionIndices.GetValue())
				{
					const FTransform Transform = BuildSystem->GetSectionTransformAt(LevelPtr->GetId(), Index, bSearchInstanced, !bRelative);
					BuildSystem->InitModularSection(GetMesh(), Transform, LevelPtr->GetId(), true, bRelative);
					SpawnedTransforms.Add(Transform);
				}
			}
		}
	}
	else if (Class.IsSet())
	{
		if (Transforms.IsSet())
		{
			for (auto& Transform : Transforms.GetValue())
			{
				BuildSystem->InitModularSectionActor(Transform, LevelPtr->GetId(), Class.GetValue(), true, bRelative);
				SpawnedTransforms.Add(Transform);
			}
		}
		else if (SectionIndices.IsSet())
		{
			for (const int32 Index : SectionIndices.GetValue())
			{
				const FTransform Transform = BuildSystem->GetSectionTransformAt(LevelPtr->GetId(), Index, bSearchInstanced, !bRelative);
				BuildSystem->InitModularSectionActor(Transform, LevelPtr->GetId(), Class.GetValue(), true, bRelative);
				SpawnedTransforms.Add(Transform);
			}
		}
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Replace()
{
	if (BuildSystem.Get() && (MeshPtr.Get() && LevelPtr))
	{
		for (const int32 Index : SectionIndices.GetValue())
		{
			FTransform OutTransform;
			BuildSystem->ReplaceWithNonInstancedSection(MeshPtr.Get(), Index, LevelPtr->GetId(), OutTransform);
			SpawnedTransforms.Add(OutTransform);
		}
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::Set()
{
	if (BuildSystem.Get() && (MeshPtr.Get() && LevelPtr))
	{
		for (const int32 Index : SectionIndices.GetValue())
		{
			BuildSystem->SetMeshAt(*LevelPtr, Index, MeshPtr.Get());
			SpawnedTransforms.Add(BuildSystem->GetSectionTransformAt(LevelPtr->GetId(), Index, false, false));
		}
	}
	return *this;
}

MBS::FSectionBuilder& MBS::FSectionBuilder::SpawnOrReplace(bool bRelative, bool bCondition)
{
	return bCondition ? Spawn(bRelative) : Replace();
}

MBS::FSectionBuilder& MBS::FSectionBuilder::SpawnOrSet(bool bRelative, bool bCondition)
{
	return bCondition ? Spawn(bRelative) : Set();
}

MBS::FSectionBuilder& MBS::FSectionBuilder::ReplaceOrSet(bool bCondition)
{
	return bCondition ? Replace() : Set();
}

MBS::FSectionBuilder& MBS::FSectionBuilder::ReplaceIfInstancedOrSet()
{
	return BuildSystem->IsOfInstancedMeshConfigurationType() ? Replace() : Set();
}

UStaticMesh* MBS::FSectionBuilder::GetMesh() const
{
	return MeshPtr.Get() ? MeshPtr.Get() : OtherMeshPtr.Get();
}

FTransform MBS::FSectionBuilder::GetTransform() const
{
	return !SpawnedTransforms.IsEmpty() ? SpawnedTransforms[0] : FTransform::Identity;
}

TArray<FTransform> MBS::FSectionBuilder::GetTransforms() const
{
	return SpawnedTransforms;
}
