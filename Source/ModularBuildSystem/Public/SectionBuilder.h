// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AModularBuildSystemActor;
struct FModularLevel;

namespace MBS
{

/**
 * Builder class for handling new section creation and spawning.
 */
class MODULARBUILDSYSTEM_API FSectionBuilder
{
	TWeakObjectPtr<AModularBuildSystemActor> BuildSystem;
	TWeakObjectPtr<UStaticMesh> MeshPtr;
	TWeakObjectPtr<UStaticMesh> OtherMeshPtr;
	
	TOptional<TSubclassOf<AActor>> Class;
	TOptional<TArray<int32>> SectionIndices;
	TOptional<TArray<FTransform>> Transforms;
	
	const FModularLevel* LevelPtr = nullptr;

	TArray<FTransform> SpawnedTransforms;
	bool bSearchInstanced;
	
public:
	FSectionBuilder(AModularBuildSystemActor* InBuildSystem, const FModularLevel* InLevel);
	~FSectionBuilder();

	FSectionBuilder& Mesh(UStaticMesh* InMesh);
	FSectionBuilder& Mesh(UStaticMesh* InMesh1, UStaticMesh* InMesh2);
	FSectionBuilder& Actor(TSubclassOf<AActor> InClass);
	
	FSectionBuilder& Of(const FModularLevel* InLevel);
	
	FSectionBuilder& At(const FTransform& InTransform);
	FSectionBuilder& At(const TArray<FTransform>& InTransforms);
	FSectionBuilder& At(const int32 InIndex);
	FSectionBuilder& At(const TArray<int32> InIndices);
	FSectionBuilder& AtInstanced(const int32 InIndex, bool bInstanced);
	FSectionBuilder& AtInstanced(const TArray<int32> InIndices, bool bInstanced);

	FSectionBuilder& MoveBy(const FVector Location);
	
	FSectionBuilder& Rotate(const FRotator Rotator);
	FSectionBuilder& RotatePitch(float DeltaPitch);
	FSectionBuilder& RotateYaw(float DeltaYaw);
	FSectionBuilder& RotateRoll(float DeltaRoll);
	
	/**
	 * Spawns a new static, instanced or actor section depending on the passed arguments.
	 * @param bRelative Do spawn relative to the BuildSystem
	 * @return Pointer to self.
	 */
	FSectionBuilder& Spawn(bool bRelative);
	FSectionBuilder& Replace();
	FSectionBuilder& Set();

	FSectionBuilder& SpawnOrReplace(bool bRelative, bool bCondition);
	FSectionBuilder& SpawnOrSet(bool bRelative, bool bCondition);
	FSectionBuilder& ReplaceOrSet(bool bCondition);
	FSectionBuilder& ReplaceIfInstancedOrSet();

	UStaticMesh* GetMesh() const;
	FTransform GetTransform() const;
	TArray<FTransform> GetTransforms() const;
};

}
