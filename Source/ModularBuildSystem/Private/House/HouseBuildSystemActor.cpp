// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HouseBuildSystemActor.h"
#include "House/HouseBuildSystemPreset.h"
#include "House/HouseBuildSystemPresetList.h"
#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"
#include "Engine/StaticMeshActor.h"
#include "AssetRegistryModule.h"
#include "MBSBounds.h"
#include "MBSFunctionLibrary.h"
#include "Shape/ModularLevelShape.h"
#include "House/HouseBuildSystemGenerator.h"
#include "House/HousePresetManager.h"
#include "Interior/MBSInterior.h"

AHouseBuildSystemActor::AHouseBuildSystemActor()
{
	bGenerateOnChange = true;
	PresetManager = CreateDefaultSubobject<UHousePresetManager>(TEXT("PresetManager"));
}

#if WITH_EDITOR
void AHouseBuildSystemActor::SelectBasement()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting basement sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Basement);
}

void AHouseBuildSystemActor::SelectFloors()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting floors sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Floors);
}

void AHouseBuildSystemActor::SelectWalls()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting walls sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Walls);
}

void AHouseBuildSystemActor::SelectCorners()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting corners sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Corners);
}

void AHouseBuildSystemActor::SelectRoof()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting roof sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Roof);
}

void AHouseBuildSystemActor::SelectRooftop()
{
	UE_LOG(LogMBS, Log, TEXT("%s: Selecting rooftop sections."), *GetActorLabel());
	UnselectAll();
	SelectSections(Rooftop);
}

void AHouseBuildSystemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (CanReload(Transform) && !IsBuildModeActivated() && Generator == nullptr)
	{
		UE_LOG(LogMBS, Log, TEXT("%s: OnConstruction!"), *GetName());
		switch (GetReloadMode())
		{
			case EModularSectionReloadMode::All:
			{
				ResetBuildSystem();
				Init();
				break;
			}
			case EModularSectionReloadMode::CurrentLevelOnly:
			{
				ResetCurrentModifiedLevel();
				Init();
				break;
			}
			case EModularSectionReloadMode::None:
			{
				break;
			}
		}
	}
}

void AHouseBuildSystemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AHouseBuildSystemActor::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();

		if (bGenerateOnChange && PropertyName == GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Generator))
		{
			if (Generator)
			{
				UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Root component before generation = %s"), *GetName(),
					*GetRootComponent()->GetName());
				
				// TODO: Test. Maybe move to the other place?
				TransformBounds.SetBounds(FIntVector(Generator->Bounds.X, Generator->Bounds.Y, Generator->LevelCount),
					StretchManager.GetScaleCoefficientsSwappedXY());
				
				Generator->SetBuildSystemPtr(this);
				if (IBuildingGeneratorInterface::Execute_Generate(Generator).Succeeded())
				{
					ApplyStretch();
				}

				// TODO: Test. New
				if (GetInterior() && GetInterior()->ShouldRegenerateOnBuildSystemUpdate())
				{
					GetInterior()->GenerateInterior(this);
				}

				// NEW. Is used to properly refresh Generator object details after CustomLevelShape is set.
				// Currently there is a bug that prevents showing the custom level shape properties until it is set again manually.
				const auto ExactProperty = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
				if (ExactProperty->GetFName() == "CustomLevelShape")
				{
					FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
					PropertyEditorModule.NotifyCustomizationModuleChanged();
				}
			}
		}

		const auto StructProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
		const FName StructName = StructProperty->GetFName();

		if (StructName == GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, HouseDisplaySettings))
		{
			UpdateHouseDisplay();
		}

		const int32 ModifiedIndex = PropertyChangedEvent.GetArrayIndex(StructName.ToString());
		const bool bWasLevelUpdate = LevelObserver.OnLevelsUpdate(StructName,
		{ 
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Basement), &Basement },
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Roof), &Roof },
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Rooftop), &Rooftop },
		}, 
		{
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Floors), &Floors },
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Walls), &Walls },
			{ GET_MEMBER_NAME_CHECKED(AHouseBuildSystemActor, Corners), &Corners },
		}, ModifiedIndex,
		[this](FModularLevel& Level)
		{
			UpdateMeshes(Level);
		},
		[this](TArray<FModularLevel>& Levels)
		{
			UpdateMeshes(Levels);
		});
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

void AHouseBuildSystemActor::Init()
{
	Super::Init();
	
	uint8 ShiftCount = 0;

	// Basement
	float BasementZMultiplier;
	LevelInitializer.InitSingleLevel(TEXT("Basement"),
		Basement,
		0,
		ShiftCount,
		0.f,
		nullptr,
		BasementZMultiplier);
	
	// Floors
	// for ex if basement next z multiplier returns 1.f - then first floor and first wall levels will be at DefaultSectionSize * 1.f from the basement pivot
	// if 0.5f - the they will be at DefaultSectionSize * 0.5f from the basement pivot
	int32 FloorsCount;
	float FloorsZMultiplier;
	LevelInitializer.InitMultipleLevels(TEXT("Floors"),
		Floors, 
		ShiftCount,
		BasementZMultiplier,
		nullptr,
		FloorsZMultiplier,
		FloorsCount);

	// Walls
	int32 WallsCount;
	float WallsZMultiplier;
	LevelInitializer.InitMultipleLevels(TEXT("Walls"),
		Walls,
		ShiftCount,
		BasementZMultiplier,
		nullptr,
		WallsZMultiplier,
		WallsCount);

	// Corners
	int32 CornersCount;
	float CornersZMultiplier;
	LevelInitializer.InitMultipleLevels(TEXT("Corners"),
		Corners,
		ShiftCount,
		BasementZMultiplier,
		nullptr,
		CornersZMultiplier,
		CornersCount);

	// Roof
	float RoofZMultiplier;
	LevelInitializer.InitSingleLevel(TEXT("Roof"),
		Roof,
		FMath::Max(FloorsCount, WallsCount) + 1,
		ShiftCount,
		FMath::Max(FloorsZMultiplier, WallsZMultiplier),
		nullptr,
		RoofZMultiplier);

	float RooftopZMultiplier;
	LevelInitializer.InitSingleLevel(TEXT("Rooftop"),
		Rooftop,
		FMath::Max(FloorsCount, WallsCount) + 2,
		ShiftCount,
		RoofZMultiplier,
		Roof.GetInitializer().GetResolution(),
		RooftopZMultiplier);
	
	EndInit();
}

TScriptInterface<IBuildingGeneratorInterface> AHouseBuildSystemActor::GetGenerator() const
{
	return Generator.Get();
}

void AHouseBuildSystemActor::CollectStats()
{
	Super::CollectStats();

	if (Generator)
	{
		// Set build stats bounds and level count
		BuildStats.OverrideBounds(Generator->Bounds);
		BuildStats.OverrideLevelCount(Generator->LevelCount);
	}
}

void AHouseBuildSystemActor::OnUpdateTransformBounds()
{
	Super::OnUpdateTransformBounds();
	UE_LOG(LogMBS, Verbose, TEXT("%s: OnUpdateTransformBounds"), *GetName());
	const FIntVector Bounds = GetTransformBounds().GetBounds();
	Generator->Bounds = FIntPoint(Bounds.X, Bounds.Y);
	Generator->LevelCount = Bounds.Z;

	// TODO: This should be handled in a more appropriate place, maybe?
	if (IsMerged())
	{
		return;
	}
	
	if (GetSpawnConfiguration().ClampMode == EMBSClampMode::Stretch)
	{
		OnBoundsTransformChange();
	}
	else
	{
		IBuildingGeneratorInterface::Execute_Generate(Generator);
	}
}

void AHouseBuildSystemActor::OnBoundsTransformChange()
{
	Super::OnBoundsTransformChange();
	UE_LOG(LogMBS, Verbose, TEXT("%s: OnBoundsTransformChange"), *GetName());

	// TODO: This should be handled in a more appropriate place, maybe?
	if (IsMerged())
	{
		return;
	}
	
	// TODO: Optimize and refactor
	if (GetSpawnConfiguration().ClampMode == EMBSClampMode::Stretch)
	{
		const FName InitialTransformName = GetTransformBounds().GetUpdatedTransformName();
		const FVector InitialTransformLocation = GetTransformBounds().GetUpdatedTransformLocation();
		UE_LOG(LogMBS, Verbose, TEXT("%s: bWasUpdated=%d, InitialTransformName=%s, InitialTransformLocation=%s"),
			*GetName(),
			static_cast<int32>(GetTransformBounds().WasUpdated()),
			*InitialTransformName.ToString(),
			*InitialTransformLocation.ToCompactString());
		// TODO: Test this!! Was just Generator->Generate() before
		if (IBuildingGeneratorInterface::Execute_Generate(Generator).Succeeded())
		{
			ApplyStretch();
		}
	}

	if (GetInterior() && GetInterior()->ShouldRegenerateOnBuildSystemUpdate())
	{
		GetInterior()->GenerateInterior(this);
	}
}

FModularLevel* AHouseBuildSystemActor::GetLevelWithId(int32 Id) const
{
	return UMBSFunctionLibrary::GetLevelWithIdWrapper(
		{&Basement, &Roof, &Rooftop},
		{&Floors, &Walls, &Corners},
		Id);
}

TArray<FModularLevel*> AHouseBuildSystemActor::GetAllLevels() const
{
	return UMBSFunctionLibrary::GetAllLevelsWrapper(
		{&Basement, &Roof, &Rooftop},
		{&Floors, &Walls, &Corners});
}

TScriptInterface<IMBSPresetManagerInterface> AHouseBuildSystemActor::GetPresetManager() const
{
	return PresetManager.Get();
}

void AHouseBuildSystemActor::ApplyStretch()
{
	const FName ChangedTransformName = GetTransformBounds().GetUpdatedTransformName();
	const FVector ChangedTransformLocation = GetTransformBounds().GetUpdatedTransformLocation();
	UE_LOG(LogMBS, Verbose, TEXT("%s: ChangedTransformName=%s, ChangedTransformLocation=%s"), *GetName(),
		*ChangedTransformName.ToString(), *ChangedTransformLocation.ToCompactString());
	
	auto CalculateScaleCoefficient = [&](const float ChangedLocationAxis, const int32 BoundsAxisValue) -> float
	{
		const float ScaleCoefficient = FMBSStretchManager::CalculateScaleCoefficient(ChangedLocationAxis, BoundsAxisValue);
		UE_LOG(LogMBS, Log, TEXT("%s: ScaleCoefficient=%.2f"), *GetName(), ScaleCoefficient);
		return ScaleCoefficient;
	};
	
	// Adjust all sections from the sides parallel to the updated transform
	// TODO: Stretch in all sides (X, Y, Z) - as StretchManager has ScaleCoefficients vector now
	// so it is necessary to scale in all sides independently from modified transform handle.
	if (ChangedTransformName == GetTransformBounds().GetTransforms().RightLocationName()
		|| ChangedTransformName == GetTransformBounds().GetTransforms().BackLocationName()
		|| ChangedTransformName == GetTransformBounds().GetTransforms().TopLocationName()
		|| GetTransformBounds().WasUpdated())
	{
		// TODO: Move to StretchManager's method
		//StretchManager.UpdateScaleCoefficients(ChangedTransformLocation, GetTransformBounds().Bounds);
		if (ChangedTransformName == GetTransformBounds().GetTransforms().RightLocationName())
		{
			StretchManager.SetScaleCoefficientX(
				CalculateScaleCoefficient(ChangedTransformLocation.Y, GetTransformBounds().GetBounds().Y));	
		}
		else if (ChangedTransformName == GetTransformBounds().GetTransforms().BackLocationName())
		{
			StretchManager.SetScaleCoefficientY(
				CalculateScaleCoefficient(ChangedTransformLocation.X, GetTransformBounds().GetBounds().X));	
		}
		else if (ChangedTransformName == GetTransformBounds().GetTransforms().TopLocationName())
		{
			StretchManager.SetScaleCoefficientZ(
				CalculateScaleCoefficient(ChangedTransformLocation.Z, GetTransformBounds().GetBounds().Z + 2));	
		}

		const MBS::FStretchArgs StretchArgs(
			GetAllSections(),
			GetActorTransform(),
			false,
			Corners,
			{Basement, Rooftop});
		
		StretchManager.StretchSectionsUsingScaleCoefficients(StretchArgs,
			[&](const MBS::FStretchSingleSectionArgs& Args)
			{
				const FVector CurrentScale = Args.Section->GetTransform().GetScale3D();
				if (Args.Section->IsInLevel({Basement, Rooftop}))
				{
					// TODO: Continue here
					FVector NewScale = FVector::OneVector;
					if (/*Args.bIsInSameDirection && */Args.CurrentDirectionAxis == EAxis::X)
					{
						NewScale = FVector(CurrentScale.X, Args.LocationMultiplier.Y, CurrentScale.Z);
						UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: -- %s -- Args.CurrentDirectionAxis=%s, CurrentScale=%s, Args.LocationMultiplier=%s, NewScale=%s"),
							*GetName(),
							*UEnum::GetValueAsString(Args.CurrentDirectionAxis),
							*Args.Section->GetName(),
							*CurrentScale.ToCompactString(),
							*Args.LocationMultiplier.ToCompactString(),
							*NewScale.ToCompactString());
						Args.Section->Scale(NewScale);
					}
					else if (/*!Args.bIsInSameDirection && */Args.CurrentDirectionAxis == EAxis::Y)
					{
						NewScale = FVector(Args.LocationMultiplier.X, CurrentScale.Y, CurrentScale.Z);
						UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: -- %s -- Args.CurrentDirectionAxis=%s, CurrentScale=%s, Args.LocationMultiplier=%s, NewScale=%s"),
							*GetName(),
							*UEnum::GetValueAsString(Args.CurrentDirectionAxis),
							*Args.Section->GetName(),
							*CurrentScale.ToCompactString(),
							*Args.LocationMultiplier.ToCompactString(),
							*NewScale.ToCompactString());
						Args.Section->Scale(NewScale);
					}
					// TODO: Change axis from None to Z when CorrectAxisDependent will be refactored
					else if (Args.CurrentDirectionAxis == EAxis::None)
					{
						NewScale = FVector(CurrentScale.X, CurrentScale.Y, Args.LocationMultiplier.Z);
						Args.Section->Scale(NewScale);
					}
				}

				if ((Args.Section->IsInLevel(Walls) || Args.Section->IsInLevel(Roof))
					&& !Args.bIsInSameDirection
					&& Args.CurrentDirectionAxis == EAxis::Y)
				{
					Args.Section->Scale(Args.LocationMultiplier);
				}
				else if (Args.Section->IsInLevel(Roof) && Args.bIsInSameDirection && Args.CurrentDirectionAxis == EAxis::X)
				{
					Args.Section->Scale(Args.LocationMultiplier);
				}

				// TODO: Change axis from None to Z when CorrectAxisDependent will be refactored
				if ((Args.Section->IsInLevel(Walls) || Args.Section->IsInLevel(Roof)) && Args.CurrentDirectionAxis == EAxis::None)
				{
					UE_LOG(LogGenerator, VeryVerbose, TEXT("%s: -- %s -- CurrentScale=%s, Args.LocationMultiplier=%s"),
						*GetName(),
						*Args.Section->GetName(),
						*CurrentScale.ToCompactString(),
						*Args.LocationMultiplier.ToCompactString());
					Args.Section->Scale(FVector(CurrentScale.X, CurrentScale.Y, Args.LocationMultiplier.Z));
				}
			});
		//return;
	}
}

void AHouseBuildSystemActor::UpdateHouseDisplay() const
{
	SetLevelVisibility(Basement, HouseDisplaySettings.bShowBasement);
	SetLevelVisibility(Floors, HouseDisplaySettings.bShowFloors);
	SetLevelVisibility(Walls, HouseDisplaySettings.bShowWalls);
	SetLevelVisibility(Corners, HouseDisplaySettings.bShowCorners);
	SetLevelVisibility(Roof, HouseDisplaySettings.bShowRoof);
	SetLevelVisibility(Rooftop, HouseDisplaySettings.bShowRooftop);
}
