// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/MBSMeshConfiguration.h"
#include "ModularBuildSystemActor.h"

bool FMBSMeshConfiguration::IsOfInstancedType() const
{
	return Type == EMBSMeshConfigurationType::InstancedStaticMeshes ||
		Type == EMBSMeshConfigurationType::HierarchicalInstancedStaticMeshes;
}
