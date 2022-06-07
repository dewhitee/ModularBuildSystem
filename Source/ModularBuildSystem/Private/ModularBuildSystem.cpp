// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularBuildSystem.h"

#define LOCTEXT_NAMESPACE "FModularBuildSystemModule"

DEFINE_LOG_CATEGORY(LogMBS);
DEFINE_LOG_CATEGORY(LogGenerator);
DEFINE_LOG_CATEGORY(LogMBSProperty);
DEFINE_LOG_CATEGORY(LogMBSInterior);
DEFINE_LOG_CATEGORY(LogMBSRoom);
DEFINE_LOG_CATEGORY(LogMBSBounds);
DEFINE_LOG_CATEGORY(LogMBSMerger);
DEFINE_LOG_CATEGORY(LogMBSLSystem);
DEFINE_LOG_CATEGORY(LogMBSTreemap);
DEFINE_LOG_CATEGORY(LogMBSStretchManager);
DEFINE_LOG_CATEGORY(LogInteriorGenerator);
DEFINE_LOG_CATEGORY(LogMBSTextureGenerator);
DEFINE_LOG_CATEGORY(LogSectionResolution);
DEFINE_LOG_CATEGORY(LogModularLevel);
DEFINE_LOG_CATEGORY(LogModularSection);
DEFINE_LOG_CATEGORY(LogMBSSection);
DEFINE_LOG_CATEGORY(LogModularLevelShape);
DEFINE_LOG_CATEGORY(LogMBSResolutionRecognizer);
DEFINE_LOG_CATEGORY(LogMBSIndexCalculation);
DEFINE_LOG_CATEGORY(LogTransformSolver);

void FModularBuildSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FModularBuildSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModularBuildSystemModule, ModularBuildSystem)