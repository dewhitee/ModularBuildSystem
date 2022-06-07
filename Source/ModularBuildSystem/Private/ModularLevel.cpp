// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularLevel.h"

#include "MBSFunctionLibrary.h"
#include "ModularBuildSystemActor.h"
#include "ModularSectionResolution.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Solver/MBSTransformSolver.h"
#include "ModularBuildSystem.h"

bool FModularLevel::OverrideLevelId(int32 NewId)
{
	UE_LOG(LogModularLevel, Verbose, TEXT("%s: Overriding LevelId..."), *Name.ToString());
	check(NewId != InvalidLevelId);

	if (Id == InvalidLevelId)
	{
		Id = FMath::Clamp(NewId, 0, INT_MAX);
		UE_LOG(LogModularLevel, Verbose, TEXT("LevelId = %d"), Id);
		return true;
	}
	UE_LOG(LogModularLevel, Verbose, TEXT("%s: LevelId is already valid (%d != %d)"), *Name.ToString(), Id, InvalidLevelId);
	return false;
}

void FModularLevel::OverrideLevelIndex(uint8 NewLevelIndex, uint8 MinLevel, uint8 MaxLevel)
{
	LevelIndex = FMath::Clamp(NewLevelIndex, MinLevel, MaxLevel);
}

void FModularLevel::OverrideLevelZMultiplier(float NewZMultiplier, float Min, float Max)
{
	ZMultiplier = FMath::Clamp(NewZMultiplier, Min, Max);
	UE_LOG(LogModularLevel, VeryVerbose, TEXT("%s: OverrideLevelZMultiplier. ZMultiplier=%.2f"), *Name.ToString(), ZMultiplier);
}

void FModularLevel::MarkUpdated(bool bCondition)
{
	bUpdated |= bCondition;
}

float FModularLevel::GetZMultiplierForNextLevel(float PreviousMultiplier) const
{
	return IsValid() && bAffectsNextZMultiplier ? PreviousMultiplier + Initializer.GetZMultiplier() : PreviousMultiplier;
}

bool FModularLevel::Init(const FName& InFallbackDebugName, AModularBuildSystemActor* InSystem, int32 InNewId, uint8 InNewLevelIndex,
	uint8& InShiftCount, float InNewZMultiplier, UModularSectionResolution* InPreviousLevelResolution,
	TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction,
	void (AModularBuildSystemActor::* InAppendMethod)(const FInitModularSectionsArgs&))
{
	if (IsValid())
	{
		UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): ======= %s - run initialization ======="),
			*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString());
		
		if (InSystem->GetReloadMode() == EModularSectionReloadMode::CurrentLevelOnly && !InSystem->WasReset())
		{
			if (InSystem->LevelObserver.GetModifiedLevelId() != Id)
			{
				UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s - level has not been modified (%d != %d). Only current modified level should be re-init."),
					*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString(), InSystem->LevelObserver.GetModifiedLevelId(), InNewId);
				return false;
			}
		}

		if (OverrideLevelId(InNewId))
		{
			bUpdated = true;
			InSystem->LevelInitializer.IncrementLastLevelId();
		}

		//bUpdated |= InNewLevelIndex != LevelIndex || InNewZMultiplier != ZMultiplier;
		MarkUpdated(InNewLevelIndex != LevelIndex || InNewZMultiplier != ZMultiplier);
		UE_LOG(LogModularLevel, Verbose, TEXT("%s: bUpdated=%s (InNewLevelIndex=%d != LevelIndex=%d || InNewZMultiplier=%.2f != ZMultiplier=%.2f)"),
			*GetName(), bUpdated ? TEXT("true") : TEXT("false"), InNewLevelIndex, LevelIndex, InNewZMultiplier, ZMultiplier);
		
		OverrideLevelIndex(InNewLevelIndex);
		OverrideLevelZMultiplier(InNewZMultiplier);

		// Append sections or instanced sections using provided lambda function
		const FInitModularSectionsArgs& AppendArgs = FInitModularSectionsArgs(
			Initializer, Id, ZMultiplier, InPreviousLevelResolution, CustomShape, InstancedStaticMeshComponent,
			PivotLocation, Solver);
		
		InitAppend(InAppendFunction, InAppendMethod, InSystem, AppendArgs);
		bUpdated = false;
		return true;
	}
	else if (MustBeSkippedWithShift())
	{
		UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s initializer is not valid and must be skipped with shift."),
			*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString());
		InShiftCount++;
	}
	else
	{
		UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s initializer is not valid."),
			*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString());
		if (!Initializer.GetResolution())
		{
			UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s - Initializer.Resolution is nullptr."),
				*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString());
		}
		if (Initializer.GetTotalCount() <= 0)
		{
			UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s - Initializer.TotalCount (%d) <= 0."),
				*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString(), Initializer.GetTotalCount());
		}
		if (!Initializer.GetStaticMesh())
		{
			UE_LOG(LogModularLevel, Error, TEXT("%s (%s): %s - Initializer.StaticMesh is nullptr."),
				*UMBSFunctionLibrary::GetDisplayName(InSystem), *Name.ToString(), *InFallbackDebugName.ToString());
		}
	}

	return false;
}

bool FModularLevel::IsValid() const
{
	return Initializer.GetResolution() != nullptr 
		&& ((Initializer.GetTotalCount() > 0 && !bSkipThisLevel) || (bSkipThisLevel && SkipMode == EModularSectionSkipMode::Hide)) 
		&& Initializer.GetStaticMesh();
}

bool FModularLevel::MustBeSkippedWithShift() const
{
	return bSkipThisLevel && SkipMode == EModularSectionSkipMode::HideAndShift;
}

FTransform FModularLevel::GetWorldTransform(const IModularBuildSystemInterface* InBuildSystem) const
{
	check(InBuildSystem);
	return GetWorldTransform(InBuildSystem->GetBuildSystemTransform());
}

FTransform FModularLevel::GetWorldTransform(FTransform InBuildSystemTransform) const
{
	InBuildSystemTransform.AddToTranslation(
		FVector(0.f, 0.f, UModularSectionResolution::DefaultSectionSize * ZMultiplier));
	return InBuildSystemTransform;
}

FTransform FModularLevel::GetRelativeTransform(const IModularBuildSystemInterface* InBuildSystem) const
{
	check(InBuildSystem);
	return GetRelativeTransform(InBuildSystem->GetBuildSystemTransform());
}

FTransform FModularLevel::GetRelativeTransform(const FTransform& InBuildSystemTransform) const
{
	return GetWorldTransform(InBuildSystemTransform).GetRelativeTransform(InBuildSystemTransform);
}

FIntVector FModularLevel::GetResolutionVec() const
{
	if (Initializer.GetResolution())
	{
		return Initializer.GetResolution()->GetValue();
	}
	UE_LOG(LogModularLevel, Error, TEXT("%s: Can't get resolution vector because Initializer.Resolution was nullptr."),
		*Name.ToString());
	return {};
}

FString FModularLevel::GetName() const
{
#if WITH_EDITOR
	return Name.ToString();
#else
	return FString("None");
#endif
}

bool FModularLevel::IsUpdated() const
{
	return bUpdated;
}

void FModularLevel::Invalidate()
{
	const FName PreviousName = Name;
	*this = FModularLevel();
	Name = PreviousName;
}

FModularLevel& FModularLevel::SetMeshList(UModularBuildSystemMeshList* InMeshList)
{
	MarkUpdated(Initializer.GetMeshList() != InMeshList);
	Initializer.SetMeshList(InMeshList);
	return *this;
}

FModularLevel& FModularLevel::SetMeshListIndex(int32 InIndex)
{
	MarkUpdated(Initializer.GetMeshListIndex() != InIndex);
	Initializer.SetMeshListIndex(InIndex);
	return *this;
}

FModularLevel& FModularLevel::SetResolution(UModularSectionResolution* InResolution)
{
	MarkUpdated(Initializer.GetResolution() != InResolution);
	Initializer.SetResolution(InResolution);
	return *this;
}

FModularLevel& FModularLevel::SetTotalCount(int32 InTotalCount)
{
	MarkUpdated(Initializer.GetTotalCount() != InTotalCount);
	Initializer.SetTotalCount(InTotalCount);
	return *this;
}

FModularLevel& FModularLevel::SetMaxInRow(int32 InMaxInRow)
{
	MarkUpdated(Initializer.GetMaxInRow() != InMaxInRow);
	Initializer.SetMaxInRow(InMaxInRow);
	return *this;
}

FModularLevel& FModularLevel::SetFromMeshList()
{
	Initializer.SetFromMeshList();
	return *this;
}

FModularLevel& FModularLevel::SetShape(UModularLevelShape* InShape)
{
	MarkUpdated(CustomShape != InShape);
	CustomShape = InShape;
	return *this;
}

FModularLevel& FModularLevel::SetInstancedStaticMeshComponent(UStaticMesh* InStaticMesh,
	TScriptInterface<IModularBuildSystemInterface> InBuildSystem, bool bUseExistingMesh)
{
	check(InBuildSystem);

	if (bUseExistingMesh)
	{
		InStaticMesh = Initializer.GetStaticMesh();
	}

	if (!InStaticMesh)
	{
		UE_LOG(LogModularLevel, Error, TEXT("%s (%s) Error on SetInstancedStaticMeshComponent: InStaticMesh was nullptr"),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString());
		return *this;
	}
	
	if (InstancedStaticMeshComponent)
	{
		MarkUpdated(InstancedStaticMeshComponent->GetStaticMesh() != InStaticMesh);
		UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): Instanced static mesh component is already initialized. Setting static mesh."),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString());
		UE_LOG(LogModularLevel, VeryVerbose, TEXT("%s (%s): Root component name = %s"), *InBuildSystem->GetRoot()->GetName(), *Name.ToString());
		InstancedStaticMeshComponent->SetStaticMesh(InStaticMesh);
	}
	else
	{
		bUpdated = true;
		switch (InBuildSystem->GetMeshConfiguration().Type)
		{
		case EMBSMeshConfigurationType::Default:
		case EMBSMeshConfigurationType::StaticMeshes:
			UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s) Initialization of InstancedStaticMeshComponent was skipped because of MeshConfigurationType."),
				*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString());
			break;
		case EMBSMeshConfigurationType::InstancedStaticMeshes:
			CreateNewInstancedStaticMeshComponent(UInstancedStaticMeshComponent::StaticClass(), InBuildSystem, InStaticMesh);
			break;
		case EMBSMeshConfigurationType::HierarchicalInstancedStaticMeshes:
			CreateNewInstancedStaticMeshComponent(UHierarchicalInstancedStaticMeshComponent::StaticClass(), InBuildSystem, InStaticMesh);
			break;
		}
	}

	return *this;
}

FModularLevel& FModularLevel::SetPivotLocation(EModularSectionPivotLocation NewPivotLocation)
{
	const FString& PivotLocationName = UEnum::GetValueAsString(PivotLocation);
	FString NewPivotLocationName = UEnum::GetValueAsString(NewPivotLocation);
	
	if (FName(NewPivotLocationName).IsNone())
	{
		constexpr EModularSectionPivotLocation BackupPivotLocationValue = EModularSectionPivotLocation::Default;
		UE_LOG(LogModularLevel, Error, TEXT("%s: NewPivotLocationName = %s. Set NewPivotLocationName to %s"),
			*Name.ToString(), *NewPivotLocationName, *UEnum::GetValueAsString(BackupPivotLocationValue));
		
		NewPivotLocation = BackupPivotLocationValue;
		NewPivotLocationName = UEnum::GetValueAsString(NewPivotLocation);
	}
	UE_LOG(LogModularLevel, Verbose, TEXT("%s: Pivot location of a level was changed: %s ---> %s"),
		*Name.ToString(), *PivotLocationName, *NewPivotLocationName);

	MarkUpdated(PivotLocation != NewPivotLocation);
	PivotLocation = NewPivotLocation;
	return *this;
}

FModularLevel& FModularLevel::SetName(const FName NewName)
{
#if WITH_EDITOR
	Name = NewName;
#endif
	return *this;
}

FModularLevel& FModularLevel::SetAffectsNextZMultiplier(bool bAffects)
{
	MarkUpdated(bAffectsNextZMultiplier != bAffects);
	bAffectsNextZMultiplier = bAffects;
	return *this;
}

FModularLevel& FModularLevel::SetSolver(UMBSTransformSolver* InSolver)
{
	MarkUpdated(Solver != InSolver);
	Solver = InSolver;
	return *this;
}

FModularLevel& FModularLevel::SetUpdated(bool bNewUpdated)
{
	bUpdated = bNewUpdated;
	return *this;
}

void FModularLevel::CreateNewInstancedStaticMeshComponent(TSubclassOf<UInstancedStaticMeshComponent> ComponentClass,
	TScriptInterface<IModularBuildSystemInterface> InBuildSystem, UStaticMesh* InStaticMesh)
{
	check(InBuildSystem->GetRoot());
	if (InBuildSystem->GetRoot())
	{
		UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): Root component is valid! Current root component = %s"),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString(), *InBuildSystem->GetRoot()->GetName());

		const FName ComponentName = MakeUniqueObjectName(InBuildSystem.GetObject(), ComponentClass);
		InstancedStaticMeshComponent = NewObject<UInstancedStaticMeshComponent>(InBuildSystem.GetObject(), ComponentClass, ComponentName);
		InstancedStaticMeshComponent->SetMobility(EComponentMobility::Static);
		InstancedStaticMeshComponent->SetupAttachment(InBuildSystem->GetRoot());
		InstancedStaticMeshComponent->SetStaticMesh(InStaticMesh);
		InstancedStaticMeshComponent->RegisterComponent();

		UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): Created new instanced static mesh component named %s"),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString(), *ComponentName.ToString());
	}
	else
	{
		UE_LOG(LogModularLevel, Error, TEXT("%s (%s): Root component is not valid!"),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString());
	}
	UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): Build system transform (After component creation): %s"),
		*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Name.ToString(), *InBuildSystem->GetBuildSystemTransform().ToHumanReadableString());
}

void FModularLevel::InitAppend(TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction,
	void (AModularBuildSystemActor::* InAppendMethod)(const FInitModularSectionsArgs&),
	AModularBuildSystemActor* InSystem, const FInitModularSectionsArgs& InArgs)
{
	if (InAppendFunction)
	{
		InAppendFunction(InArgs);
	}
	else if (InAppendMethod)
	{
		(InSystem->*InAppendMethod)(InArgs);
	}
}
