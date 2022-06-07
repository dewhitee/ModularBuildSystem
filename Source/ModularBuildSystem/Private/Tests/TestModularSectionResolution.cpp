#include "Solver/DefaultTransformSolver.h"
#include "Solver/MBSTransformSolver.h"
#include "ModularSectionResolution.h"
#include "ModularBuildStats.h"
#include "House/HouseWallTransformSolver.h"
#include "Misc/AutomationTest.h"
#include "Solver/CornerTransformSolver.h"
#include "Solver/RooftopTransformSolver.h"

namespace MBS
{

struct FExpectedResult
{
	const FString& What;
	FVector Result;
};
	
class UTestSectionResolution : public UModularSectionResolution
{
public:
	void SetResolution(FIntVector InResolution) { Resolution = InResolution; }
	void SetSnapMode(EModularSectionResolutionSnapMode InSnapMode) { SnapMode = InSnapMode; }

	static UTestSectionResolution* GetNew4x4x4(EModularSectionResolutionSnapMode SnapMode)
	{
		UTestSectionResolution* New4x4x4 = NewObject<UTestSectionResolution>();
		New4x4x4->SetResolution(FIntVector(400.f, 400.f, 400.f));
		New4x4x4->SetSnapMode(SnapMode);
		return New4x4x4;
	}

	static UTestSectionResolution* GetNew2x2x2(EModularSectionResolutionSnapMode SnapMode)
	{
		UTestSectionResolution* New2x2x2 = NewObject<UTestSectionResolution>();
		New2x2x2->SetResolution(FIntVector(200.f, 200.f, 200.f));
		New2x2x2->SetSnapMode(SnapMode);
		return New2x2x2;
	}

	static UTestSectionResolution* GetNewUndefined(EModularSectionResolutionSnapMode SnapMode)
	{
		UTestSectionResolution* NewUndefined = NewObject<UTestSectionResolution>();
		NewUndefined->SetResolution(FIntVector(UndefinedSectionSize));
		return NewUndefined;
	}

	static TArray<FNextTransformArgs> GetActualArgs(const FModularBuildStats& BuildStats, const UModularSectionResolution* Resolution, UMBSTransformSolver* Solver)
	{
		return {
			FNextTransformArgs(FTransform(), 0, 0, 0, 0.f, BuildStats, nullptr, Resolution, Solver),
			FNextTransformArgs(FTransform(), 0, 0, 5, 0.f, BuildStats, nullptr, Resolution, Solver),
			FNextTransformArgs(FTransform(), 5, 1, 2, 0.f, BuildStats, nullptr, Resolution, Solver),
			FNextTransformArgs(FTransform(), 0, 1, 2, 0.f, BuildStats, nullptr, Resolution, Solver),
			FNextTransformArgs(FTransform(), 1, 1, 2, 0.f, BuildStats, nullptr, Resolution, Solver),
		};
	}

	static TArray<FTransform> GetActual(const UModularSectionResolution* Resolution, const TArray<FNextTransformArgs>& Args)
	{
		TArray<FTransform> OutActualResults;
		for (const auto& A : Args)
		{
			OutActualResults.Add(Resolution->GetNextTransform(
				A.InTransform, A.InIndex, A.InMaxInRow, A.InMaxCount, A.InLevelZMultiplier, A.InBuildStats, A.InSolver, A.InPreviousLevelResolution));
		}
		return OutActualResults;
	}

	static TArray<FTransform> GetActual(UMBSTransformSolver* Solver, const TArray<FNextTransformArgs>& Args)
	{
		TArray<FTransform> OutActualResults;
		for (const auto& A : Args)
		{
			FNextTransformArgs Current = A;
			OutActualResults.Add(Solver->GetNextTransform(Current));
		}
		return OutActualResults;
	}
	
	static void TestMultiple(FAutomationTestBase* Test, const TArray<FTransform>& ActualFromResolution, const TArray<FTransform>& ActualFromSolver, const TArray<FExpectedResult>& Results)
	{
		for (int32 i = 0; i < Results.Num(); i++)
		{
			if (!ActualFromResolution.IsValidIndex(i) || !ActualFromSolver.IsValidIndex(i))
			{
				Test->AddError("ActualFromResolution or ActualFromSolver length is less that expected...");
				break;
			}
			Test->TestEqual(FString::Printf(TEXT("Actual%d: (Resolution) %s"), i, *Results[i].What), ActualFromResolution[i].GetLocation(), Results[i].Result);
			Test->TestEqual(FString::Printf(TEXT("Actual%d: (Solver)     %s"), i, *Results[i].What), ActualFromSolver[i].GetLocation(), Results[i].Result);
		}
	}
};
	
struct FTestData
{
	const FModularBuildStats BuildStats;
	
	TArray<FTransform> Actual4x4x4;
	TArray<FTransform> Actual2x2x2;
	TArray<FTransform> ActualUndefined;

	TArray<FTransform> Actual4x4x4_Solver;
	TArray<FTransform> Actual2x2x2_Solver;
	TArray<FTransform> ActualUndefined_Solver;
	
	FTestData(EModularSectionResolutionSnapMode SnapMode, UMBSTransformSolver* Solver, const FModularBuildStats& BuildStats)
		: BuildStats(BuildStats)
	{
		const UTestSectionResolution* Resolution4x4x4		= UTestSectionResolution::GetNew4x4x4(SnapMode);
		const UTestSectionResolution* Resolution2x2x2		= UTestSectionResolution::GetNew2x2x2(SnapMode);
		const UTestSectionResolution* ResolutionUndefined	= UTestSectionResolution::GetNewUndefined(SnapMode);
		
		const TArray<FNextTransformArgs>& ActualArgs_4x4x4		= UTestSectionResolution::GetActualArgs(BuildStats, Resolution4x4x4, nullptr);
		const TArray<FNextTransformArgs>& ActualArgs_2x2x2		= UTestSectionResolution::GetActualArgs(BuildStats, Resolution2x2x2, nullptr);
		const TArray<FNextTransformArgs>& ActualArgs_Undefined	= UTestSectionResolution::GetActualArgs(BuildStats, ResolutionUndefined, nullptr);

		// Resolution
		Actual4x4x4				= UTestSectionResolution::GetActual(Resolution4x4x4, ActualArgs_4x4x4);
		Actual2x2x2				= UTestSectionResolution::GetActual(Resolution2x2x2, ActualArgs_2x2x2);
		ActualUndefined			= UTestSectionResolution::GetActual(ResolutionUndefined, ActualArgs_Undefined);

		// Solver
		Actual4x4x4_Solver		= UTestSectionResolution::GetActual(Solver, ActualArgs_4x4x4);
		Actual2x2x2_Solver		= UTestSectionResolution::GetActual(Solver, ActualArgs_2x2x2);
		ActualUndefined_Solver	= UTestSectionResolution::GetActual(Solver, ActualArgs_Undefined);
	}
};
	
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNextDefaultTransform, "ModularBuildSystem.SectionResolution.GetNextDefaultTransform",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FGetNextDefaultTransform::RunTest(const FString& Parameters)
{
	UDefaultTransformSolver* Solver = NewObject<UDefaultTransformSolver>();
	const auto Data = MBS::FTestData(EModularSectionResolutionSnapMode::Default, Solver,
		FModularBuildStats(FIntPoint(2, 2), 4, 2, 2));
	
	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual4x4x4, Data.Actual4x4x4_Solver, {
		{ "4x4x4: location is zero", FVector::ZeroVector },
		{ "4x4x4: location is zero", FVector::ZeroVector },
		{ "4x4x4: location is FVector(0.f, 2000.f, 0.f)", FVector(0.f, 2000.f, 0.f) },
		{ "4x4x4: location is zero", FVector::ZeroVector },
		{ "4x4x4: location is FVector(0.f, 400.f, 0.f)", FVector(0.f, 400.f, 0.f) },
	});
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNextWallTransform, "ModularBuildSystem.SectionResolution.GetNextWallTransform",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FGetNextWallTransform::RunTest(const FString& Parameters)
{
	UHouseWallTransformSolver* Solver = NewObject<UHouseWallTransformSolver>();
	const auto Data = MBS::FTestData(EModularSectionResolutionSnapMode::Wall, Solver,
		FModularBuildStats(FIntPoint(2, 2), 4, 2, 2));

	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual4x4x4, Data.Actual4x4x4_Solver, {
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is FVector(0.f, 2000.f, 0.f)", FVector(800.f, 400.f, 0.f) },
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is FVector(0.f, 400.f, 0.f)", FVector(-400.f, 400.f, 0.f) },
	});
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNextCornerTransform, "ModularBuildSystem.SectionResolution.GetNextCornerTransform",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FGetNextCornerTransform::RunTest(const FString& Parameters)
{
	UCornerTransformSolver* Solver = NewObject<UCornerTransformSolver>();
	const auto Data = MBS::FTestData(EModularSectionResolutionSnapMode::Corner, Solver,
		FModularBuildStats(FIntPoint(2, 2), 4, 2, 2));
	
	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual4x4x4, Data.Actual4x4x4_Solver, {
		{ "4x4x4: location is FVector(0.f, -400.f, -400.f)", FVector(0.f, -400.f, -400.f) },
		{ "4x4x4: location is FVector(0.f, -400.f, -400.f)", FVector(0.f, -400.f, -400.f) },
		{ "4x4x4: location is FVector(0.f, -400.f, -400.f)", FVector(0.f, -400.f, -400.f) },
		{ "4x4x4: location is FVector(0.f, -400.f, -400.f)", FVector(0.f, -400.f, -400.f) },
		{ "4x4x4: location is FVector(0.f, 400.f, -400.f)", FVector(0.f, 400.f, -400.f) },
	});
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNextRoofTransform, "ModularBuildSystem.SectionResolution.GetNextRoofTransform",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FGetNextRoofTransform::RunTest(const FString& Parameters)
{
	UHouseWallTransformSolver* Solver = NewObject<UHouseWallTransformSolver>();
	const auto Data = MBS::FTestData(EModularSectionResolutionSnapMode::Roof, Solver,
		FModularBuildStats(FIntPoint(2, 2), 4, 2, 2));
	
	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual4x4x4, Data.Actual4x4x4_Solver, {
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is FVector(0.f, 2000.f, 0.f)", FVector(800.f, 400.f, 0.f) },
		{ "4x4x4: location is zero", FVector(0.f, -400.f, 0.f) },
		{ "4x4x4: location is FVector(0.f, 400.f, 0.f)", FVector(-400.f, 400.f, 0.f) },
	});
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNextRooftopTransform, "ModularBuildSystem.SectionResolution.GetNextRooftopTransform",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
bool FGetNextRooftopTransform::RunTest(const FString& Parameters)
{
	URooftopTransformSolver* Solver = NewObject<URooftopTransformSolver>();
	const auto Data = MBS::FTestData(EModularSectionResolutionSnapMode::Rooftop, Solver,
		FModularBuildStats(FIntPoint(2, 2), 4, 2, 2));
	
	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual4x4x4, Data.Actual4x4x4_Solver, {
		{"4x4x4: location is zero", FVector::ZeroVector},
		{"4x4x4: location is zero", FVector::ZeroVector},
		// TODO: Add more test cases
	});
	MBS::UTestSectionResolution::TestMultiple(this, Data.Actual2x2x2, Data.Actual2x2x2_Solver, {
		{"2x2x2: location is zero", FVector::ZeroVector},
		{"2x2x2: location is zero", FVector::ZeroVector},
		// TODO: Add more test cases
	});
	return true;
}


