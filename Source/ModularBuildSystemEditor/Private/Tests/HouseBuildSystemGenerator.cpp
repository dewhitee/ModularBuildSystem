#include "House/HouseBuildSystemGenerator.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCheckBounds, "Test.House.Generator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCheckBounds::RunTest(const FString& Parameters)
{
	UHouseBuildSystemGenerator* Generator = NewObject<UHouseBuildSystemGenerator>();
	//if (Generator->)
	return true;
}
