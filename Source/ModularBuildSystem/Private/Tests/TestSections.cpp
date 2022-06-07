
#include "MBSFunctionLibrary.h"
#include "ModularLevel.h"
#include "ModularSection.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSetLocation, "ModularBuildSystem.Sections.SetLocation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FSetLocation::RunTest(const FString& Parameters)
{
	UWorld* World = UMBSFunctionLibrary::GetTestWorld();
	UTEST_NOT_NULL("World is valid", World);

	constexpr int32 LevelId = FModularLevel::InvalidLevelId;

	// Static
	FModularSection Static = FModularSection(LevelId, World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator));
	TestEqual("Static check default", Static.GetLocation(), FVector::ZeroVector);
	Static.SetLocation(FVector::OneVector);
	TestEqual("Static check", Static.GetLocation(), FVector::OneVector);

	// Actor
	FModularSectionActor Actor = FModularSectionActor(LevelId, World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator));
	TestEqual("Actor check default", Actor.GetLocation(), FVector::ZeroVector);
	Actor.SetLocation(FVector::OneVector);
	TestEqual("Actor check", Actor.GetLocation(), FVector::OneVector);

	// Instanced
	AActor* Actor_InstancedTest = World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	UInstancedStaticMeshComponent* Component = Cast<UInstancedStaticMeshComponent>(
		Actor_InstancedTest->AddComponentByClass(UInstancedStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));
	Component->AddInstance(FTransform::Identity, false);
	
	FModularSectionInstanced Instanced = FModularSectionInstanced(LevelId, Component);
	TestEqual("Instanced check default", Instanced.GetLocation(0), FVector::ZeroVector);
	
	Instanced.SetLocation(FVector::OneVector, true, 0);
	TestEqual("Instanced check", Instanced.GetLocation(0), FVector::OneVector);

	Static.Reset();
	Actor.Reset();
	Instanced.Reset();
	
	return true;
}
