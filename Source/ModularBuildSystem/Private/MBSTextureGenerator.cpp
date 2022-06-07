// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSTextureGenerator.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"

bool UMBSTextureGenerator::SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystemPtr)
{
	return (BuildSystem = InBuildSystemPtr) != nullptr;
}

TScriptInterface<IModularBuildSystemInterface> UMBSTextureGenerator::GetBuildSystemPtr() const
{
	return BuildSystem;
}

void UMBSTextureGenerator::LogGenerationSummary() const
{
	Super::LogGenerationSummary();
#if WITH_EDITOR
	UE_LOG(LogMBSTextureGenerator, Log, TEXT("%s: Generating textures for %s"),
		*GetName(), *Cast<AActor>(BuildSystem.GetObject())->GetActorLabel());
#endif
	
}
