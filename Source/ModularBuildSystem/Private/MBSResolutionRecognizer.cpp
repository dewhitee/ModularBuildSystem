// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSResolutionRecognizer.h"

#include "AssetSelection.h"
#include "AssetViewUtils.h"
#include "ModularBuildSystem.h"
#include "ModularSectionResolution.h"

UModularSectionResolution* UMBSResolutionRecognizer::RecognizeFromSelection()
{
	TArray<FAssetData> SelectedAssets;
	AssetSelectionUtils::GetSelectedAssets(SelectedAssets);

	// Find all resolutions available in the project dir and MBS plugin dir
	TArray<FAssetData> FoundResolutions;
	AssetViewUtils::GetAssetsInPaths(MBS::FPaths::GetSectionResolutionDirectories(), FoundResolutions);
	
	for (auto& SelectedAsset : SelectedAssets)
	{
		UE_LOG(LogMBSResolutionRecognizer, Log, TEXT("%s: Selected asset name = %s"),
			*GetName(), *SelectedAsset.AssetName.ToString());
		SelectedAsset.PrintAssetData();
		
		if (!SelectedAsset.IsUAsset())
		{
			UE_LOG(LogMBSResolutionRecognizer, Error, TEXT("%s: Selected asset is not UAsset"), *GetName());
			continue;
		}
		
		// TODO: Implement
		// Check if static mesh
		UObject* Asset = SelectedAsset.GetAsset();
		if (const UStaticMesh* SM = Cast<UStaticMesh>(Asset))
		{
			// Get bounds of a static mesh
			// Save FIntVector from bounds clamped
			const FVector Bounds = SM->GetBoundingBox().GetSize();
			UModularSectionResolution* ClosestResolution = nullptr;

			// Check if there is any Resolution that is close enough to saved FIntVector with specified tolerance
			if (FoundResolutions.Num() == 0)
			{
				UE_LOG(LogMBSResolutionRecognizer, Error, TEXT("%s: No modular section resolutions found."),
					*GetName());
				break;
			}
			
			// Loop all found resolutions to find the closest to the saved bounds vector
			FVector ClosestVec = {};
			float ClosestDistance = 0.f;
			for (const auto& FoundAsset : FoundResolutions)
			{
				if (UModularSectionResolution* Resolution = Cast<UModularSectionResolution>(FoundAsset.GetAsset()))
				{
					const FVector ResolutionVec = FVector(Resolution->GetValue());
					const float CurrentDistance = FVector::Dist(ClosestVec, ResolutionVec);
					if (ClosestResolution && CurrentDistance < ClosestDistance)
					{
						// Close enough resolution found
						ClosestResolution = Resolution;
						ClosestVec = ResolutionVec;
						ClosestDistance = CurrentDistance;
					}
					else if (ResolutionVec.Equals(Bounds, 0.1))
					{
						ClosestResolution = Resolution;
						break;
					}
				}
			}

			// If there is no close enough Resolution - then create new one and return pointer to it
			if (!ClosestResolution)
			{
				// If we didn't found any resolution - return the Undefined
				if (bReturnUndefinedIfNotRecognized)
				{
					// TODO: Find and return undefined resolution
					return nullptr;
				}
				
				return nullptr;
			}
			
			// Else return pointer to the found Resolution
			return ClosestResolution;
		}
	}
	
	return nullptr;
}
