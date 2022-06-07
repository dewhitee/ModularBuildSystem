// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/MBSSettings.h"
#include "Config/MBSMeshConfiguration.h"
#include "ModularSectionResolution.h"

UMBSSettings::UMBSSettings()
	: DefaultMeshConfigurationType(EMBSMeshConfigurationType::Default)
	, DefaultClampMode(EMBSClampMode::Stretch)
	, DefaultExecutionMode(EMBSExecutionMode::Smart)
	, bUseSingleInstancedComponentPerUniqueMesh(false)
	, bAutoAttachVisualizationComponent(false)
	, DefaultSectionSize(FIntVector(UModularSectionResolution::DefaultSectionSize))
{
}
