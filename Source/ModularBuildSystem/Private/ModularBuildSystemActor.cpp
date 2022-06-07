// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularBuildSystemActor.h"
#include "ModularBuildSystem.h"
#include "List/ModularBuildSystemMeshList.h"
#include "ModularBuildSystemPreset.h"
#include "List/ModularBuildSystemPresetList.h"
#include "ModularBuildSystemGenerator.h"
#include "ModularSectionResolution.h"
#include "Shape/ModularLevelShape.h"
#include "Interior/MBSInterior.h"

//#include "MBSActorVisualizationComponent.h"

#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"

#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "MeshUtilities.h"
#include "MeshMergeModule.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#endif

#include "MBSFunctionLibrary.h"
#include "Config/MBSSettings.h"
#include "Config/MBSSpawnConfiguration.h"
#include "Timer.h"
#include "Components/SplineComponent.h"
#include "Interior/MBSInteriorGenerator.h"

// Sets default values
AModularBuildSystemActor::AModularBuildSystemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// TODO: Set custom name generation for newly spawned sections (maybe in InitModularSections method).
	//SectionSpawnParams.Name = FName("MBS_Section_");

	// Creating root scene component to solve runtime creation-removal errors of attached static mesh components
	RootComponent = CreateDefaultSubobject<USceneComponent>(UMBSFunctionLibrary::DefaultRootName);

	// Root component should be static, otherwise child scene components can't be attached to it
	RootComponent->SetMobility(EComponentMobility::Static);
	
#if WITH_EDITOR
	UMBSFunctionLibrary::AddBillboard(this);
#endif

	// User should not be able to manually modify building levels until it is fully generated
	bBuildModeIsActivated = false;
	
	LevelInitializer = {this};
	LevelObserver = {this};

	// Loads config from DefaultGame.ini file
	LoadConfig();
}

void AModularBuildSystemActor::PostLoad()
{
	Super::PostLoad();

	UE_LOG(LogMBS, Log, TEXT("%s: PostLoad"), *GetName());
	if (const UMBSSettings* Settings = GetDefault<UMBSSettings>())
	{
		this->MeshConfiguration.DefaultType = Settings->DefaultMeshConfigurationType;
		this->SpawnConfiguration.DefaultClampMode = Settings->DefaultClampMode;
		this->SpawnConfiguration.DefaultExecutionMode = Settings->DefaultExecutionMode;
	}

	Sections = {this};
	SpawnConfiguration.SectionSpawnParams.Owner = this;
}

void AModularBuildSystemActor::PostActorCreated()
{
	Super::PostActorCreated();

	UE_LOG(LogMBS, Log, TEXT("%s: PostActorCreated"), *GetName());
	if (const UMBSSettings* Settings = GetDefault<UMBSSettings>())
	{
		this->MeshConfiguration.DefaultType = Settings->DefaultMeshConfigurationType;
		this->SpawnConfiguration.DefaultClampMode = Settings->DefaultClampMode;
		this->SpawnConfiguration.DefaultExecutionMode = Settings->DefaultExecutionMode;
	}

	Sections = {this};
	SpawnConfiguration.SectionSpawnParams.Owner = this;
}

void AModularBuildSystemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UpdateTransformBounds(PropertyChangedEvent);
	//SetupSpline();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AModularBuildSystemActor::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UpdateTransformBounds(PropertyChangedEvent);
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void AModularBuildSystemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Currently there is a bug that invalidates RootComponent (set it to InstancedStaticMeshComponent instead of DefaultSceneRoot after PostEditChangeProperty)
	// So we need to handle it here. Fixing root component here if necessary.
	UMBSFunctionLibrary::FixRootComponent(this);
}

// Called when the game starts or when spawned
void AModularBuildSystemActor::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnConfiguration.SectionSpawnParams.Owner = this;

#if WITH_EDITOR
	if (ensure(RootComponent))
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Root component name on BeginPlay = %s"), *GetName(), *RootComponent->GetName());
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Root component was nullptr on BeginPlay!"), *GetName());
	}
#endif
}

TArray<FModularSection> AModularBuildSystemActor::InitModularSections(const FInitModularSectionsArgs& Args)
{
	TArray<FModularSection> OutSections;
	const FTransform& ActorTransform = GetAdjustedBuildSystemActorTransform(Args.InPivotLocationOverride);

	// Calculate bounds
	//const FIntPoint Bounds = BuildStats.Bounds;
	//UE_LOG(LogMBS, Verbose, TEXT("%s: Bounds calculated: x = %d, y = %d"), *GetName(), Bounds.X, Bounds.Y);

	for (int32 i = 0; i < Args.Initializer.GetTotalCount(); i++)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: --- Initializing section at index=%d"), *GetName(), i);
		bool bOutShouldBeSkipped = false;
		FTransform NewTransform = UMBSFunctionLibrary::CalculateNewTransform(
			this, BuildStats, i, ActorTransform, Args, bOutShouldBeSkipped);

		if (bOutShouldBeSkipped)
		{
			UE_LOG(LogMBS, Verbose, TEXT("%s: Skipped index=%d section due to shape adjustment."), *GetName(), i);
			Args.OutSkippedIndices.Add(i);
			continue;
		}

		// Initialize new section at the calculated transform
		FModularSection NewSection = Sections.InitStatic(Args.Initializer.GetStaticMesh(), NewTransform, Args.InLevelId,
			false, SpawnConfiguration.bUseRelativeTransform);

		UE_LOG(LogMBS, Verbose, TEXT("%s: --- %s - has been initialized"), *GetName(), *NewSection.GetStaticMeshActor()->GetName());
		OutSections.Add(NewSection);
	}

	Args.Initializer.SetSkippedCount(Args.OutSkippedIndices.Num());
	return OutSections;
}

void AModularBuildSystemActor::UpdateModularSections(const FInitModularSectionsArgs& Args)
{
	const FTransform& ActorTransform = GetAdjustedBuildSystemActorTransform(Args.InPivotLocationOverride);
	const FModularLevel* CurrentLevel = GetLevelWithId(Args.InLevelId);
	checkf(CurrentLevel, TEXT("Args.InLevelId=%d, LevelName=%s"), Args.InLevelId, *CurrentLevel->GetName());
	
	UE_LOG(LogMBS, Verbose, TEXT("%s: Updating %s level (Args.InLevelId=%d)"), *GetName(),
		*CurrentLevel->GetName(), Args.InLevelId);
	
	TArray<FModularSection*> CurrentLevelSections = Sections.GetStaticSectionsOfLevel(*CurrentLevel);
	const int32 CurrentLevelSectionsCount = CurrentLevelSections.Num();

	check(Sections.GetStatic().Num() > 0);

	if (CurrentLevelSectionsCount == 0)
	{
		// TODO: This should not happen in the future, but for now handle it here
		// Reinitialize if no sections found for the specified modular level
		Sections.Append(InitModularSections(Args));
		return;
	}

	if (!CurrentLevel->IsUpdated())
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: %s is not updated => skip it."), *GetName(), *CurrentLevel->GetName());
		return;
	}

	const int32 TotalCount = Args.Initializer.GetTotalCount();
	checkf(CurrentLevelSectionsCount >= TotalCount,
		TEXT("%s: CurrentLevelSectionsCount=%d >= Args.Initializer.TotalCount=%d; Args.InLevelId=%d, LevelName=%s"),
		*GetName(), CurrentLevelSectionsCount, TotalCount, Args.InLevelId, *CurrentLevel->GetName());
	
	for (int32 i = 0; i < TotalCount; i++) // TODO: Maybe use SectionsOfThisLevel.Num()?
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: --- Updating section (%s) at index=%d"),
			*GetName(), *CurrentLevelSections[i]->GetName(), i);
		bool bOutShouldBeSkipped = false;
		FTransform NewTransform = UMBSFunctionLibrary::CalculateNewTransform(
			this, BuildStats, i, ActorTransform, Args, bOutShouldBeSkipped);
		// TODO: Do we need to skip here (due to shape adjustment)?

		// Update existing section with newly calculated transform
		CurrentLevelSections[i]->SetTransform(NewTransform);

		// Update mesh in case it is not the same as initializer's
		CurrentLevelSections[i]->SetMesh(Args.Initializer.GetStaticMesh());
	}

	// TODO: Remove existing or leave it to the Generator implementer? Removing for now.
	if (CurrentLevelSectionsCount > TotalCount)
	{
		Sections.RemoveSectionsAfterIndex(TotalCount, Args.InLevelId);
	}
}

FTransform AModularBuildSystemActor::GetAdjustedBuildSystemActorTransform(EModularSectionPivotLocation PivotLocation) const
{
	FTransform ActorTransform = SpawnConfiguration.bUseRelativeTransform ? FTransform::Identity : GetActorTransform();

	// Adjusting overall location of modular sections depending on the location of a pivot point 
	// (pivot point supposed to be the same for all sections).
	ActorTransform.AddToTranslation(GetLocationShiftFromSectionPivot(PivotLocation));

	return ActorTransform;
}

void AModularBuildSystemActor::AdjustSectionsUsingSpline(USplineComponent* InSpline)
{
	// TODO: Implement
	if (!InSpline)
	{
		return;
	}

	// TODO: Calculate position
	const FTransform Point0 = InSpline->GetTransformAtSplinePoint(0, ESplineCoordinateSpace::World);
	const FTransform Point1 = InSpline->GetTransformAtSplinePoint(1, ESplineCoordinateSpace::World);

	// Update sections
	if (MeshConfiguration.IsOfInstancedType())
	{
		for (auto& Section : Sections.GetInstanced())
		{
			for (int32 i = 0; i < Section.GetInstanceCount(); i++)
			{
				// Get current section transform
				FTransform SectionTransform = Section.GetSectionTransform(i);
				
				// Find the coefficient (where location of a section is between Point0 and Point1)
				const float InterpCoefficient = 0.f;
				
				// Calculate offset from the coefficient
				const FVector Offset = FVector::ZeroVector;
				
				// Apply adjustment to section transform
				SectionTransform.AddToTranslation(Offset);
				
				Section.GetISMC()->UpdateInstanceTransform(
					i,
					SectionTransform,
					true);
			}
		}
	}
	else
	{
		for (auto& Section : Sections.GetStatic())
		{
			//Section.StaticMesh->AddActorWorldOffset()
		}
	}
}

TArray<FModularSectionInstanced> AModularBuildSystemActor::InitInstancedModularSections(const FInitModularSectionsArgs& Args)
{
	// Get transform shifted by section pivot location configuration
	const FTransform& ActorTransform = GetAdjustedBuildSystemActorTransform(Args.InPivotLocationOverride);

	// If instanced - initializing here and adding new instances to it's component in for loop.
	FModularSectionInstanced NewSection = Sections.InitInstanced(Args.InLevelId, false,
		Args.InInstancedStaticMeshComponent);
	
	for (int32 i = 0; i < Args.Initializer.GetTotalCount(); i++)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: --- Initializing section at index=%d"), *GetName(), i);
		bool bOutShouldBeSkipped = false;
		FTransform NewTransform = UMBSFunctionLibrary::CalculateNewTransform(
			this, BuildStats, i, ActorTransform, Args, bOutShouldBeSkipped);

		if (bOutShouldBeSkipped)
		{
			UE_LOG(LogMBS, Verbose, TEXT("%s: Skipped index=%d section due to shape adjustment."),
				*GetName(), i);
			Args.OutSkippedIndices.Add(i);
			continue;
		}

		// Adding new instance to instanced static mesh component of section
		AddNewInstance(NewTransform, SpawnConfiguration.bUseRelativeTransform, Args.InInstancedStaticMeshComponent);

		UE_LOG(LogMBS, Verbose, TEXT("%s: --- %s - has been initialized"), *GetName(), *NewSection.GetSectionName(i));
	}

	Sections.UpdateInstanceCount(NewSection);
	Args.Initializer.SetSkippedCount(Args.OutSkippedIndices.Num());
	return TArray<FModularSectionInstanced>({ NewSection });
}

void AModularBuildSystemActor::UpdateInstancedModularSections(const FInitModularSectionsArgs& Args)
{
	const FTransform& ActorTransform = GetAdjustedBuildSystemActorTransform(Args.InPivotLocationOverride);
	const FModularLevel* CurrentLevel = GetLevelWithId(Args.InLevelId);
	const int32 TestInstanceCount = CurrentLevel->InstancedStaticMeshComponent->GetInstanceCount();
	
	checkf(CurrentLevel, TEXT("Args.InLevelId=%d, LevelName=%s"), Args.InLevelId, *CurrentLevel->GetName());

	const int32 TotalCount = Args.Initializer.GetTotalCount();
	UE_LOG(LogMBS, Log, TEXT("%s: Updating %s level (Args.InLevelId=%d, InstanceCount=%d, Args.Initializer.TotalCount=%d)"),
		*GetName(), *CurrentLevel->GetName(), Args.InLevelId, TestInstanceCount, TotalCount);
	
	FModularSectionInstanced* ExistingSection = Sections.GetInstancedSectionOfLevel(Args.InLevelId);
	check(ExistingSection);
	const int32 CurrentLevelSectionCount = ExistingSection->GetInstanceCount();

	check(Sections.GetInstanced().Num() > 0);
	if (CurrentLevelSectionCount == 0)
	{
		Sections.Append(InitInstancedModularSections(Args));
		return;
	}
	
	if (CurrentLevelSectionCount < TotalCount)
	{
		// TODO: Currently clear this level if not all sections are set.
		// TODO: It is necessary to find which instances were removed before updating existing, as it can break easily.
		Sections.ResetInstancedSectionOfLevel(Args.InLevelId);
		Sections.Append(InitInstancedModularSections(Args));
		return;
	}

	for (int32 i = 0; i < TotalCount; i++)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: --- Updating section at index=%d"), *GetName(), i);
		bool bOutShouldBeSkipped = false;
		FTransform NewTransform = UMBSFunctionLibrary::CalculateNewTransform(
			this, BuildStats, i, ActorTransform, Args, bOutShouldBeSkipped);
		// TODO: Skip here?
		
		// Update existing instance with newly calculated transform
		ExistingSection->SetTransform(NewTransform, true, i);
	}

	// TODO: Remove existing or leave it to the Generator implementer? Removing for now.
	if (CurrentLevelSectionCount > TotalCount)
	{
		Sections.RemoveInstancedSectionInstancesAfterIndex(TotalCount, Args.InLevelId);
	}
}

FModularSection AModularBuildSystemActor::InitModularSection(UStaticMesh* InStaticMesh, const FTransform& InTransform,
	int32 InLevelId, bool bAddToSections, bool bWithRelativeTransform)
{
	return Sections.InitStatic(InStaticMesh, InTransform, InLevelId, bAddToSections, bWithRelativeTransform);
}

TArray<FModularSection> AModularBuildSystemActor::InitMultipleModularSections(UStaticMesh* InStaticMesh,
	const TArray<FTransform>& InTransforms, int32 InLevelId, bool bAddToSections, bool bWithRelativeTransform)
{
	if (!InStaticMesh || !FModularLevel::IsValidLevelId(InLevelId))
	{
		return {};
	}
	
	TArray<FModularSection> OutSections;
	for (const FTransform& Transform : InTransforms)
	{
		OutSections.Add(Sections.InitStatic(InStaticMesh, Transform, InLevelId, bAddToSections, bWithRelativeTransform));
	}
	return OutSections;
}

FModularSectionActor AModularBuildSystemActor::InitModularSectionActor(const FTransform& InTransform, int32 InLevelId,
	TSubclassOf<AActor> InClass, bool bAddToActorSections, bool bWithRelativeTransform)
{
	check(FModularLevel::IsValidLevelId(InLevelId));
	
	FModularSectionActor NewSectionActor = FModularSectionActor(
		InLevelId, Sections.SpawnNewSectionActor(InTransform, InClass, SpawnConfiguration.SectionSpawnParams));

	checkf(NewSectionActor.IsValid(), TEXT("%s: NewSectionActor was invalid: %s"), *GetName(), *NewSectionActor.ToString());
	AttachActor(NewSectionActor.GetActor(), bWithRelativeTransform);

	if (bAddToActorSections)
	{
		Sections.Add(NewSectionActor);
	}

	UE_LOG(LogMBS, Verbose, TEXT("%s: %s - modular section actor (of %s class) has been initialized"),
		*GetName(), *NewSectionActor.GetActor()->GetName(), *InClass->GetName());
	return NewSectionActor;
}

void AModularBuildSystemActor::AddNewInstance(const FTransform& InTransform, bool bWithRelativeTransform,
	UInstancedStaticMeshComponent* InInstancedStaticMeshComponent)
{
	Sections.AddNewInstance(InTransform, bWithRelativeTransform, InInstancedStaticMeshComponent);
}

void AModularBuildSystemActor::SelectAll_Implementation()
{
#if WITH_EDITOR
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting all sections of a build system."), *GetName());
	Sections.SelectAll();
#else
	UE_LOG(LogBuildSystem, Warning, TEXT("%s: SelectAll don't work during Runtime!"), *GetName());
#endif
}

void AModularBuildSystemActor::UnselectAll_Implementation()
{
#if WITH_EDITOR
	UE_LOG(LogMBS, Log, TEXT("%s: Unselecting all sections of a build system."), *GetName());
	Sections.UnselectAll();
#else
	UE_LOG(LogBuildSystem, Warning, TEXT("%s: UnselectAll don't work during Runtime!"), *GetName());
#endif
}

void AModularBuildSystemActor::Init()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Running initialization"), *GetName());

	BuildStats.Init();

	// Update overall stats of the build system actor, as they may be necessary in some resolution calculations (corners, for example)
	CollectStats();

	UE_LOG(LogMBS, VeryVerbose, TEXT("%s: [Init_Implementation] Root component name = %s"),
		*GetName(), *GetRootComponent()->GetName());
}

void AModularBuildSystemActor::Generate_Implementation()
{
	UModularBuildSystemGenerator* Generator = Cast<UModularBuildSystemGenerator>(GetGenerator().GetObject());
	if (Generator && !bBuildModeIsActivated)
	{
#if WITH_EDITOR
		if (Merger.bIsMerged)
		{
			UE_LOG(LogMBS, Warning, TEXT("%s: Build system actor is merged. Skipping generation."),
				*GetName());
			return;
		}
#endif

		UE_LOG(LogMBS, Log, TEXT("%s: Generating new modular building using %s generator"), *GetName(),
			*Generator->GetName());
		
		Generator->SetBuildSystemPtr(this);
		if (IBuildingGeneratorInterface::Execute_Generate(Generator).Succeeded())
		{
			ApplyStretch();
		}
	}
	else if (!Generator)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Cannot generate because Generator was nullptr."), *GetName());
	}
	else if (bBuildModeIsActivated)
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Cannot generate because build mode is active."), *GetName());
	}
}

#if WITH_EDITOR

void AModularBuildSystemActor::ToggleShowOnlyInterior_Implementation()
{
	DisplaySettings.bShowInteriorOnly = !DisplaySettings.bShowInteriorOnly;
	Sections.SetVisibility(!DisplaySettings.bShowInteriorOnly);
}

void AModularBuildSystemActor::MergeIntoStaticMesh()
{
	Merger.MergeIntoStaticMesh(this);
}

void AModularBuildSystemActor::UnmergeIntoModularSections()
{
	if (Merger.bIsMerged)
	{
		Merger.UnmergeIntoModularSections(this);
		if (!Sections.IsAnyNotEmpty())
		{
			UE_LOG(LogMBS, Log, TEXT("%s: Stretching"), *GetName());
			TransformBounds.SetBounds(TransformBounds.GetBounds(), StretchManager.GetScaleCoefficientsSwappedXY());
			ApplyStretch();
		}
	}
}

void AModularBuildSystemActor::FixModularSectionsTransform()
{
	UE_LOG(LogMBS, Warning, TEXT("%s: Fixing modular sections transform."), *GetName());

	if (!Merger.bIsMerged)
	{
		UE_LOG(LogMBS, Log, TEXT("%s: Reattaching modular section static mesh actors."), *GetName());
		for (const auto& Section : Sections.GetStatic())
		{
			Section.GetStaticMeshActor()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		UE_LOG(LogMBS, Log, TEXT("%s: Reattaching modular section actors."), *GetName());
		for (const auto& Section : Sections.GetActor())
		{
			Section.GetActor()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		UE_LOG(LogMBS, Log, TEXT("%s: Reattaching instanced modular sections."), *GetName());
		for (const auto& Section : Sections.GetInstanced())
		{
			Section.GetISMC()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		UE_LOG(LogMBS, Log, TEXT("%s: All modular sections transforms were fixed."), *GetName());
	}
	else if (Merger.MergedSectionsStaticMeshActor)
	{
		UE_LOG(LogMBS, Log, TEXT("%s: Reattaching merged static mesh actor."), *GetName());
		Merger.MergedSectionsStaticMeshActor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		UE_LOG(LogMBS, Log, TEXT("%s: Merged static mesh actor transform has been fixed."), *GetName());

		UE_LOG(LogMBS, Log, TEXT("%s: Reattaching modular section actors."), *GetName());
		for (const auto& Section : Sections.GetActor())
		{
			Section.GetActor()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		UE_LOG(LogMBS, Log, TEXT("%s: All modular section actors transforms were fixed."), *GetName());
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: MergedSectionsStaticMeshActor was nullptr"), *GetName());
	}
}

void AModularBuildSystemActor::SelectSections(const FModularLevel& InLevel) const
{
	Sections.SelectSections(InLevel);
}

void AModularBuildSystemActor::SelectSections(const TArray<FModularLevel>& InLevels) const
{
	UE_LOG(LogMBS, Log, TEXT("%s: Start multiple level selection."), *GetActorLabel());

	if (Merger.bIsMerged)
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: Can't perform selection when building is merged. Aborting."), *GetActorLabel());
		return;
	}

	Sections.SelectSections(InLevels);
	UE_LOG(LogMBS, Log, TEXT("%s: Multiple level selection performed."), *GetActorLabel());
}

void AModularBuildSystemActor::UnselectSections(const FModularLevel& InLevel) const
{
	Sections.UnselectSections(InLevel);
}

void AModularBuildSystemActor::UnselectSections(const TArray<FModularLevel>& InLevels) const
{
	UE_LOG(LogMBS, Log, TEXT("%s: Unselecting multiple levels."), *GetActorLabel());
	Sections.UnselectSections(InLevels);
	UE_LOG(LogMBS, Log, TEXT("%s: Multiple levels were unselected."), *GetActorLabel());
}

#endif

void AModularBuildSystemActor::ResetBuildSystem(bool bResetSections, bool bResetActorSections,
	bool bResetInstancedSections, bool bResetMergedSectionsStaticMeshActor)
{
	UE_LOG(LogMBS, Log, TEXT("%s: Resetting sections and clearing sections array."), *GetName());

	UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Build system transform before reset: %s"),
		*GetName(), *GetActorTransform().ToHumanReadableString());
	UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Build system root component name before reset: %s"),
		*GetName(), *GetRootComponent()->GetName());

	Sections.Reset(bResetSections, bResetActorSections, bResetInstancedSections);

	if (SpawnConfiguration.bResetRotationAlso)
	{
		this->SetActorRotation(FRotator::ZeroRotator);
	}

	if (bResetMergedSectionsStaticMeshActor)
	{
		Merger.Reset(this);
	}

	Sections.SetWasReset(true);
	UE_LOG(LogMBS, Verbose, TEXT("%s: Build system transform after reset: %s"), *GetName(), *GetActorTransform().ToHumanReadableString());
}

void AModularBuildSystemActor::RemoveSectionsOfLevel(int32 LevelId)
{
	Sections.RemoveSectionsOfLevel(LevelId);
}

void AModularBuildSystemActor::ResetInstancedSectionOfLevel(int32 LevelId)
{
	Sections.ResetInstancedSectionOfLevel(LevelId);
}

void AModularBuildSystemActor::ClearInstancedSectionOfLevel(int32 LevelId)
{
	Sections.ClearInstancedSectionOfLevel(LevelId);
}

void AModularBuildSystemActor::RemoveSectionsAfterIndex(int32 Index, int32 LevelId)
{
	Sections.RemoveSectionsAfterIndex(Index, LevelId);
}

void AModularBuildSystemActor::RemoveActorSectionsAfterIndex(int32 Index, int32 LevelId)
{
	Sections.RemoveActorSectionsAfterIndex(Index, LevelId);
}

void AModularBuildSystemActor::RemoveInstancedSectionsAfterIndex(int32 Index, int32 LevelId)
{
	Sections.RemoveInstancedSectionsAfterIndex(Index, LevelId);
}

void AModularBuildSystemActor::RemoveInstancedSectionInstancesAfterIndex(int32 Index, int32 LevelId)
{
	Sections.RemoveInstancedSectionInstancesAfterIndex(Index, LevelId);
}

void AModularBuildSystemActor::ReloadMeshes(int32 InLevelId)
{
	Sections.ReloadMeshes(InLevelId);
}

void AModularBuildSystemActor::ResetCurrentModifiedLevel()
{
	Sections.ClearInvalidSections();

	if (!FModularLevel::IsValidLevelId(LevelObserver.GetModifiedLevelId()))
	{
		UE_LOG(LogMBS, Error, TEXT("%s: ModifiedLevelId is invalid. It is possible that you didn't modify any level at this point"),
			*GetName());
	}

	Sections.ResetSingleLevel(LevelObserver.GetModifiedLevelId());
	Sections.ClearInvalidSections();

	if (SpawnConfiguration.bResetRotationAlso)
	{
		this->SetActorRotation(FRotator::ZeroRotator);
	}
}

void AModularBuildSystemActor::ResetSingleLevel(int32 LevelId)
{
	Sections.ResetSingleLevel(LevelId);
}

void AModularBuildSystemActor::EndInit()
{
	Sections.SetWasReset(false);
	UE_LOG(LogMBS, Log, TEXT("%s: Initialized %d sections."), *GetName(), Sections.GetStatic().Num());
	UE_LOG(LogMBS, Log, TEXT("%s: Initialization ends"), *GetName());
}

void AModularBuildSystemActor::CollectStats()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Collecting stats"), *GetName());
	for (const FModularLevel* Level : GetAllLevels())
	{
		BuildStats.Update(Level->GetInitializer().GetBuildStats());
	}
}

void AModularBuildSystemActor::UpdateTransformBounds(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		UE_LOG(LogMBS, VeryVerbose, TEXT("%s: PostEditChangeProperty | PropertyName = %s"), *GetActorLabel(),
			*PropertyName.ToString());

		// Handle here TransformBounds
		if (PropertyName == "FrontLocation"
			|| PropertyName == "RightLocation"
			|| PropertyName == "BackLocation"
			|| PropertyName == "LeftLocation"
			|| PropertyName == "TopLocation")
		{
			UE_LOG(LogMBS, VeryVerbose, TEXT("%s: UpdateTransformBounds; PropertyName=%s"), *GetName(), *PropertyName.ToString());
			TransformBounds.SetUpdatedTransformName(PropertyName);
			if (TransformBounds.UpdateBounds(this, PropertyName))
			{
				// Regenerate with new bounds
				OnUpdateTransformBounds();
				// TODO: Call OnBoundsTransformChange here also?
				TransformBounds.ResetUpdated();
			}
			else
			{
				OnBoundsTransformChange();
				TransformBounds.ResetUpdated();
			}
		}
	}
}

void AModularBuildSystemActor::UpdateTransformBounds(const FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		const auto StructProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
		const FName StructName = StructProperty->GetFName();
			
		UE_LOG(LogMBS, VeryVerbose, TEXT("%s: StructName: %s"), *GetName(), *StructName.ToString());
		if (StructName == GET_MEMBER_NAME_CHECKED(AModularBuildSystemActor, TransformBounds))
		{
			UE_LOG(LogMBS, VeryVerbose, TEXT("%s: TransformBounds"), *GetName());
			if (TransformBounds.UpdateBounds(this, FName()))
			{
				// Regenerate with new bounds
				OnUpdateTransformBounds();
				TransformBounds.ResetUpdated();
			}
		}
	}
}

void AModularBuildSystemActor::AttachActor(AActor* ToAttach, bool bWithRelativeTransform, bool bForceReattach)
{
	checkf(ToAttach, TEXT("%s: Passed actor was nullptr"), *GetName());
	if (!ToAttach->IsAttachedTo(this))
	{
		ToAttach->AttachToActor(this, bWithRelativeTransform ?
			FAttachmentTransformRules::KeepRelativeTransform : FAttachmentTransformRules::KeepWorldTransform);
		UE_LOG(LogMBS, Verbose, TEXT("%s: Section %s has been attached to this build system with %s attachment rule."),
			*GetName(), *ToAttach->GetName(), bWithRelativeTransform ? TEXT("KeepRelativeTransform") : TEXT("KeepWorldTransform"));
	}
	else
	{
		UE_LOG(LogMBS, Error, TEXT("%s: Section %s is already attached to this MBS actor."),
			*GetName(), *ToAttach->GetName());

		if (bForceReattach)
		{
			UE_LOG(LogMBS, Verbose, TEXT("%s: Reattaching (bForceReattach=%s)."),
				*GetName(), (bForceReattach ? TEXT("true") : TEXT("false")));
			
			ToAttach->DetachFromActor(bWithRelativeTransform ?
				FDetachmentTransformRules::KeepRelativeTransform : FDetachmentTransformRules::KeepWorldTransform);
			AttachActor(ToAttach, bWithRelativeTransform, false);
		}
	}
}

FVector AModularBuildSystemActor::GetLocationShiftFromSectionPivot(EModularSectionPivotLocation PivotLocation) const
{
	const FString PivotLocationName = UEnum::GetValueAsString(PivotLocation);
	if (PivotLocation == EModularSectionPivotLocation::Default)
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: PivotLocation == EModularSectionPivotLocation::Default. "
			"Using MBS actor SectionPivotLocation property value instead (%s)."),
			*GetName(), *UEnum::GetValueAsString(SpawnConfiguration.SectionPivotLocation));
		PivotLocation = SpawnConfiguration.SectionPivotLocation;
	}
	else if (PivotLocationName.IsEmpty() ||
		FName(PivotLocationName).IsNone() || 
		PivotLocationName.Equals(TEXT("EModularSectionPivotLocation::EModularSectionPivotLocation_MAX")))
	{
		UE_LOG(LogMBS, Error, TEXT("%s: PivotLocation = %s. PivotLocation is invalid!"),
			*GetName(), *PivotLocationName);
	}
	else
	{
		UE_LOG(LogMBS, Verbose, TEXT("%s: PivotLocation = %s "), *GetName(), *PivotLocationName);
	}
	
	switch (PivotLocation)
	{
	case EModularSectionPivotLocation::Default:
		// TODO: add logic for default case
		return FVector::ZeroVector;
	case EModularSectionPivotLocation::BottomRight:
		return FVector(0.f, UModularSectionResolution::DefaultSectionSize, 0.f);
	case EModularSectionPivotLocation::BottomLeft:
		return FVector::ZeroVector;
	case EModularSectionPivotLocation::TopRight:
		//return FVector(0.f, UModularSectionResolution)
		// TODO: Implement
		unimplemented();
		return FVector::ZeroVector;
	case EModularSectionPivotLocation::TopLeft:
		// TODO: Implement
		unimplemented();
		return FVector::ZeroVector;
	default:
		return FVector::ZeroVector;
	}
}

TArray<FModularSection*> AModularBuildSystemActor::GetSectionsOfLevel(const FModularLevel& InLevel) const
{
	return Sections.GetStaticSectionsOfLevel(InLevel.GetId());
}

TArray<FModularSection*> AModularBuildSystemActor::GetSectionsOfLevel(const int32 InLevelId) const
{
	return Sections.GetStaticSectionsOfLevel(InLevelId);
}

TArray<FModularSectionActor*> AModularBuildSystemActor::GetActorSectionsOfLevel(const FModularLevel& InLevel) const
{
	return Sections.GetActorSectionsOfLevel(InLevel);
}

TArray<FModularSectionActor*> AModularBuildSystemActor::GetActorSectionsOfLevel(const int32 InLevelId) const
{
	return Sections.GetActorSectionsOfLevel(InLevelId);
}

FModularSectionInstanced* AModularBuildSystemActor::GetInstancedSectionOfLevel(const FModularLevel& InLevel) const
{
	return Sections.GetInstancedSectionOfLevel(InLevel);
}

FModularSectionInstanced* AModularBuildSystemActor::GetInstancedSectionOfLevel(const int32 InLevelId) const
{
	return Sections.GetInstancedSectionOfLevel(InLevelId);
}

FBox AModularBuildSystemActor::GetBoundsBox(bool bRelative) const
{
	if (bRelative)
	{
		return TransformBounds.GetBox(StretchManager.GetScaleCoefficients());
	}
	
	FBox OutBox = TransformBounds.GetBox(StretchManager.GetScaleCoefficients());
	const FTransform& Transform = GetActorTransform();
	OutBox.Min = Transform.GetRelativeTransform(FTransform(OutBox.Min)).GetLocation();
	OutBox.Max = Transform.GetRelativeTransform(FTransform(OutBox.Max)).GetLocation();
	return OutBox;
}

bool AModularBuildSystemActor::IsValidSectionIndex(const FModularLevel& InLevel, int32 Index) const
{
	return Sections.IsValidSectionIndex(InLevel, Index);
}

void AModularBuildSystemActor::OffsetModularLevel(const FModularLevel& Level, FVector Offset, bool bUpdateZMultiplier) const
{
	Sections.OffsetLevel(Level, Offset, bUpdateZMultiplier);
}

void AModularBuildSystemActor::OffsetModularSection(const FModularLevel& Level, int32 SectionIndex, FVector Offset,
	bool bRelative) const
{
	Sections.OffsetSection(Level, SectionIndex, Offset, bRelative);
}

void AModularBuildSystemActor::SetMeshForEach(const FModularLevel& InLevel, int32 InEachElement, UStaticMesh* InMesh)
{
	Sections.SetMeshForEach(InLevel, InEachElement, InMesh);
}

void AModularBuildSystemActor::SetMeshAt(const FModularLevel& InLevel, int32 InElement, int32 InRow, UStaticMesh* InMesh)
{
	Sections.SetMeshAt(InLevel, InElement, InRow, InMesh);
}

void AModularBuildSystemActor::SetMeshAt(const FModularLevel& InLevel, int32 InIndex, UStaticMesh* InMesh)
{
	Sections.SetMeshAt(InLevel, InIndex, InMesh);
}

FModularSection* AModularBuildSystemActor::GetSectionAt(const FModularLevel& InLevel, int32 InIndex) const
{
	return Sections.GetSectionAt(InLevel, InIndex);
}

FModularSection* AModularBuildSystemActor::GetSectionAt(const FModularLevel& InLevel, int32 InElement, int32 InRow) const
{
	return Sections.GetSectionAt(InLevel, InElement, InRow);
}

FTransform AModularBuildSystemActor::GetSectionTransformAt(int32 InLevelId, int32 InIndex, bool bInstanced,
	bool bWorldSpace) const
{
	return Sections.GetSectionTransformAt(InLevelId, InIndex, bInstanced, bWorldSpace);
}

FTransform AModularBuildSystemActor::GetSectionTransformAt(const FModularLevel& InLevel, int32 InIndex,
	bool bInstanced, bool bWorldSpace) const
{
	return Sections.GetSectionTransformAt(InLevel, InIndex, bInstanced, bWorldSpace);
}

TArray<FTransform> AModularBuildSystemActor::GetSectionTransformAt(const FModularLevel& InLevel,
	TArray<int32> InIndices, bool bInstanced, bool bWorldSpace) const
{
	return Sections.GetSectionTransformAt(InLevel, InIndices, bInstanced, bWorldSpace);
}

FTransform AModularBuildSystemActor::GetSectionTransformAtRandom(int32 InLevelId, bool bInstanced,
	bool bWorldSpace) const
{
	return Sections.GetSectionTransformAtRandom(InLevelId, bInstanced, bWorldSpace);
}

FTransform AModularBuildSystemActor::GetSectionTransformAtRandom(const FModularLevel& InLevel, bool bInstanced,
	bool bWorldSpace) const
{
	return Sections.GetSectionTransformAtRandom(InLevel, bInstanced, bWorldSpace);
}

bool AModularBuildSystemActor::CanReload(const FTransform& CurrentTransform) const
{
	if (SpawnConfiguration.bReloadOnConstruction)
	{
		return SpawnConfiguration.bReconstructOnMove ? true : CurrentTransform.Equals(PreviousTransform);
	}
	return false;
}

TArray<FModularSectionBase*> AModularBuildSystemActor::GetAllSections()
{
	return Sections.GetAll();
}

int32 AModularBuildSystemActor::GetFirstIndexOfSectionWithLevelId(const int32 InLevelId) const
{
	return Sections.GetFirstIndexOfSectionWithLevelId(InLevelId);
}

int32 AModularBuildSystemActor::GetLastIndexOfSectionWithLevelId(const int32 InLevelId) const
{
	return Sections.GetLastIndexOfSectionWithLevelId(InLevelId);
}

FModularSection* AModularBuildSystemActor::ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh, int32 InInstanceIndex,
	int32 InLevelId, UInstancedStaticMeshComponent* InComponent, FTransform& OutReplacedInstanceTransform)
{
	return Sections.ReplaceWithNonInstancedSection(InNewStaticMesh, InInstanceIndex, InLevelId, InComponent, OutReplacedInstanceTransform);
}

FModularSection* AModularBuildSystemActor::ReplaceWithNonInstancedSection(UStaticMesh* InNewStaticMesh,
	int32 InInstanceIndex, int32 InLevelId, FTransform& OutReplacedInstanceTransform)
{
	return Sections.ReplaceWithNonInstancedSection(InNewStaticMesh, InInstanceIndex, InLevelId, OutReplacedInstanceTransform);
}

void AModularBuildSystemActor::SetLevelVisibility(const FModularLevel& InLevel, bool bVisible) const
{
	if (Merger.bIsMerged)
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: Can't change visibility when building is merged. Aborting."), *GetName());
		return;
	}

	Sections.SetVisibility(InLevel, bVisible);
}

void AModularBuildSystemActor::SetLevelVisibility(const TArray<FModularLevel>& InLevels, bool bVisible) const
{
	if (Merger.bIsMerged)
	{
		UE_LOG(LogMBS, Warning, TEXT("%s: Can't change visibility when building is merged. Aborting."), *GetName());
		return;
	}
	
	for (const auto& Level : InLevels)
	{
		Sections.SetVisibility(Level, bVisible);
	}
}

void AModularBuildSystemActor::UpdateMeshes(FModularLevel& InLevel)
{
	if (MeshConfiguration.bDynamicallyChangeMeshes && !bBuildModeIsActivated)
	{
		InLevel.SetFromMeshList();
		Sections.ReloadMeshes(MeshConfiguration.ReloadMode == EModularSectionReloadMode::CurrentLevelOnly
			? LevelObserver.GetModifiedLevelId() : FModularLevel::InvalidLevelId);
	}
}

void AModularBuildSystemActor::UpdateMeshes(TArray<FModularLevel>& InLevels)
{
	if (MeshConfiguration.bDynamicallyChangeMeshes && !bBuildModeIsActivated)
	{
		for (auto& Level : InLevels)
		{
			Level.SetFromMeshList();
		}
		Sections.ReloadMeshes(MeshConfiguration.ReloadMode == EModularSectionReloadMode::CurrentLevelOnly
			? LevelObserver.GetModifiedLevelId() : FModularLevel::InvalidLevelId);
	}
}

void AModularBuildSystemActor::AppendSections(const FInitModularSectionsArgs& Args)
{
	// TODO: Maybe add another bool property that will show if PlaceRemainingActors method should run or not
	// (if conditions are not met). This could be useful, as it will eliminate the need to check if CanShapeTransform
	// method returns true.
	const FMBSPlaceRemainingActorsArgs PlaceRemainingActorsArgs(
		BuildStats.Bounds,
		0,
		Args.InLevelId,
		Args.InLevelZMultiplier,
		&Args.Initializer,
		this,
		Args.OutSkippedIndices);
	
	const FMBSShapeTransformArgs ShapeTransformArgs(
		0,
		0,
		BuildStats.Bounds,
		&Args.Initializer,
		this,
		Args.InLevelId,
		nullptr,
		nullptr);
	
	if (SpawnConfiguration.ExecutionMode == EMBSExecutionMode::Smart)
	{
		// TODO: Handle smart execution
		
		if (MeshConfiguration.IsOfInstancedType())
		{
			// Initialize if sections are cleared
			if (Sections.GetInstanced().IsEmpty())
			{
				Sections.Append(InitInstancedModularSections(Args));
			}
			// Update existing instanced modular sections
			else
			{
				// TODO: Atm, update do not work for instanced sections, so initializing here instead.
				//UpdateInstancedModularSections(Args);
				Sections.Append(InitInstancedModularSections(Args));
			}

			// TODO: Test
			if (Args.InShape)
			{
				Args.InShape->PlaceRemainingActors(PlaceRemainingActorsArgs);
			}
		}
		else
		{
			if (Sections.GetStatic().IsEmpty())
			{
				Sections.Append(InitModularSections(Args));
			}
			// Update existing modular sections
			else
			{
				UpdateModularSections(Args);
			}

			// TODO: Test
			// Should not be triggered if Args.InShape->CanShapeTransform is false
			if (Args.InShape && Args.InShape->CanShapeTransform(ShapeTransformArgs))
			{
				Args.InShape->PlaceRemainingActors(PlaceRemainingActorsArgs);
			}
		}

		// TODO: Handle custom shape update
	}
	else
	{
		if (MeshConfiguration.IsOfInstancedType())
		{
			Sections.Append(InitInstancedModularSections(Args));
		}
		else
		{
			Sections.Append(InitModularSections(Args));
		}

		if (Args.InShape && Args.InShape->CanShapeTransform(ShapeTransformArgs))
		{
			Args.InShape->PlaceRemainingActors(PlaceRemainingActorsArgs);
		}
	}
}
