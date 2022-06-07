// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularSection.h"

class IModularBuildSystemInterface;
class AModularBuildSystemActor;
class UModularSectionResolution;
struct FModularLevel;

namespace MBS
{

/**
 * 
 */
class MODULARBUILDSYSTEM_API FModularLevelInitializer
{
	TWeakInterfacePtr<IModularBuildSystemInterface> BuildSystem;
	
	/**
	 * Identifier of a last initialized modular level.
	 */
	int32 LastLevelId;
	
public:
	FModularLevelInitializer();
	FModularLevelInitializer(TWeakInterfacePtr<IModularBuildSystemInterface> InBuildSystem);
	~FModularLevelInitializer();

	/**
 	 * Initializes single ModularLevel
 	 * @param InLevelName Debug-only level name.
 	 * @param InLevel Reference to the modular level that should be initialized.
 	 * @param InLevelIndex Index of an initialized modular level (the floor that this level is placed).
 	 * @param InShiftCount Count of levels that were skipped before initialization of this modular level.
 	 * @param InZMultiplier Height (Z-axis) value multiplier of this modular level.
 	 * @param InPreviousLevelResolution Height (Z-axis) value multiplier of a modular level before (under) this modular level.
 	 * @param OutZMultiplier Calculated new height (Z-axis) value multiplier that should be used in initialization of a next floor modular level.
 	 * @param InAppendFunction Will be used if provided. If not - AModularBuildSystem::AppendSections method will be.
 	 * used instead.
 	 */
	void InitSingleLevel(const FName& InLevelName, FModularLevel& InLevel, int32 InLevelIndex,
		uint8& InShiftCount, float InZMultiplier, UModularSectionResolution* InPreviousLevelResolution,
		float& OutZMultiplier, TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction = nullptr);
	
	/**
	 * Initializes multiple ModularLevels.
	 * @param InLevelName Debug-only level name.
	 * @param InLevels Reference to the modular level that should be initialized. 
	 * @param InShiftCount Count of levels that were skipped before initialization of this modular level. 
	 * @param InZMultiplier Height (Z-axis) value multiplier of this modular level.
	 * @param InPreviousLevelResolution Height (Z-axis) value multiplier of a modular level before (under) this modular level.
	 * @param OutZMultiplier Calculated new height (Z-axis) value multiplier that should be used in initialization of a next floor modular level. 
	 * @param OutCount Count of a modular levels that should be initialized.
	 * All initialized levels will be placed upon each other depending on a Z-axis multiplier value (ZMultiplier).
	 * @param InAppendFunction Will be used if provided. If not - AModularBuildSystem::AppendSections method will be.
	 * used instead.
	 */
	void InitMultipleLevels(const FName& InLevelName, TArray<FModularLevel>& InLevels, uint8& InShiftCount,
		float InZMultiplier, UModularSectionResolution* InPreviousLevelResolution, float& OutZMultiplier,
		int32& OutCount, TFunction<void(const FInitModularSectionsArgs&)> InAppendFunction = nullptr);

	/**
 	 * Use in case modular level is already initialized but has invalid LevelId.
 	 * @param InLevel Level to fix.
 	 */
	void FixLevelId(FModularLevel* InLevel);

	void IncrementLastLevelId() { LastLevelId++; }
	int32 GetLastLevelId() const { return LastLevelId; }
	
private:	
	void PostInitSingleLevel(const FName& InLevelName, const FModularLevel& InLevel, const uint8 InShiftCount,
		float InZMultiplier, float& OutZMultiplier, const bool bSuccess) const;

};

}
