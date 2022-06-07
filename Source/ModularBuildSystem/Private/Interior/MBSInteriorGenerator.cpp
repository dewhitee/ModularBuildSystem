// Fill out your copyright notice in the Description page of Project Settings.


#include "Interior/MBSInteriorGenerator.h"

#include "Interior/MBSInteriorPropList.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularSectionResolution.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

FGeneratedInterior UMBSInteriorGenerator::Generate_Implementation()
{
    UE_LOG(LogInteriorGenerator, Log, TEXT("%s: Generating..."), *GetName());
    return FGeneratedInterior();
}

void UMBSInteriorGenerator::Update_Implementation()
{
    UE_LOG(LogInteriorGenerator, Log, TEXT("%s: Updating..."), *GetName());
}

void UMBSInteriorGenerator::ApplyPresets_Implementation()
{
    UE_LOG(LogInteriorGenerator, Log, TEXT("%s: Applying presets..."), *GetName());
}

void UMBSInteriorGenerator::AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust,
    const UModularSectionResolution* LevelOutlineResolution) const
{
    AdjustBoxToInteriorAllowedArea(BoxToAdjust, Settings.InteriorAllowedAreaRatio, Settings.InteriorAreaOffsetTransform, LevelOutlineResolution);
}

void UMBSInteriorGenerator::AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust, float AllowedAreaRatio,
    const UModularSectionResolution* LevelOutlineResolution) const
{
    AdjustBoxToInteriorAllowedArea(BoxToAdjust, AllowedAreaRatio, Settings.InteriorAreaOffsetTransform, LevelOutlineResolution);
}

void UMBSInteriorGenerator::AdjustBoxToInteriorAllowedArea(FBox& BoxToAdjust, float AllowedAreaRatio,
    const FTransform& OffsetTransform, const UModularSectionResolution* LevelOutlineResolution)
{
    FVector ResolutionVector;
    FBox MaxAllowedInteriorBoundsBox;
    
    if (LevelOutlineResolution)
    {
        ResolutionVector = FVector(LevelOutlineResolution->GetValue());
        MaxAllowedInteriorBoundsBox = BoxToAdjust;
    }
    else
    {
        // If no resolution is provided then use defaults
        constexpr float XY = 100.f;
        ResolutionVector = FVector(XY, XY, 0.f);
        BoxToAdjust = BoxToAdjust.ExpandBy(-ResolutionVector);
        MaxAllowedInteriorBoundsBox = BoxToAdjust;
        
        ResolutionVector = BoxToAdjust.GetSize() * (1.f - AllowedAreaRatio);
        ResolutionVector.Z = 0.f;
    }
    BoxToAdjust = BoxToAdjust.ExpandBy(-ResolutionVector);
    BoxToAdjust = BoxToAdjust.ShiftBy(OffsetTransform.GetLocation());

    BoxToAdjust.Min -= OffsetTransform.GetScale3D();

    static auto AdjustMin = [](double& ToAdjust, const double CompareWith)
    {
        if (ToAdjust < CompareWith)
        {
            ToAdjust = CompareWith;
        }
    };

    AdjustMin(BoxToAdjust.Min.X, MaxAllowedInteriorBoundsBox.Min.X);
    AdjustMin(BoxToAdjust.Max.X, MaxAllowedInteriorBoundsBox.Min.X);
    
    AdjustMin(BoxToAdjust.Min.Y, MaxAllowedInteriorBoundsBox.Min.Y);
    AdjustMin(BoxToAdjust.Max.Y, MaxAllowedInteriorBoundsBox.Min.Y);
    
    BoxToAdjust.Max += OffsetTransform.GetScale3D();

    static auto AdjustMax = [](double& ToAdjust, const double CompareWith)
    {
        if (ToAdjust > CompareWith)
        {
            ToAdjust = CompareWith;
        }
    };

    AdjustMax(BoxToAdjust.Min.X, MaxAllowedInteriorBoundsBox.Max.X);
    AdjustMax(BoxToAdjust.Max.X, MaxAllowedInteriorBoundsBox.Max.X);
    
    AdjustMax(BoxToAdjust.Min.Y, MaxAllowedInteriorBoundsBox.Max.Y);
    AdjustMax(BoxToAdjust.Max.Y, MaxAllowedInteriorBoundsBox.Max.Y);
    
    // TODO: add rotation to the box. Change FBox to different box type, as it is axis-aligned and can't be rotated.
}

void UMBSInteriorGenerator::PrepareInteriorData(FMBSInteriorGeneratorModularLevelData& InInteriorData)
{
    check(InInteriorData.PropList);
    UE_LOG(LogInteriorGenerator, Log, TEXT("Preparing interior data"));
    
    for (const auto& StaticMesh : InInteriorData.PropList->StaticMeshes)
    {
        if (!InInteriorData.StaticMeshes.Contains(StaticMesh))
        {
            InInteriorData.StaticMeshes.Add(StaticMesh, FInt32Range(0, 1)/*FMBSInteriorPropCountRange()*/);
        }
    }

    for (const auto& SkeletalMesh : InInteriorData.PropList->SkeletalMeshes)
    {
        if (!InInteriorData.SkeletalMeshes.Contains(SkeletalMesh))
        {
            InInteriorData.SkeletalMeshes.Add(SkeletalMesh, FInt32Range(0, 1)/*FMBSInteriorPropCountRange()*/);
        }
    }
	
    for (const auto& Actor : InInteriorData.PropList->Actors)
    {
        if (!InInteriorData.Actors.Contains(Actor))
        {
            InInteriorData.Actors.Add(Actor, FInt32Range(0, 1)/*FMBSInteriorPropCountRange()*/);
        }
    }
}

void UMBSInteriorGenerator::ClearInteriorData(FMBSInteriorGeneratorModularLevelData& InInteriorData)
{
    InInteriorData.StaticMeshes.Reset();
    InInteriorData.SkeletalMeshes.Reset();
    InInteriorData.Actors.Reset();
}

bool UMBSInteriorGenerator::IsInteriorActorOverlapsAny(AActor* InteriorActor, const TArray<AActor*> OtherActors) const
{
    const TArray<AActor*> ActorsToIgnore = { InteriorActor };
    TArray<AActor*> OutActors;
    
    static TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

    const FBox BoxA = InteriorActor->GetComponentsBoundingBox();
    UKismetSystemLibrary::BoxOverlapActors(GetWorld(),
        BoxA.GetCenter(),
        BoxA.GetExtent(),
        ObjectTypes,
        AActor::StaticClass(),
        ActorsToIgnore,
        OutActors);

    bool bOverlapsAnyOtherActor = false;
    for (const auto& OtherActor : OtherActors)
    {
        if (OutActors.Contains(OtherActor))
        {
            bOverlapsAnyOtherActor = true;
            break;
        }
    }
    return !OutActors.IsEmpty() && bOverlapsAnyOtherActor;
}

void UMBSInteriorGenerator::AdjustInteriorActorTransform(AActor* InteriorActor, const FInteriorLevel& LevelInterior,
    const FMBSRoom& InRoom, int32 MaxTryCount, bool& bShouldSkip) const
{
    check(LevelInterior.StaticMeshActors.Num() == static_cast<TArray<AActor*>>(LevelInterior.StaticMeshActors).Num());
    check(LevelInterior.SkeletalMeshActors.Num() == static_cast<TArray<AActor*>>(LevelInterior.SkeletalMeshActors).Num());
    
    AdjustTransformIfOverlapsAny(InteriorActor, static_cast<TArray<AActor*>>(LevelInterior.StaticMeshActors), InRoom, MaxTryCount, bShouldSkip);
    AdjustTransformIfOverlapsAny(InteriorActor, static_cast<TArray<AActor*>>(LevelInterior.SkeletalMeshActors), InRoom, MaxTryCount, bShouldSkip);
    AdjustTransformIfOverlapsAny(InteriorActor, LevelInterior.Actors, InRoom, MaxTryCount, bShouldSkip);
}

void UMBSInteriorGenerator::AdjustTransformIfOverlapsAny(AActor* ActorToAdjust, const TArray<AActor*> OtherActors,
    const FMBSRoom& InRoom, int32 MaxTryCount, bool& bStillOverlaps) const
{
    const int32 MaxOverlapAdjustCount = MaxTryCount;
    int32 OverlapAdjustIndex = 0;
    
    while (IsInteriorActorOverlapsAny(ActorToAdjust, OtherActors) && OverlapAdjustIndex < MaxOverlapAdjustCount)
    {
        UE_LOG(LogInteriorGenerator, Verbose, TEXT("%s: [Try=%d] Interior actor (%s) overlaps with other actor. Adjusting transform."),
            *GetName(), OverlapAdjustIndex, *ActorToAdjust->GetName());
        ActorToAdjust->SetActorTransform(CalculateNewTransform(InRoom));
        OverlapAdjustIndex++;
    }

    if (OverlapAdjustIndex >= MaxOverlapAdjustCount)
    {
        UE_LOG(LogInteriorGenerator, Warning, TEXT("%s: After %d (Max=%d) tries interior actor (%s) still overlaps with other actor."),
            *GetName(), OverlapAdjustIndex, MaxOverlapAdjustCount, *ActorToAdjust->GetName());

        bStillOverlaps |= true;
    }
}
