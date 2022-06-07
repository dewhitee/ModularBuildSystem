// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSFunctionLibrary.h"

#include "EngineUtils.h"
#include "MBSGeneratorProperty.h"
#include "ModularBuildSystemGenerator.h"
#include "Timer.h"
#include "ModularLevel.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "Components/BillboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "List/ModularBuildSystemActorList.h"
#include "List/ModularBuildSystemMeshList.h"
#include "Shape/ModularLevelShape.h"

#if WITH_EDITOR
#include "ObjectTools.h"
#endif

FModularLevel* UMBSFunctionLibrary::GetLevelWithIdChecked(const TArray<FModularLevel>& Levels, int32 Id)
{
	for (const auto& Level : Levels)
	{
		if (Level.GetId() == Id)
		{
			return const_cast<FModularLevel*>(&Level);
		}
	}
	return nullptr;
}

void UMBSFunctionLibrary::OffsetSections(const TArray<FModularSection*>& InSections, FVector Offset, bool bRelative)
{
	for (FModularSection* Section : InSections)
	{
		Section->Offset(Offset, bRelative);
	}
}

void UMBSFunctionLibrary::ScaleSections(const TArray<FModularSection*>& InSections, FVector Offset, bool bRelative)
{
	for (FModularSection* Section : InSections)
	{
		Section->Scale(Offset, bRelative);
	}
}

void UMBSFunctionLibrary::ForEachBuildSystem(void (*InFunction)(TActorIterator<AModularBuildSystemActor> It))
{
	const UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogMBS, Error, TEXT("World was nullptr!"));
		return;
	}
	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		InFunction(It);
	}
}

void UMBSFunctionLibrary::ForEachLevel(const TArray<FModularLevel*>& InLevels,
	TFunction<void(FModularLevel&)> InFunction)
{
	for (auto& Level : InLevels)
	{
		InFunction(*Level);
	}
}

void UMBSFunctionLibrary::ForEachLevel(const TArray<FModularLevel*>& InLevels,
	const TFunction<void(const FModularSectionInitializer&)> InFunction)
{
	for (auto& Level : InLevels)
	{
		InFunction(*&Level->GetInitializer());
	}
}

void UMBSFunctionLibrary::ForEachLevel(AModularBuildSystemActor* BS, const TArray<FModularLevel*>& InLevels,
	void(AModularBuildSystemActor::* InFunction)(FModularLevel&))
{
	check(BS);
	for (auto& Level : InLevels)
	{
		(BS->*InFunction)(*Level);
	}
}

void UMBSFunctionLibrary::ForEachLevel(AModularBuildSystemActor* BS, const TArray<FModularLevel*>& InLevels,
	void(AModularBuildSystemActor::* InFunction)(const FModularSectionInitializer&))
{
	check(BS);
	for (auto& Level : InLevels)
	{
		(BS->*InFunction)(*&Level->GetInitializer());
	}
}

void UMBSFunctionLibrary::InitSectionFromProperty(AModularBuildSystemActor* BS, const UMBSGeneratorProperty* Property, bool bRandomIndex,
	int32 DefaultIndex, const FTransform& AtTransform, int32 LevelId, bool bAddToSections, bool bWithRelativeTransform)
{
	check(BS);
	check(Property);
	if (Property->Data.bUseActorList)
	{
		const int32 Index = bRandomIndex
			? FMath::RandRange(0, Property->Data.ActorList->GetMaxIndex(Property->Data.Resolution))
			: DefaultIndex;

		const TSubclassOf<AActor> Class = Property->Data.GetActorClass(Index);
		BS->InitModularSectionActor(AtTransform, LevelId, Class, bAddToSections, bWithRelativeTransform);
	}
	else
	{
		const int32 Index = bRandomIndex
			? FMath::RandRange(0, Property->Data.MeshList->GetMaxIndex(Property->Data.Resolution))
			: DefaultIndex;

		UStaticMesh* Mesh = Property->Data.GetMesh(Index);
		BS->InitModularSection(Mesh, AtTransform, LevelId, bAddToSections, bWithRelativeTransform);
	}
}

UPackage* UMBSFunctionLibrary::CreatePackageChecked(const FString& PackagePath, FString AssetName, const UClass* Class)
{
	UPackage* Package = CreatePackage(*PackagePath);
	Package->FullyLoad();
	Package->Modify();

	// Check name - rename if already exists
	if (const UObject* ExistingObject = StaticFindObject(nullptr, Package, *AssetName))
	{
		// Change name of merged static mesh to avoid name collision
		UPackage* ParentPackage = CreatePackage(*FPaths::GetPath(Package->GetPathName()));
		ParentPackage->FullyLoad();

		AssetName = MakeUniqueObjectName(ParentPackage, Class, *AssetName).ToString();
		Package = CreatePackage(*(ParentPackage->GetPathName() / AssetName));
		check(Package);
		Package->FullyLoad();
		Package->Modify();

		UE_LOG(LogMBS, Warning, TEXT("Package already exists, saving it with a new name %s"), *AssetName);
	}

	UE_LOG(LogMBS, Warning, TEXT("Created package at path %s"), *PackagePath);
	return Package;
}

UWorld* UMBSFunctionLibrary::GetTestWorld()
{
	if (GEditor && GEditor->GetWorldContexts().Num() && GEditor->GetWorldContexts()[0].World())
	{
		//UE_LOG(LogGameGeneric, Verbose, TEXT("Getting world from editor"))
		return GEditor->GetWorldContexts()[0].World();
	}
	if (GEngine && GEngine->GetWorldContexts().Num() && GEngine->GetWorldContexts()[0].World())
	{
		//UE_LOG(LogGameGeneric, Verbose, TEXT("Getting world from engine"))
		return GEngine->GetWorldContexts()[0].World();
	}
	return nullptr;
}

UTexture2D* UMBSFunctionLibrary::GetMeshAssetThumbnail(UStaticMesh* InStaticMesh)
{
	// TODO: Guard this method WITH_EDITOR in header file.
#if WITH_EDITOR
	if (InStaticMesh)
	{
		if (!InStaticMesh->IsAsset())
		{
			UE_LOG(LogMBS, Error, TEXT("%s is not an Asset!!!"), *InStaticMesh->GetName());
		}
		else
		{
			UE_LOG(LogMBS, Log, TEXT("%s is an Asset"), *InStaticMesh->GetName());
		}

		if (const UPackage* Package = InStaticMesh->GetPackage())
		{
			UE_LOG(LogMBS, Log, TEXT("Package: %s"), *Package->GetName());
			TSet<FName> Names;
			FThumbnailMap ThumbnailMap;
			FString PackageName;

			if (const FObjectThumbnail* MeshThumbnail = ThumbnailTools::GetThumbnailForObject(InStaticMesh))
			{
				UE_LOG(LogMBS, Log, TEXT("MeshThumbnail found"));

				const TArray<uint8>* UncompressedImageData = &MeshThumbnail->GetUncompressedImageData();
				const int32 ImageWidth = MeshThumbnail->GetImageWidth();
				const int32 ImageHeight = MeshThumbnail->GetImageHeight();

				// Create texture
				UTexture2D* Texture = UTexture2D::CreateTransient(ImageWidth, ImageHeight, PF_B8G8R8A8);

				// Slow method, need to be refactored later somehow
				// Lock texture
				void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

				// Copy pixel data
				FMemory::Memcpy(TextureData, UncompressedImageData->GetData(), /*4 * ImageWidth * ImageHeight*/UncompressedImageData->Num());

				// Unlock texture
				Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

				// Update texture
				Texture->UpdateResource();

				UE_LOG(LogMBS, Warning, TEXT("Thumbnail texture copied"));

				return Texture;
			}
			else
			{
				UE_LOG(LogMBS, Error, TEXT("MeshThumbnail not found"));
			}
		}
	}
#endif
	return nullptr;
}

void UMBSFunctionLibrary::ReleaseMeshThumbnail(UTexture2D* InThumbnailTexture)
{
	if (InThumbnailTexture)
	{
		InThumbnailTexture->ReleaseResource();
	}
}

void UMBSFunctionLibrary::FixAllBuildSystemsTransform(const UWorld* World)
{
	TIMER;
	UE_LOG(LogMBS, Warning, TEXT("=== Fixing all build systems transforms ==="));
	int32 Count = 0;

	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] Fixing modular sections transforms of %s build system."), Count, *It->GetName());
		It->FixModularSectionsTransform();
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] %s modular section transforms has been fixed."), Count, *It->GetName());
		Count++;
	}
	UE_LOG(LogMBS, Warning, TEXT("=== All [TotalCount=%d] build systems transforms were fixed ==="), Count);
}

void UMBSFunctionLibrary::AllToBuildModeOn(const UWorld* World)
{
	check(World);
	UE_LOG(LogMBS, Warning, TEXT("Setting all build systems bBuildModeIsActivated to true."));
	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		if (!It->IsBuildModeActivated())
		{
			UE_LOG(LogMBS, Log, TEXT("%s had bBuildModeIsActivated == false"), *It->GetName());
			It->SetBuildModeActivated(true);
			UE_LOG(LogMBS, Log, TEXT("Set bBuildModeIsActivated of %s to true."), *It->GetName());
		}
		else
		{
			UE_LOG(LogMBS, Log, TEXT("bBuildModeIsActivated of %s is already true."), *It->GetName());
		}
	}
	UE_LOG(LogMBS, Warning, TEXT("End AllToBuildModeOn."));
}

void UMBSFunctionLibrary::AllToBuildModeOff(const UWorld* World)
{
	check(World);
	UE_LOG(LogMBS, Warning, TEXT("Setting all build systems bBuildModeIsActivated to false."));
	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		if (It->IsBuildModeActivated())
		{
			UE_LOG(LogMBS, Log, TEXT("%s had bBuildModeIsActivated == true"), *It->GetName());
			It->SetBuildModeActivated(false);
			UE_LOG(LogMBS, Log, TEXT("Set bBuildModeIsActivated of %s to false."), *It->GetName());
		}
		else
		{
			UE_LOG(LogMBS, Log, TEXT("bBuildModeIsActivated of %s is already false."), *It->GetName());
		}
	}
	UE_LOG(LogMBS, Warning, TEXT("End AllToBuildModeOff."));
}

void UMBSFunctionLibrary::AllToSectionReloadModeNone(const UWorld* World)
{
	check(World);
	UE_LOG(LogMBS, Warning, TEXT("Setting all build systems ReloadMode to None."));
	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		if (It->GetReloadMode() != EModularSectionReloadMode::None)
		{
			UE_LOG(LogMBS, Log, TEXT("%s had ReloadMode != EModularSectionReloadMode::None"), *It->GetName());
			It->SetReloadMode(EModularSectionReloadMode::None);
			UE_LOG(LogMBS, Log, TEXT("Set ReloadMode of %s to None."), *It->GetName());
		}
		else
		{
			UE_LOG(LogMBS, Log, TEXT("ReloadMode of %s is already None."), *It->GetName());
		}
	}
	UE_LOG(LogMBS, Warning, TEXT("End AllToSectionReloadModeNone."));
}

void UMBSFunctionLibrary::RegenerateAllBuildSystems(const UWorld* World)
{
	check(World);
	TIMER;
	UE_LOG(LogMBS, Warning, TEXT("=== Regenerating all build systems that have generators ==="));
	int32 Count = 0;

	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		if (UModularBuildSystemGenerator* Generator = Cast<UModularBuildSystemGenerator>(It->GetGenerator().GetObject()))
		{
			UE_LOG(LogMBS, Log, TEXT("[Index=%d] Regenerating %s build system."), Count, *It->GetName());
			IBuildingGeneratorInterface::Execute_Generate(Generator);
			UE_LOG(LogMBS, Log, TEXT("[Index=%d] %s has been regenerated."), Count, *It->GetName());
			Count++;
		}
	}
	UE_LOG(LogMBS, Warning, TEXT("=== All [TotalCount=%d] build systems were regenerated ==="), Count);
}

void UMBSFunctionLibrary::MergeAllBuildSystems(const UWorld* World)
{
	check(World);
	TIMER;
	UE_LOG(LogMBS, Warning, TEXT("=== Merging all build systems ==="));
	int32 Count = 0;

	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] Merging %s build system."), Count, *It->GetName());
		It->MergeIntoStaticMesh();
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] %s has been merged."), Count, *It->GetName());
		Count++;
	}
	UE_LOG(LogMBS, Warning, TEXT("=== All [TotalCount=%d] build systems were merged ==="), Count);
}

void UMBSFunctionLibrary::UnmergeAllBuildSystems(const UWorld* World)
{
	check(World);
	TIMER;
	UE_LOG(LogMBS, Warning, TEXT("=== Unmerging all build systems ==="));
	int32 Count = 0;

	for (TActorIterator<AModularBuildSystemActor> It(World); It; ++It)
	{
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] Unmerging %s build system into modular sections."), Count, *It->GetName());
		It->UnmergeIntoModularSections();
		UE_LOG(LogMBS, Log, TEXT("[Index=%d] %s has been unmerged back into modular sections."), Count, *It->GetName());
		Count++;
	}
	UE_LOG(LogMBS, Warning, TEXT("=== All [TotalCount=%d] build systems were unmerged ==="), Count);
}

FString UMBSFunctionLibrary::GetDisplayName(const UObject* InObject)
{
	return UKismetSystemLibrary::GetDisplayName(InObject);
}

FTransform UMBSFunctionLibrary::CalculateNewTransform(AModularBuildSystemActor* BuildSystem, const FModularBuildStats& BuildStats,
	int32 InIndex, const FTransform& ActorTransform, const FInitModularSectionsArgs& Args, bool& bOutShouldBeSkipped)
{
	FTransform NewTransform = Args.Initializer.GetResolution()
		->GetNextTransform(ActorTransform, InIndex, Args.Initializer.GetMaxInRow(), Args.Initializer.GetTotalCount(),
			Args.InLevelZMultiplier, BuildStats, Args.InSolver, Args.InPreviousLevelResolution);

	// Adjust calculated transform depend on shape of a level if provided
	if (Args.InShape)
	{
		// Adjust new transform
		Args.InShape->ShapeTransform(
			FMBSShapeTransformArgs(
				InIndex,
				Args.OutSkippedIndices.Num(),
				BuildStats.Bounds,
				&Args.Initializer,
				BuildSystem,
				Args.InLevelId,
				&NewTransform,
				&bOutShouldBeSkipped));
	}

	return NewTransform;
}

FBox UMBSFunctionLibrary::GetModularLevelInteriorBox(const AModularBuildSystemActor* BuildSystem, const FModularLevel* InLevel)
{
	// TODO: Currently this method returns bounds for a whole modular build system actor. Instead of that it should
	// return the bounds of a single modular level (InLevel).
	FVector Origin;
	FVector BoxExtent;
	BuildSystem->GetActorBounds(false, Origin, BoxExtent);
	return FBox::BuildAABB(Origin, BoxExtent);
}

void UMBSFunctionLibrary::FixRootComponent(AActor* Actor)
{
	USceneComponent* RootComponent = Actor->GetRootComponent();
	if (RootComponent->GetName() == DefaultRootName)
	{
		return;
	}

#if WITH_EDITOR
	LogComponents(Actor);
	UE_LOG(LogMBS, Error, TEXT("%s: Root component is not DefaultSceneRoot! [%s]"), *Actor->GetName(),
		*RootComponent->GetName());
	UE_LOG(LogMBS, Log, TEXT("%s: Preparing components to be attached to the new root."), *Actor->GetName());
#endif

	// Creating new root
	USceneComponent* NewRoot = NewObject<USceneComponent>(Actor, DefaultRootName);
	NewRoot->SetMobility(EComponentMobility::Static);

	// It is necessary to set new root transform as it will be at zero coords otherwise
	NewRoot->SetWorldTransform(RootComponent->GetComponentTransform());
	Actor->SetRootComponent(NewRoot);

	for (auto& Component : Actor->GetComponents())
	{
		if (Component == RootComponent)
		{
			continue;
		}

		if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
		{
			SceneComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

			UE_LOG(LogMBS, Log, TEXT("%s: SceneComponent [%s] was attached to the new root component [%s]"),
				*Actor->GetName(), *SceneComponent->GetName(), *RootComponent->GetName());
		}
	}

#if WITH_EDITOR
	LogComponents(Actor);
	UE_LOG(LogMBS, Warning, TEXT("%s: RootComponent has been fixed. New root component name is %s"),
		*Actor->GetName(), *RootComponent->GetName());
#endif
}

TArray<FModularLevel*> UMBSFunctionLibrary::GetAllLevelsWrapper(const TArray<const FModularLevel*>& SingleEntries,
	const TArray<FLevelsArrayEntry>& ArrayEntries)
{
	TArray<FModularLevel*> OutLevels;
	for (const FModularLevel* Level : SingleEntries)
	{
		OutLevels.Add(const_cast<FModularLevel*>(Level));
	}

	for (const FLevelsArrayEntry Entry : ArrayEntries)
	{
		for (const FModularLevel& Level : *Entry.Levels)
		{
			OutLevels.Add(const_cast<FModularLevel*>(&Level));
		}
	}
	return OutLevels;
}

FModularLevel* UMBSFunctionLibrary::GetLevelWithIdWrapper(const TArray<const FModularLevel*>& SingleEntries,
	const TArray<FLevelsArrayEntry>& ArrayEntries, int32 Id)
{
	if (FModularLevel::IsValidLevelId(Id))
	{
		for (const FModularLevel* Level : SingleEntries)
		{
			if (Level->GetId() == Id)
			{
				return const_cast<FModularLevel*>(Level);
			}
		}

		for (const FLevelsArrayEntry Entry : ArrayEntries)
		{
			for (const FModularLevel& Level : *Entry.Levels)
			{
				if (Level.GetId() == Id)
				{
					return const_cast<FModularLevel*>(&Level);
				}
			}
		}
	}
	return nullptr;
}

#if WITH_EDITOR
void UMBSFunctionLibrary::LogComponents(const AActor* Actor)
{
	for (auto& Component : Actor->GetComponents())
	{
		UE_LOG(LogMBS, Log, TEXT("%s: Component name = %s"), *Actor->GetName(), *Component->GetName());
	}
}

void UMBSFunctionLibrary::AddBillboard(AActor* Actor)
{
	//Add billboard component
	UBillboardComponent* SpriteComponent = Actor->CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> DecalTexture;
		FConstructorStatics() : DecalTexture(TEXT("/Engine/Editor/Slate/Icons/AssetIcons/Sphere_64x.Sphere_64x"))
		{
		}
	};

	// Set up sprite component
	if (SpriteComponent)
	{
		static FConstructorStatics ConstructorStatics;
		SpriteComponent->Sprite = ConstructorStatics.DecalTexture.Get();
		SpriteComponent->SetupAttachment(Actor->GetRootComponent());
		SpriteComponent->SetUsingAbsoluteScale(true);
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->bReceivesDecals = false;
		SpriteComponent->bHiddenInGame = true;
	}
}
#endif