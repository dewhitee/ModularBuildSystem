// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularLevelBuilder.h"

#include "MBSFunctionLibrary.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemInterface.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Config/MBSMeshConfiguration.h"

MBS::FModularLevelBuilder::FModularLevelBuilder()
{
}

MBS::FModularLevelBuilder::FModularLevelBuilder(const FModularLevel& InLevel)
	: Level(InLevel)
{
}

MBS::FModularLevelBuilder::~FModularLevelBuilder()
{
}

FModularLevel& MBS::FModularLevelBuilder::SetMeshList(UModularBuildSystemMeshList* InMeshList)
{
	Level.MarkUpdated(Level.Initializer.GetMeshList() != InMeshList);
	Level.Initializer.SetMeshList(InMeshList);
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetMeshListIndex(int32 InIndex)
{
	Level.MarkUpdated(Level.Initializer.GetMeshListIndex() != InIndex);
	Level.Initializer.SetMeshListIndex(InIndex);
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetResolution(UModularSectionResolution* InResolution)
{
	Level.MarkUpdated(Level.Initializer.GetResolution() != InResolution);
	Level.Initializer.SetResolution(InResolution);
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetTotalCount(int32 InTotalCount)
{
	Level.MarkUpdated(Level.Initializer.GetTotalCount() != InTotalCount);
	Level.Initializer.SetTotalCount(InTotalCount);
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetMaxInRow(int32 InMaxInRow)
{
	Level.MarkUpdated(Level.Initializer.GetMaxInRow() != InMaxInRow);
	Level.Initializer.SetMaxInRow(InMaxInRow);
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetFromMeshList()
{
	Level.Initializer.SetFromMeshList();
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetShape(UModularLevelShape* InShape)
{
	Level.MarkUpdated(Level.CustomShape != InShape);
	Level.CustomShape = InShape;
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetInstancedStaticMeshComponent(UStaticMesh* InStaticMesh,
	TScriptInterface<IModularBuildSystemInterface> InBuildSystem, bool bUseExistingMesh)
{
	check(InBuildSystem);

	if (bUseExistingMesh)
	{
		InStaticMesh = Level.Initializer.GetStaticMesh();
	}

	if (!InStaticMesh)
	{
		UE_LOG(LogModularLevel, Error, TEXT("%s (%s) Error on SetInstancedStaticMeshComponent: InStaticMesh was nullptr"),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Level.Name.ToString());
		return Level;
	}
	
	if (Level.InstancedStaticMeshComponent)
	{
		Level.MarkUpdated(Level.InstancedStaticMeshComponent->GetStaticMesh() != InStaticMesh);
		UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s): Instanced static mesh component is already initialized. Setting static mesh."),
			*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Level.Name.ToString());
		UE_LOG(LogModularLevel, VeryVerbose, TEXT("%s (%s): Root component name = %s"), *InBuildSystem->GetRoot()->GetName(), *Level.Name.ToString());
		Level.InstancedStaticMeshComponent->SetStaticMesh(InStaticMesh);
	}
	else
	{
		Level.bUpdated = true;
		switch (InBuildSystem->GetMeshConfiguration().Type)
		{
		case EMBSMeshConfigurationType::Default:
		case EMBSMeshConfigurationType::StaticMeshes:
			UE_LOG(LogModularLevel, Verbose, TEXT("%s (%s) Initialization of InstancedStaticMeshComponent was skipped because of MeshConfigurationType."),
				*UMBSFunctionLibrary::GetDisplayName(InBuildSystem), *Level.Name.ToString());
			break;
		case EMBSMeshConfigurationType::InstancedStaticMeshes:
			Level.CreateNewInstancedStaticMeshComponent(UInstancedStaticMeshComponent::StaticClass(), InBuildSystem, InStaticMesh);
			break;
		case EMBSMeshConfigurationType::HierarchicalInstancedStaticMeshes:
			Level.CreateNewInstancedStaticMeshComponent(UHierarchicalInstancedStaticMeshComponent::StaticClass(), InBuildSystem, InStaticMesh);
			break;
		}
	}

	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetPivotLocation(EModularSectionPivotLocation NewPivotLocation)
{
	const FString& PivotLocationName = UEnum::GetValueAsString(Level.PivotLocation);
	FString NewPivotLocationName = UEnum::GetValueAsString(NewPivotLocation);
	
	if (FName(NewPivotLocationName).IsNone())
	{
		constexpr EModularSectionPivotLocation BackupPivotLocationValue = EModularSectionPivotLocation::Default;
		UE_LOG(LogModularLevel, Error, TEXT("%s: NewPivotLocationName = %s. Set NewPivotLocationName to %s"),
			*Level.Name.ToString(), *NewPivotLocationName, *UEnum::GetValueAsString(BackupPivotLocationValue));
		
		NewPivotLocation = BackupPivotLocationValue;
		NewPivotLocationName = UEnum::GetValueAsString(NewPivotLocation);
	}
	UE_LOG(LogModularLevel, Verbose, TEXT("%s: Pivot location of a level was changed: %s ---> %s"),
		*Level.Name.ToString(), *PivotLocationName, *NewPivotLocationName);

	Level.MarkUpdated(Level.PivotLocation != NewPivotLocation);
	Level.PivotLocation = NewPivotLocation;
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetName(const FName NewName)
{
#if WITH_EDITOR
	Level.Name = NewName;
#endif
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetAffectsNextZMultiplier(bool bAffects)
{
	Level.MarkUpdated(Level.bAffectsNextZMultiplier != bAffects);
	Level.bAffectsNextZMultiplier = bAffects;
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetSolver(UMBSTransformSolver* InSolver)
{
	Level.MarkUpdated(Level.Solver != InSolver);
	Level.Solver = InSolver;
	return Level;
}

FModularLevel& MBS::FModularLevelBuilder::SetUpdated(bool bNewUpdated)
{
	Level.bUpdated = bNewUpdated;
	return Level;
}