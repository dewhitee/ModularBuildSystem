// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMBS, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGenerator, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSProperty, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSInterior, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSRoom, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSBounds, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSMerger, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSLSystem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSTreemap, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSStretchManager, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogInteriorGenerator, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSTextureGenerator, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSectionResolution, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogModularSection, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSSection, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogModularLevel, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogModularLevelShape, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSResolutionRecognizer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMBSIndexCalculation, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTransformSolver, Log, All);

namespace MBS
{
struct FPaths
{
	static constexpr const TCHAR* MBSDir = TEXT("/Game/MBS/");
	static constexpr const TCHAR* SavedResultsDir = TEXT("/Game/MBS/SavedResults/");
	static constexpr const TCHAR* PresetsDir = TEXT("/Game/MBS/SavedResults/Presets/");
	static constexpr const TCHAR* MergedDir = TEXT("/Game/MBS/SavedResults/Merged/");
	static constexpr const TCHAR* GameResolutionsDir = TEXT("/Game/MBS/Resolutions/");
	static constexpr const TCHAR* PluginResolutionsDir = TEXT("/ModularBuildSystem/Resolutions/");

	static TArray<FString> GetSectionResolutionDirectories()
	{
		return { GameResolutionsDir, PluginResolutionsDir };
	}
};
}

class FModularBuildSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
