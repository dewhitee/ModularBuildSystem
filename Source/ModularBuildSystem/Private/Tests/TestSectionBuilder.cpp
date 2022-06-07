
#include "MBSFunctionLibrary.h"
#include "ModularBuildSystemActor.h"
#include "SectionBuilder.h"
#include "House/HouseBuildSystemActor.h"
#include "House/HouseBuildSystemGenerator.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSetMesh, "ModularBuildSystem.SectionBuilder.SetMesh",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::MediumPriority)
bool FSetMesh::RunTest(const FString& Parameters)
{
	UWorld* World = UMBSFunctionLibrary::GetTestWorld();
	UTEST_NOT_NULL("World is valid", World);

	// Spawn new build system
	AHouseBuildSystemActor* BuildSystem = World->SpawnActor<AHouseBuildSystemActor>(FVector::ZeroVector, FRotator::ZeroRotator);

	// Load mesh asset
	const TSoftObjectPtr<UStaticMesh> MeshObject(FSoftObjectPath(TEXT("StaticMesh'/ModularBuildSystem/Modules/Basement/4x4x4/modular_basement_4x4x4_0_.modular_basement_4x4x4_0_'")));
	UStaticMesh* ActualMesh = MeshObject.LoadSynchronous();
	UTEST_NOT_NULL("ActualMeshClass is valid (Path=/ModularBuildSystem/Modules/Basement/4x4x4/modular_basement_4x4x4_0_.modular_basement_4x4x4_0_)", ActualMesh);
	
	MBS::FSectionBuilder Builder(BuildSystem, &BuildSystem->Basement);
	TestNull("0: Mesh is not set", Builder.GetMesh());
	
	Builder.Mesh(ActualMesh);
	TestEqual("1: Mesh is set", Builder.GetMesh(), ActualMesh);
	
	Builder.Mesh(ActualMesh, nullptr);
	TestEqual("2: Mesh is set", Builder.GetMesh(), ActualMesh);

	Builder.Mesh(nullptr, ActualMesh);
	TestEqual("3: Mesh is set", Builder.GetMesh(), ActualMesh);

	Builder.Mesh(nullptr);
	TestNotNull("4: Mesh is set", Builder.GetMesh());

	Builder.Mesh(nullptr, nullptr);
	TestNull("5: Mesh is not set", Builder.GetMesh());
	
	return true;
}
