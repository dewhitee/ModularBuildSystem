#include "ModularBuildSystemActor.h"
#include "House/HouseBuildSystemActor.h"
#include "Misc/AutomationTest.h"

namespace MBS
{
	static UWorld* GetTestWorld()
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetInstancedSectionOfLevel, "Test.Actor.GetInstancedSectionOfLevel", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGetInstancedSectionOfLevel::RunTest(const FString& Parameters)
{
	UWorld* World = MBS::GetTestWorld();
	if (TestNotNull("World is valid", World))
	{
		AModularBuildSystemActor* NewModularBuildSystemActor = World->SpawnActor<AHouseBuildSystemActor>(AHouseBuildSystemActor::StaticClass());
		TestNotNull("NewModularBuildSystemActor", NewModularBuildSystemActor);

		if (NewModularBuildSystemActor)
		{
			const FModularSectionInstanced* Instanced0 = NewModularBuildSystemActor->GetInstancedSectionOfLevel(FModularLevel::InvalidLevelId);
			TestNull("Instanced0", Instanced0);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetAdjustedBuildSystemActorTransform, "Test.Actor.GetAdjustedBuildSystemActor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGetAdjustedBuildSystemActorTransform::RunTest(const FString& Parameters)
{
	UWorld* World = MBS::GetTestWorld();
	if (TestNotNull("World is valid", World))
	{
		AModularBuildSystemActor* BS = World->SpawnActor<AModularBuildSystemActor>();
		//BS->GetAd
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInitModularSectionActor, "Test.Actor.InitModularSectionActor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FInitModularSectionActor::RunTest(const FString& Parameters)
{
	UWorld* World = MBS::GetTestWorld();
	if (TestNotNull("World is valid", World))
	{
		AModularBuildSystemActor* BS = World->SpawnActor<AModularBuildSystemActor>();
		if (BS)
		{
			FModularSectionActor Section = BS->InitModularSectionActor(
				FTransform(), FModularLevel::InvalidLevelId, nullptr, false, false);
		}
	}
	return true;
}



