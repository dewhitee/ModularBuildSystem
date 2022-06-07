
#include "MBSFunctionLibrary.h"
#include "MBSStretchManager.h"
#include "ModularSectionResolution.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetStretchOffset, "ModularBuildSystem.StretchManager.GetStretchOffset",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::MediumPriority)
bool FGetStretchOffset::RunTest(const FString& Parameters)
{
	TestEqual("Zero (minimum) check == 0.f", FMBSStretchManager::GetStretchOffset(0.f), 0.f);
	TestEqual("400.f (maximum) check == 1.f", FMBSStretchManager::GetStretchOffset(UModularSectionResolution::DefaultSectionSize), 1.f);
	TestEqual("200.f (middle) check == 0.5f", FMBSStretchManager::GetStretchOffset(200.f), 0.5f);
	TestEqual("-100.f (less than minimum) check == 0.f", FMBSStretchManager::GetStretchOffset(-100.f), 0.f);
	TestEqual("500.f (more than maximum) check == 1.f", FMBSStretchManager::GetStretchOffset(500.f), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCalculateScaleCoefficient, "ModularBuildSystem.StretchManager.CalculateScaleCoefficient",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::MediumPriority)
bool FCalculateScaleCoefficient::RunTest(const FString& Parameters)
{
	TestEqual("Zero (minimum) check == 1.f", FMBSStretchManager::CalculateScaleCoefficient(0.f, 0), 1.f);
	TestEqual("Zero bounds axis value check == 1.f", FMBSStretchManager::CalculateScaleCoefficient(200.f, 0), 1.f);
	TestEqual("Zero changed axis location value check == 1.f", FMBSStretchManager::CalculateScaleCoefficient(0.f, 1), 1.f);
	TestEqual("ChangedAxisLocation=200.f, BoundsAxisValue=1 == 1.5f", FMBSStretchManager::CalculateScaleCoefficient(200.f, 1), 1.5f);
	TestEqual("ChangedAxisLocation=400.f, BoundsAxisValue=1 == 1.0f", FMBSStretchManager::CalculateScaleCoefficient(400.f, 1), 1.0f);
	TestEqual("ChangedAxisLocation=400.f, BoundsAxisValue=2 == 1.0f", FMBSStretchManager::CalculateScaleCoefficient(400.f, 2), 1.0f);
	TestEqual("ChangedAxisLocation=400.f, BoundsAxisValue=4 == 1.0f", FMBSStretchManager::CalculateScaleCoefficient(400.f, 4), 1.0f);
	TestEqual("ChangedAxisLocation=200.f, BoundsAxisValue=4 == 1.125f", FMBSStretchManager::CalculateScaleCoefficient(200.f, 4), 1.125f);
	TestEqual("ChangedAxisLocation=200.f, BoundsAxisValue=4 == 1.125f", FMBSStretchManager::CalculateScaleCoefficient(200.f, 4), 1.125f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStretchSectionsUsingScaleCoefficients, "ModularBuildSystem.StretchManager.StretchSectionsUsingScaleCoefficients",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::MediumPriority)
bool FStretchSectionsUsingScaleCoefficients::RunTest(const FString& Parameters)
{
	UWorld* World = UMBSFunctionLibrary::GetTestWorld();
	UTEST_NOT_NULL("World is valid", World);
	
	// Modular sections to check
	const FVector Location0 = FVector::ZeroVector;
	const FVector Location1 = FVector(400.f, 0.f, 0.f);
	const FVector Location2 = FVector(0.f, 400.f, 0.f);
	const FVector Location3 = FVector(400.f, 400.f, 0.f);
	
	FModularSection Section0 = FModularSection(FModularLevel::InvalidLevelId, World->SpawnActor<AStaticMeshActor>(Location0, FRotator::ZeroRotator));
	FModularSection Section1 = FModularSection(FModularLevel::InvalidLevelId, World->SpawnActor<AStaticMeshActor>(Location1, FRotator::ZeroRotator));
	FModularSection Section2 = FModularSection(FModularLevel::InvalidLevelId, World->SpawnActor<AStaticMeshActor>(Location2, FRotator::ZeroRotator));
	FModularSection Section3 = FModularSection(FModularLevel::InvalidLevelId, World->SpawnActor<AStaticMeshActor>(Location3, FRotator::ZeroRotator));

	FMBSStretchManager StretchManager;
	MBS::FStretchArgs Args;
	Args.Sections = { &Section0, &Section1, &Section2, &Section3 };
	Args.Transform = FTransform::Identity;
	
	StretchManager.StretchSectionsUsingScaleCoefficients(Args, nullptr);
	TestEqual("Default check (Section 0)", Section0.GetLocation(), Location0);
	TestEqual("Default check (Section 1)", Section1.GetLocation(), Location1);
	TestEqual("Default check (Section 2)", Section2.GetLocation(), Location2);
	TestEqual("Default check (Section 2)", Section3.GetLocation(), Location3);
	
	StretchManager.SetScaleCoefficientX(1.5f);
	StretchManager.StretchSectionsUsingScaleCoefficients(Args, nullptr);
	TestEqual("Coefficients X=1.5, Y=1.0, Z=1.0, (Section 0)", Section0.GetLocation(), FVector::ZeroVector);
	TestEqual("Coefficients X=1.5, Y=1.0, Z=1.0, (Section 1)", Section1.GetLocation(), FVector(400.f, 0.f, 0.f));
	TestEqual("Coefficients X=1.5, Y=1.0, Z=1.0, (Section 2)", Section2.GetLocation(), FVector(0.f, 600.f, 0.f));
	TestEqual("Coefficients X=1.5, Y=1.0, Z=1.0, (Section 3)", Section3.GetLocation(), FVector(400.f, 600.f, 0.f));

	Section0.SetLocation(Location0);
	Section1.SetLocation(Location1);
	Section2.SetLocation(Location2);
	Section3.SetLocation(Location3);
	
	StretchManager.ResetScaleCoefficients();
	StretchManager.SetScaleCoefficientY(1.5f);
	StretchManager.StretchSectionsUsingScaleCoefficients(Args, nullptr);
	TestEqual("Coefficients X=1.0, Y=1.5, Z=1.0, (Section 0)", Section0.GetLocation(), FVector::ZeroVector);
	TestEqual("Coefficients X=1.0, Y=1.5, Z=1.0, (Section 1)", Section1.GetLocation(), FVector(600.f, 0.f, 0.f));
	TestEqual("Coefficients X=1.0, Y=1.5, Z=1.0, (Section 2)", Section2.GetLocation(), FVector(0.f, 400.f, 0.f));
	TestEqual("Coefficients X=1.0, Y=1.5, Z=1.0, (Section 3)", Section3.GetLocation(), FVector(600.f, 400.f, 0.f));
	return true;
}

