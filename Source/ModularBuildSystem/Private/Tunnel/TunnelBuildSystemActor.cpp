// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel/TunnelBuildSystemActor.h"

#include "MBSFunctionLibrary.h"
#include "Tunnel/TunnelBuildSystemGenerator.h"

#include "Shape/ModularLevelShape.h"
#include "Components/SplineComponent.h"
#include "ModularBuildSystem.h"

ATunnelBuildSystemActor::ATunnelBuildSystemActor()
{
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(GetRootComponent());
}

void ATunnelBuildSystemActor::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();
		if (bGenerateOnChange && PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelBuildSystemActor, Generator))
		{
			if (Generator)
			{
				UE_LOG(LogMBS, VeryVerbose, TEXT("%s: Root component before generation = %s"),
					*GetName(), *GetRootComponent()->GetName());
				Generator->SetBuildSystemPtr(this);
				IBuildingGeneratorInterface::Execute_Generate(Generator);
			}
		}
	}
	
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void ATunnelBuildSystemActor::Init()
{
	Super::Init();

	uint8 ShiftCount = 0;
	float BasementZMultiplier = 0.f;
	LevelInitializer.InitSingleLevel(TEXT("Basement"),
		TunnelBasement,
		0,
		ShiftCount,
		0.f,
		nullptr,
		BasementZMultiplier);
	
	int32 WallsCount = 0;
	float WallsZMultiplier = 0.f;
	LevelInitializer.InitMultipleLevels(TEXT("Walls"),
		TunnelWalls,
		ShiftCount,
		0.f,
		nullptr,
		WallsZMultiplier,
		WallsCount);

	float RoofZMultiplier = 0.f;
	LevelInitializer.InitSingleLevel(TEXT("Roof"),
		TunnelRoof,
		WallsCount + 1,
		ShiftCount,
		WallsZMultiplier,
		nullptr,
		RoofZMultiplier);

	float RooftopZMultiplier = 0.f;
	LevelInitializer.InitSingleLevel(TEXT("Rooftop"),
		TunnelRooftop,
		WallsCount + 2,
		ShiftCount,
		RoofZMultiplier,
		TunnelRoof.GetInitializer().GetResolution(),
		RooftopZMultiplier);
	
	EndInit();
}

TScriptInterface<IBuildingGeneratorInterface> ATunnelBuildSystemActor::GetGenerator() const
{
	return Generator.Get();
}

TArray<FModularLevel*> ATunnelBuildSystemActor::GetAllLevels() const
{
	TArray<FModularLevel*> OutLevels;
	OutLevels.Add(const_cast<FModularLevel*>(&TunnelBasement));
	for (auto& Wall : TunnelWalls)
	{
		OutLevels.Add(const_cast<FModularLevel*>(&Wall));
	}
	OutLevels.Add(const_cast<FModularLevel*>(&TunnelRoof));
	OutLevels.Add(const_cast<FModularLevel*>(&TunnelRooftop));
	return OutLevels;
}

FModularLevel* ATunnelBuildSystemActor::GetLevelWithId(int32 Id) const
{
	return UMBSFunctionLibrary::GetLevelWithIdWrapper(
			{&TunnelBasement, &TunnelRoof, &TunnelRooftop},
			{{&TunnelWalls}},
			Id);
}

void ATunnelBuildSystemActor::CollectStats()
{
	Super::CollectStats();

	//UMBSFunctionLibrary::ForEachLevel(this, GetAllLevels(), &ATunnelBuildSystemActor::UpdateStats);

	if (Generator)
	{
		// Set build stats bounds and level count
		BuildStats.OverrideBounds(Generator->GetBounds());
		BuildStats.OverrideLevelCount(Generator->GetLevelCount());
	}
}

void ATunnelBuildSystemActor::OnUpdateTransformBounds()
{
	Super::OnUpdateTransformBounds();
	Generator->SetBounds(FIntPoint(GetTransformBounds().GetBounds().X, GetTransformBounds().GetBounds().Y));
	Generator->SetLevelCount(GetTransformBounds().GetBounds().Z);
	/*Generator->*/Generate();
}

/*void ATunnelBuildSystemActor::SetupSpline()
{
	/*if (bUseSpline && !Spline)
	{
		Spline = NewObject<USplineComponent>(this);
	}
	else if (!bUseSpline && Spline)
	{
		Spline->UnregisterComponent();
		Spline->DestroyComponent();
	}#1#
}*/
