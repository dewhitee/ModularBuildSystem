// Fill out your copyright notice in the Description page of Project Settings.


#include "House/HouseWallTransformSolver.h"

FTransform UHouseWallTransformSolver::GetNextTransform_Implementation(FNextTransformArgs& Args)
{
    // TODO: Implement Gen3 transform
    In(Args);
    
    const int32 AdjustedIndex = GetAdjustedIndex();
    constexpr float DefaultSectionSize = UModularSectionResolution::DefaultSectionSize;

    //UE_LOG(LogSectionResolution, Warning, TEXT("%s: Getting next wall transform 3: InIndex=%d, Bounds=( X=%d, Y=%d )"),
    //    *GetName(), Args.InIndex, Args.InBuildStats.Bounds.X, Args.InBuildStats.Bounds.Y);

    if (Args.InIndex < Args.InMaxInRow) // Left
    {
        if (IsFacingFront())
        {
            Args.OutRotation.Yaw += 90.f;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
        else if (IsFacingBack())
        {
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += (DefaultSectionSize * (Args.InBuildStats.Bounds.X + 1)) - DefaultSectionSize;
        }
        else if (IsFacingLeft())
        {
            Args.OutRotation.Yaw -= 180.f;
            Args.OutLocation.X += DefaultSectionSize * AdjustedIndex;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
    }
    else if (Args.InIndex >= Args.InMaxCount - Args.InMaxInRow + 2) // Right
    {
        // With Gen3 there is no Right-Front and Right-Back. Only Right.
        // We are facing the right side
        {
            // Remainder is used on small Bounds.X and Bounds.Y, when there is no middle walls
            const int32 Remainder = (Args.InMaxCount % (Args.InMaxInRow * 2));
            const int32 MiddleWallsCount = Args.InMaxCount - (Args.InMaxInRow * 2);
            const int32 ShiftMultiplier = Args.InIndex % (Args.InMaxCount - Args.InMaxInRow +
                (MiddleWallsCount > 0 || (Remainder == 0 && MiddleWallsCount == 0) ? 2 : 0));
            
            Args.OutLocation.X += (DefaultSectionSize * ShiftMultiplier);
            Args.OutLocation.Y += (DefaultSectionSize * Args.InBuildStats.Bounds.Y) - DefaultSectionSize;
        }
    }
    else // Middle
    {
        const int32 WallCurrentRow = GetWallCurrentRow(Args.InIndex, Args.InMaxInRow);

        if (IsFacingFront(GetAdjustedIndex(Args.InIndex, 2)))
        {
            Args.OutRotation.Yaw += 90.f;
            Args.OutLocation.Y += (DefaultSectionSize * WallCurrentRow) - DefaultSectionSize;
        }
        else
        {
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += (DefaultSectionSize * (Args.InMaxInRow - 1)) - DefaultSectionSize;
            Args.OutLocation.Y += (DefaultSectionSize * WallCurrentRow);
        }
    }

    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
    return Out(Args);
}
