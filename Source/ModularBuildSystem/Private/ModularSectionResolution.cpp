// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularSectionResolution.h"
#include "ModularBuildSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Solver/MBSTransformSolver.h"
#include "ModularLevel.h"

FNextTransformArgs::FNextTransformArgs()
{
}

FNextTransformArgs::FNextTransformArgs(const FTransform& Transform, int32 Index, int32 MaxInRow, int32 MaxCount, float LevelZMultiplier,
        const FModularBuildStats& BuildStats, const UModularSectionResolution* PreviousLevelResolution,
        const UModularSectionResolution* Resolution, UMBSTransformSolver* Solver)
    : InTransform(Transform)
    , InIndex(Index)
    , InMaxInRow(MaxInRow != 0.f ? MaxInRow : 1)
    , InMaxCount(MaxCount)
    , InLevelZMultiplier(LevelZMultiplier)
    , InBuildStats(BuildStats)
    , InPreviousLevelResolution(PreviousLevelResolution)
    , InResolution(Resolution)
    , InSolver(Solver)
    , OutLocation(InTransform.GetLocation())
    , OutRotation(InTransform.GetRotation().Rotator())
    , OutScale(InTransform.GetScale3D())
{
}

FNextTransformArgs::FNextTransformArgs(const FTransform& Transform, int32 Index, const FModularLevel& Level,
        const FModularBuildStats& BuildStats, const UModularSectionResolution* PreviousLevelResolution,
        USplineComponent* Spline)
    : InTransform(Transform)
    , InIndex(Index)
    , InMaxInRow(Level.GetInitializer().GetMaxInRow() != 0.f ? Level.GetInitializer().GetMaxInRow() : 1)
    , InMaxCount(Level.GetInitializer().GetTotalCount())
    , InLevelZMultiplier(Level.GetZMultiplier())
    , InBuildStats(BuildStats)
    , InPreviousLevelResolution(PreviousLevelResolution)
    , InResolution(Level.GetInitializer().GetResolution())
    , InSolver(Level.GetSolver())
    , InSpline(Spline)
    , OutLocation(InTransform.GetLocation())
    , OutRotation(InTransform.GetRotation().Rotator())
    , OutScale(InTransform.GetScale3D())
{
}

FTransform UModularSectionResolution::GetNextTransform(const FTransform& InTransform, int32 InIndex, int32 MaxInRow,
    int32 InMaxCount, float InLevelZMultiplier, const FModularBuildStats& InStats, UMBSTransformSolver* InSolver, 
    const UModularSectionResolution* InPreviousLevelResolution) const
{
    if (MaxInRow == 0)
    {
        UE_LOG(LogMBS, Verbose, TEXT("%s: MaxInRow value was zero. Adjusting it to be 1"), *GetName());
        MaxInRow = 1;
    }

    // Set initial location as the InTransform location
    // From now - they are set inside FNextTransformArgs ctor
    //const FVector OutLocation = InTransform.GetLocation();
    //const FRotator OutRotation = InTransform.GetRotation().Rotator();

    FNextTransformArgs Args = FNextTransformArgs(
        InTransform, 
        InIndex, 
        MaxInRow, 
        InMaxCount, 
        InLevelZMultiplier,
        InStats,
        InPreviousLevelResolution,
        this,
        InSolver);

    if (InSolver)
    {
        InSolver->GetNextTransform(Args);
        return FTransform(Args.OutRotation, Args.OutLocation);
    }

    switch (SnapMode)
    {
        case EModularSectionResolutionSnapMode::Default:
        {
            GetNextDefaultTransform(Args);
            break;
        }
        case EModularSectionResolutionSnapMode::Wall:
        {
            if constexpr (PlacementGeneration == EMBSResolutionAlgorithmGeneration::Gen2)
            {
                GetNextWallTransform_Gen2(Args);
            }
            else
            {
                GetNextWallTransform(Args);
            }
            break;
        }
        case EModularSectionResolutionSnapMode::Roof:
        {
            if constexpr (PlacementGeneration == EMBSResolutionAlgorithmGeneration::Gen2)
            {
                GetNextRoofTransform_Gen2(Args);
            }
            else
            {
                GetNextRoofTransform(Args);
            }
            break;
        }
        case EModularSectionResolutionSnapMode::Rooftop:
        {
            GetNextRooftopTransform(Args);
            break;
        }
        case EModularSectionResolutionSnapMode::Corner:
        {
            GetNextCornerTransform(Args);
            break;
        }
    }

    return FTransform(Args.OutRotation, Args.OutLocation, Args.OutScale);
}

void UModularSectionResolution::GetNextDefaultTransform(FNextTransformArgs& Args) const
{
    // Default location handling
    Args.OutLocation.X += Resolution.X * GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);       // Updates on every iteration
    Args.OutLocation.Y += Resolution.Y * GetCurrentRow(Args.InIndex, Args.InMaxInRow);          // Updates on every row shift
    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;                         // Updates on each level (floor) shift
}

void UModularSectionResolution::GetNextWallTransform_DEPRECATED(FNextTransformArgs& Args) const
{
    // TODO: This (and similar - NextRoof) method needs to be refactored. 
    // - Currently there is overlapped transform on the back of a house that is need to be distributed properly.
    // - Count of walls that is required at this moment is higher than actually needed to be, in some cases. 
    // Example: for a building with Perimeter=14 (3x4) this method do not handle 14 walls properly - it starts to work from 17 walls only.
    // - This method works properly only in some specific conditions.

    checkf(false, TEXT("GetNextWallTransform_DEPRECATED method is deprecated and should not be used anymore. Use GetNextWallTransform instead."));

    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);
    const int32 CurrentRow = GetCurrentRow(Args.InIndex, Args.InMaxInRow);

    if (IsFacingFront(AdjustedIndex)) // Front of a house
    {
        UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall IsFacingFront."), *GetName());
    
        // Wall will be facing the front direction of a house
        Args.OutRotation.Yaw += 90.f;
        Args.OutLocation.Y += (DefaultSectionSize * CurrentRow) - DefaultSectionSize;
    }
    else if (IsFacingBack(AdjustedIndex, Args.InMaxInRow)) // Back of a house
    {
        UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall IsFacingBack."), *GetName());
    
        // Wall will be facing the back direction of a house
        Args.OutRotation.Yaw -= 90.f;
        Args.OutLocation.X += (DefaultSectionSize * AdjustedIndex) - DefaultSectionSize;
        Args.OutLocation.Y += (DefaultSectionSize * CurrentRow);
    }
    else // Handling walls that are facing sides (with max length of sides defined by MaxInRow variable)
    {
        if (IsFacingLeft(CurrentRow)) // We are facing the left side
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall IsFacingLeft."), *GetName());
    
            // Mirroring the rotation of a section to fit it with an outer border of a basement/floor
            Args.OutRotation.Yaw -= 180.f;
            Args.OutLocation.X += DefaultSectionSize * AdjustedIndex;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
        else if (IsFacingRight(CurrentRow, Args.InMaxCount, Args.InMaxInRow)) // We are facing the right side
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall IsFacingRight."), *GetName());
            Args.OutLocation.X += (DefaultSectionSize * AdjustedIndex) - DefaultSectionSize;
    
            ///uint8 RemainderShift = (bool)(Args.InMaxCount % Args.InMaxInRow);
            ///Args.OutLocation.Y += (DefaultSectionSize * CurrentRow) + (DefaultSectionSize * RemainderShift);        // here Y need to be adjusted
            Args.OutLocation.Y += (DefaultSectionSize * /*GetWallMaxRow(Args.InMaxCount, Args.InMaxInRow)*/Args.InBuildStats.Bounds.Y) - DefaultSectionSize;
        }
        else // We are somewhere in the middle. Suppose to be remainder of a division (CurrentRow == MaxRow) that goes to the Back side of a house
        {
            if (IsFacingBack(GetAdjustedIndex(AdjustedIndex, 2), 2)) // AdjustedIndex == 1
            {
                Args.OutRotation.Yaw -= 90.f;
                Args.OutLocation.X += (DefaultSectionSize * (Args.InMaxInRow - 1)) - DefaultSectionSize;
                ///Args.OutLocation.Y += (DefaultSectionSize * GetWallCurrentRow(Args.InMaxCount, Args.InMaxInRow));
    
                //const uint8 RemainderShift = (bool)(Args.InMaxCount % Args.InMaxInRow);
                const uint8 Remainder = Args.InMaxCount - Args.InIndex;
                ///Args.OutLocation.Y += (DefaultSectionSize * GetWallCurrentRow(AdjustedIndex, Args.InMaxInRow, Args.InMaxCount));
    
                const int32 BoundsDifference = Args.InBuildStats.Bounds.Y - Args.InBuildStats.Bounds.X;
                const int32 WallCurrentRow = FMath::Clamp(GetWallCurrentRow(Args.InIndex, Args.InMaxInRow), 0, Args.InBuildStats.Bounds.Y);
                UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: WallCurrentRow = %d"), *GetName(), WallCurrentRow);
    
                Args.OutLocation.Y += (DefaultSectionSize * WallCurrentRow - DefaultSectionSize)/* + DefaultSectionSize * BoundsDifference*/;
    
                //Args.OutLocation.Y += (DefaultSectionSize * Remainder) - DefaultSectionSize;
                UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall (middle) IsFacingBack (Remainder=%d, InIndex=%d)"),
                    *GetName(), Remainder, Args.InIndex);
            }
            else
            {
                UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Wall (middle) IsFacingFront (InIndex=%d)"),
                    *GetName(), Args.InIndex);
    
                Args.OutRotation.Yaw += 90.f;
                Args.OutLocation.Y += (DefaultSectionSize * CurrentRow);
            }
    
            // This is not generic - in some cases this will not work. Needs to be refactored
            ///Args.OutLocation.X += (DefaultSectionSize * (Args.InMaxInRow - 2));
            ///Args.OutLocation.Y += (DefaultSectionSize * CurrentRow);
        }
    }

    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
}

void UModularSectionResolution::GetNextWallTransform_Gen2(FNextTransformArgs& Args) const
{
    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);

    if (Args.InIndex < Args.InMaxInRow) // Left
    {
        if (IsFacingFront(AdjustedIndex))
        {
            Args.OutRotation.Yaw += 90.f;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
        else if (IsFacingBack(AdjustedIndex, Args.InMaxInRow))
        {
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += (DefaultSectionSize * (Args.InBuildStats.Bounds.X + 1)) - DefaultSectionSize;
        }
        else if (IsFacingLeft(GetCurrentRow(Args.InIndex, Args.InMaxInRow)))
        {
            Args.OutRotation.Yaw -= 180.f;
            Args.OutLocation.X += DefaultSectionSize * AdjustedIndex;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
    }
    else if (Args.InIndex >= Args.InMaxCount - Args.InMaxInRow) // Right
    {
        const int32 Remainder = (Args.InMaxCount % (Args.InMaxInRow * 2));
        const int32 RightAdjustedIndex = (AdjustedIndex + Remainder) % Args.InMaxInRow;

        // FixSectionCount = 6 - ((6 - 2) + 1) = 1
        const int32 FixSectionCount = Args.InMaxCount - ((Args.InMaxCount - Args.InBuildStats.Bounds.X) + 1);

        // TODO: New check for Y=1
        if (Args.InBuildStats.Bounds.Y == 1 && Args.InMaxCount - Args.InIndex <= FixSectionCount)
        {
            UE_LOG(LogMBS, VeryVerbose, TEXT("%s: InMaxCount=%d, InIndex=%d, FixSectionCount=%d, Bounds.X=%d, RightAdjustedIndex=%d"
            ", AdjustedIndex=%d, Remainder=%d"), 
                *GetName(), Args.InMaxCount, Args.InIndex, FixSectionCount, Args.InBuildStats.Bounds.X, RightAdjustedIndex, AdjustedIndex, Remainder);
            UE_LOG(LogMBS, VeryVerbose, TEXT("%s: %d - %d <= %d"), *GetName(), Args.InMaxCount, Args.InIndex, FixSectionCount);

            UE_LOG(LogMBS, VeryVerbose, TEXT("%s: TRUE"), *GetName());
            // We are facing the right side
            Args.OutLocation.X += (DefaultSectionSize * /*RightAdjustedIndex*/AdjustedIndex) - DefaultSectionSize;
            Args.OutLocation.Y += (DefaultSectionSize * Args.InBuildStats.Bounds.Y) - DefaultSectionSize;
        }
        else if (IsFacingFront(RightAdjustedIndex))
        {
            // TODO: Fix bug
            // In case of Bounds.X = 2 and Bounds.Y = 1
            // Index=7 goes here, but need to be at IsFacingRight
            Args.OutRotation.Yaw += 90.f;
            Args.OutLocation.Y += (DefaultSectionSize * (Args.InBuildStats.Bounds.Y - 1)) - DefaultSectionSize;
        }
        else if (IsFacingBack(RightAdjustedIndex, Args.InMaxInRow))
        {
            // TODO: Fix bug
            // In case of Bounds.X = 2 and Bounds.Y = 1
            // Index=6 goes here, but need to be at IsFacingRight
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += (DefaultSectionSize * (Args.InBuildStats.Bounds.X + 1)) - DefaultSectionSize;
            Args.OutLocation.Y += (DefaultSectionSize * (Args.InBuildStats.Bounds.Y - 1));
        }
        else // We are facing the right side
        {
            Args.OutLocation.X += (DefaultSectionSize * RightAdjustedIndex) - DefaultSectionSize;
            Args.OutLocation.Y += (DefaultSectionSize * Args.InBuildStats.Bounds.Y) - DefaultSectionSize;
        }
    }
    else // Middle
    {
        //const int32 ClampedIndex = FMath::Clamp(Args.InIndex, 0, Args.InMaxCount);
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
}

void UModularSectionResolution::GetNextWallTransform(FNextTransformArgs& Args) const
{
    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);

    if (Args.InIndex < Args.InMaxInRow) // Left
    {
        if (IsFacingFront(AdjustedIndex))
        {
            Args.OutRotation.Yaw += 90.f;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
        else if (IsFacingBack(AdjustedIndex, Args.InMaxInRow))
        {
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += DefaultSectionSize * Args.InBuildStats.Bounds.X;
        }
        else if (IsFacingLeft(GetCurrentRow(Args.InIndex, Args.InMaxInRow)))
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
}

void UModularSectionResolution::GetNextRoofTransform_DEPRECATED(FNextTransformArgs& Args) const
{
    checkf(false, TEXT("GetNextRoofTransform method is deprecated and should not be used anymore."
        " Use GetNextRoofTransform2 or GetNextRoofTransform3 instead."));

    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);
    const int32 CurrentRow = GetCurrentRow(Args.InIndex, Args.InMaxInRow);

    if (IsFacingFront(AdjustedIndex))
    {
        Args.OutRotation.Yaw += 90.f;
        Args.OutLocation.Y += (DefaultSectionSize * CurrentRow) - DefaultSectionSize;
    }
    else if (IsFacingBack(AdjustedIndex, Args.InMaxInRow))
    {
        Args.OutRotation.Yaw -= 90.f;
        Args.OutLocation.X += (DefaultSectionSize * AdjustedIndex) - DefaultSectionSize;
        Args.OutLocation.Y += (DefaultSectionSize * CurrentRow);
    }
    else
    {
        if (IsFacingLeft(CurrentRow)) // We are facing the left side
        {
            Args.OutRotation.Yaw -= 180.f;
            Args.OutLocation.X += DefaultSectionSize * AdjustedIndex;
            Args.OutLocation.Y -= DefaultSectionSize;
        }
        else if (IsFacingRight(CurrentRow, Args.InMaxCount, Args.InMaxInRow)) // We are facing the right side
        {
            Args.OutLocation.X += (DefaultSectionSize * AdjustedIndex) - DefaultSectionSize;

            const uint8 RemainderShift = static_cast<bool>(Args.InMaxCount % Args.InMaxInRow);
            Args.OutLocation.Y += (DefaultSectionSize * CurrentRow) + (DefaultSectionSize * RemainderShift);        // here Y need to be adjusted
        }
        else
        {
            Args.OutRotation.Yaw -= 90.f;
            Args.OutLocation.X += (DefaultSectionSize * (Args.InMaxInRow - 2));
            Args.OutLocation.Y += (DefaultSectionSize * CurrentRow);
        }

    }

    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
}

void UModularSectionResolution::GetNextRoofTransform_Gen2(FNextTransformArgs& Args) const
{
    GetNextWallTransform_Gen2(Args);
}

void UModularSectionResolution::GetNextRoofTransform(FNextTransformArgs& Args) const
{
    GetNextWallTransform(Args);
}

void UModularSectionResolution::GetNextRooftopTransform(FNextTransformArgs& Args) const
{
    if (Args.InPreviousLevelResolution)
    {
        Args.OutLocation.X += Args.InPreviousLevelResolution->Resolution.Y;
        Args.OutLocation.Y += Args.InPreviousLevelResolution->Resolution.Y;
    }

    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);
    const int32 CurrentRow = GetCurrentRow(Args.InIndex, Args.InMaxInRow);
    
    Args.OutLocation.X += Resolution.X * AdjustedIndex;
    Args.OutLocation.Y += Resolution.Y * CurrentRow;
    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
}

void UModularSectionResolution::GetNextCornerTransform(FNextTransformArgs& Args) const
{
    const int32 AdjustedIndex = GetAdjustedIndex(Args.InIndex, Args.InMaxInRow);
    const int32 CurrentRow = GetCurrentRow(Args.InIndex, Args.InMaxInRow);

    // If we are on the back side in the current resolution - then we should be
    // on the back side on the max resolution (from the stats)
    if (IsFacingBack(AdjustedIndex, Args.InMaxInRow))
    {
        Args.OutLocation.X += (DefaultSectionSize * Args.InBuildStats.MaxCountInRow) - DefaultSectionSize * 2.f;
        if (IsFacingLeft(CurrentRow))
        {
            Args.OutRotation.Yaw += 90.f;
        }

        if (IsFacingRight(CurrentRow, Args.InMaxCount, Args.InMaxInRow))
        {
            Args.OutRotation.Yaw -= 90.f;
        }
    }

    // Same with the right side
    if (IsFacingRight(CurrentRow, Args.InMaxCount, Args.InMaxInRow))
    {
        // TODO: With Bounds = (X=2, Y=1) this is not working yet. Fix
        // With such bounds BuildStats have MaxTotalRows property equals 2 and need to be 1
        UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: InBuildStats.MaxTotalRows = %d, InBuildStats.Bounds.Y = %d"),
            *GetName(), 
            Args.InBuildStats.MaxTotalRows, Args.InBuildStats.Bounds.Y);
        ///Args.OutLocation.Y += (DefaultSectionSize * Args.InBuildStats.MaxTotalRows) - DefaultSectionSize;
        Args.OutLocation.Y += (DefaultSectionSize * Args.InBuildStats.Bounds.Y) - DefaultSectionSize;
    }
    else if (IsFacingFront(AdjustedIndex) || IsFacingLeft(CurrentRow))
    {
        Args.OutRotation.Yaw += 90.f;
        Args.OutLocation.Y -= DefaultSectionSize;
    }

    // -1.f as corners should be placed starting from the basement level.
    Args.OutLocation.Z += DefaultSectionSize * (Args.InLevelZMultiplier - 1.f);
}

FTransform UModularSectionResolution::GetNearTransform(const FTransform& InTransform, float InLevelZMultiplier,
    EModularSectionRelativePosition InSectionRelativePosition) const
{
    const FVector OutLocation = InTransform.GetLocation();
    const FRotator OutRotation = InTransform.GetRotation().Rotator();

    FNearTransformArgs Args = FNearTransformArgs(InLevelZMultiplier, OutLocation, OutRotation);

    switch (InSectionRelativePosition)
    {
        case EModularSectionRelativePosition::Front:
        {
            GetNearFrontTransform(Args);
            break;
        }
        case EModularSectionRelativePosition::Right:
        {
            break;
        }
        case EModularSectionRelativePosition::Back:
        {
            break;
        }
        case EModularSectionRelativePosition::Left:
        {
            break;
        }
    }

    return FTransform(OutRotation, OutLocation);
}

void UModularSectionResolution::GetNearFrontTransform(FNearTransformArgs& Args) const
{
    Args.OutLocation.X += Resolution.X;
    Args.OutLocation.Z += DefaultSectionSize * Args.InLevelZMultiplier;
}

FTransform UModularSectionResolution::GetTransformShifted(const FTransform& Initial, const FVector Shift,
    const bool bTranslationOnly)
{
    if (bTranslationOnly)
    {
        return FTransform(Initial.GetLocation() + Shift * DefaultSectionSize);
    }
    return FTransform(Initial.GetRotation(), Initial.GetLocation() + Shift * DefaultSectionSize, Initial.GetScale3D());
}

FName UModularSectionResolution::GetResolutionName() const
{
    return FName(FString::Printf(TEXT("%.1fx%.1fx%.1f"),
        static_cast<float>(Resolution.X) * .01f,
        static_cast<float>(Resolution.Y) * .01f,
        static_cast<float>(Resolution.Z) * .01f));
}

float UModularSectionResolution::GetHeightAsMultiplier() const
{
    return Resolution.Z > 0 ? static_cast<float>(Resolution.Z) / DefaultSectionSize : -1.f;
}

FIntPoint UModularSectionResolution::GetBounds(int32 MaxInRow, int32 TotalCount) const
{
    checkf(MaxInRow > 0 && TotalCount > 0, TEXT("Both MaxInRow and TotalCount must be larger than zero."));

    switch (SnapMode)
    {
        case EModularSectionResolutionSnapMode::Default:
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Bounds from Default snap mode."), *GetName());
            const int32 BoundX = MaxInRow;
            const int32 BoundY = TotalCount / BoundX;
            return FIntPoint(BoundX, BoundY);
        }
        case EModularSectionResolutionSnapMode::Wall:
        case EModularSectionResolutionSnapMode::Roof:
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Bounds from Wall or Roof snap mode."), *GetName());
            const int32 BoundX = MaxInRow - 2;
            const int32 BoundY = (TotalCount - MaxInRow) / BoundX;
            //const int32 BoundY = (TotalCount + MaxInRow) / MaxInRow;
            return FIntPoint(BoundX, BoundY);
        }
        case EModularSectionResolutionSnapMode::Rooftop:
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Bounds from Rooftop snap mode."), *GetName());
            const int32 BoundX = MaxInRow + 1;
            const int32 BoundY = (TotalCount + MaxInRow) / MaxInRow;
            return FIntPoint(BoundX, BoundY);
        }
        case EModularSectionResolutionSnapMode::Corner:
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Bounds from Corner snap mode."), *GetName());
            return FIntPoint(2, 2);
        }
        default:
        {
            UE_LOG(LogSectionResolution, VeryVerbose, TEXT("%s: Invalid bounds."), *GetName());
            return FIntPoint(-1, -1);
        }
    }
}

FIntVector UModularSectionResolution::GetValue() const
{
    return Resolution;
}

EModularSectionResolutionSnapMode UModularSectionResolution::GetSnapMode() const
{
    return SnapMode;
}

FIntVector UModularSectionResolution::GetResolutionFromMesh(UStaticMesh* InMesh)
{
    if (InMesh)
    {
        return FIntVector(InMesh->GetBoundingBox().GetSize());
    }
    return FIntVector::ZeroValue;
}

bool UModularSectionResolution::IsWallFacingFront(int32 Index, int32 MaxInRow, int32 MaxCount)
{
    if (IsWallFacingLeft(Index, MaxInRow) || IsWallFacingRight(Index, MaxInRow, MaxCount))
    {
        return false;
    }
    return IsDefaultFacingFront(Index, MaxInRow);
}

bool UModularSectionResolution::IsWallFacingRight(int32 Index, int32 MaxInRow, int32 MaxCount)
{
    if (PlacementGeneration == EMBSResolutionAlgorithmGeneration::Gen2)
    {
        unimplemented();
        // TODO: Implement for legacy of remove Gen2
        return false;
    }
    if (PlacementGeneration == EMBSResolutionAlgorithmGeneration::Gen3)
    {
        return Index >= MaxCount - MaxInRow + 2;
    }
}

bool UModularSectionResolution::IsWallFacingRightLast(int32 Index, int32 MaxCount)
{
    return Index == MaxCount - 1;
}

bool UModularSectionResolution::IsWallFacingRightFirst(int32 Index, int32 MaxInRow, int32 MaxCount)
{
    return Index == MaxCount - MaxInRow + 2;
}

bool UModularSectionResolution::IsWallFacingBack(int32 Index, int32 MaxInRow, int32 MaxCount)
{
    UE_LOG(LogSectionResolution, VeryVerbose, TEXT("IsWallFacingBack: Index=%d, MaxInRow=%d, MaxCount=%d. "
        "%d >= %d && %d < %d - %d + 2 = %s"),
        Index, MaxInRow, MaxCount, Index, MaxInRow, Index, MaxCount, MaxInRow,
        (Index >= MaxInRow && Index < MaxCount - MaxInRow + 2 ? TEXT("true") : TEXT("false")));

    if (Index < MaxInRow)
    {
        UE_LOG(LogSectionResolution, VeryVerbose, TEXT("On the left (%d < %d), Back=%s"),
            Index, MaxInRow, (IsDefaultFacingBack(Index, MaxInRow) ? TEXT("true") : TEXT("false")));
        return IsDefaultFacingBack(Index, MaxInRow);
    }
    if (Index >= MaxCount - MaxInRow + 2)
    {
        UE_LOG(LogSectionResolution, VeryVerbose, TEXT("On the right (%d >= %d - %d + 2), Back=false"),
            Index, MaxCount, MaxInRow);
        return false;
    }
    UE_LOG(LogSectionResolution, VeryVerbose, TEXT("In the middle, Back=%s"),
        (!IsFacingFront(GetAdjustedIndex(Index, 2)) ? TEXT("true") : TEXT("false")));
    return !IsFacingFront(GetAdjustedIndex(Index, 2));
}

bool UModularSectionResolution::IsWallFacingLeft(int32 Index, int32 MaxInRow)
{
    return Index != 0 && Index < MaxInRow;
}

bool UModularSectionResolution::IsWallFacingLeftLast(int32 Index, int32 MaxInRow)
{
    return Index == MaxInRow - 2;
}

bool UModularSectionResolution::IsWallFacingLeftFirst(int32 Index)
{
    return Index == 1;
}

bool UModularSectionResolution::IsWallInMiddle(int32 Index, int32 MaxInRow, int32 MaxCount)
{
    return Index >= MaxInRow && Index < MaxCount - MaxInRow + 2;
}

bool UModularSectionResolution::IsDefaultFacingFront(int32 Index, int32 MaxInRow)
{
    return Index % MaxInRow == 0;
}

bool UModularSectionResolution::IsDefaultFacingBack(int32 Index, int32 MaxInRow)
{
    return Index % MaxInRow == MaxInRow - 1;
}

constexpr int32 UModularSectionResolution::GetMaxRow(int32 MaxCount, int32 MaxInRow)
{
    return MaxCount / MaxInRow;
}

constexpr int32 UModularSectionResolution::GetWallMaxRow(int32 MaxCount, int32 MaxInRow)
{
    const int32 Remainder = MaxCount % MaxInRow;
    if (Remainder != 0)
    {
        return GetMaxRow(MaxCount, MaxInRow) + Remainder / 2;
    }

    return GetMaxRow(MaxCount, MaxInRow);
}

constexpr int32 UModularSectionResolution::GetAdjustedIndex(int32 Index, int32 MaxInRow)
{
    return Index % MaxInRow;
}

constexpr int32 UModularSectionResolution::GetAdjustedIndex(int32 Index, const FModularBuildStats& BuildStats)
{
    return Index % BuildStats.MaxCountInRow;
}

constexpr int32 UModularSectionResolution::GetCurrentRow(int32 Index, int32 MaxInRow)
{
    return Index > 0 ? Index / MaxInRow : 0;
}

int32 UModularSectionResolution::GetWallCurrentRow(int32 Index, int32 MaxInRow)
{
    return GetCurrentRow(Index - MaxInRow, 2) + 1;
}

int32 UModularSectionResolution::GetWallLeftFirstIndex()
{
    return 1;
}

int32 UModularSectionResolution::GetWallLeftLastIndex(int32 MaxInRow)
{
    return MaxInRow - 2;
}

int32 UModularSectionResolution::GetWallRightFirstIndex(int32 MaxInRow, int32 MaxCount)
{
    return MaxCount - MaxInRow + 2;
}

int32 UModularSectionResolution::GetWallRightLastIndex(int32 MaxCount)
{
    return MaxCount - 1;
}

TArray<int32> UModularSectionResolution::GetWallFrontIndices(int32 MaxInRow, int32 MaxCount)
{
    TArray<int32> OutIndices;
    for (int32 i = 0; i < MaxCount; i++)
    {
        if (IsWallFacingFront(i, MaxInRow, MaxCount))
        {
            OutIndices.Add(i);
        }
    }
    return OutIndices;
}

TArray<int32> UModularSectionResolution::GetWallBackIndices(int32 MaxInRow, int32 MaxCount)
{
    TArray<int32> OutIndices;
    for (int32 i = 0; i < MaxCount; i++)
    {
        if (IsWallFacingBack(i, MaxInRow, MaxCount))
        {
            OutIndices.Add(i);
        }
    }
    return OutIndices;
}

constexpr int32 UModularSectionResolution::GetCurrentRow(int32 Index, const FModularBuildStats& BuildStats)
{
    return Index > 0 ? Index / BuildStats.MaxCountInRow : 0;
}

constexpr bool UModularSectionResolution::IsFacingFront(int32 AdjustedIndex)
{
    return AdjustedIndex == 0;
}

constexpr bool UModularSectionResolution::IsFacingBack(int32 AdjustedIndex, int32 MaxInRow)
{
    return AdjustedIndex == MaxInRow - 1;
}

constexpr bool UModularSectionResolution::IsFacingBack(int32 AdjustedIndex, const FModularBuildStats& BuildStats)
{
    return AdjustedIndex == BuildStats.MaxCountInRow - 1;
}

constexpr bool UModularSectionResolution::IsFacingLeft(int32 CurrentRow)
{
    return CurrentRow == 0;
}

constexpr bool UModularSectionResolution::IsFacingRight(int32 CurrentRow, int32 MaxCount, int32 MaxInRow)
{
    return CurrentRow == GetMaxRow(MaxCount, MaxInRow) - 1;
}

constexpr bool UModularSectionResolution::IsFacingRight(int32 CurrentRow, const FModularBuildStats& BuildStats)
{
    return CurrentRow == GetMaxRow(BuildStats.MaxTotalCount, BuildStats.MaxCountInRow) - 1;
}
