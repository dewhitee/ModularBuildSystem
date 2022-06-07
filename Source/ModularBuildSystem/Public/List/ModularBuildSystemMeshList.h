// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularBuildSystemListBase.h"
#include "ModularBuildSystemMeshList.generated.h"

class UTexture2D;
class UStaticMesh;
class UModularSectionResolution;

USTRUCT(BlueprintType)
struct FModularBuildSystemMeshListElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TArray<UStaticMesh*> StaticMeshes;
};

/**
 * Data asset that contains meshes used for fast mesh change on modular build system actors.
 */
UCLASS(BlueprintType, Blueprintable)
class MODULARBUILDSYSTEM_API UModularBuildSystemMeshList : public UModularBuildSystemListBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModularBuildSystem")
	TMap<UModularSectionResolution*, FModularBuildSystemMeshListElement> Elements;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	int32 GetCountOfMeshesWithResolution(UModularSectionResolution* Resolution) const;

	//const FModularBuildSystemMeshListElement* GetElement(int32 AtIndex) const;
	const UStaticMesh* GetConstMesh(int32 AtIndex, const UModularSectionResolution* Resolution) const;

	UFUNCTION(BlueprintCallable, Category = "ModularBuildSystem")
	UStaticMesh* GetMesh(int32 AtIndex, UModularSectionResolution* Resolution) const;
	
	virtual int32 GetLength(UModularSectionResolution* Resolution) const override;
};
