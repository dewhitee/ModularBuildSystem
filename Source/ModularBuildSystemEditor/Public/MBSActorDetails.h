// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class AModularBuildSystemActor;

namespace MBS
{
/**
 * Handles creation of custom detail sections in Details panel for AModularBuildSystemActor
 * @see AModularBuildSystemActor
 */
class MODULARBUILDSYSTEMEDITOR_API FActorDetails final : public IDetailCustomization
{
	struct FButtonEntry
	{
		FName CategoryName;
		FName ButtonName;
		FText Tooltip;
		TFunction<FReply()> OnClick;

		FButtonEntry(FName InCategoryName, FName InButtonName, const FString& InTooltip, const TFunction<FReply()>& InOnClick)
			: CategoryName(InCategoryName), ButtonName(InButtonName), Tooltip(FText::FromString(InTooltip)), OnClick(InOnClick)
		{
		}
	};
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FActorDetails);	
	}
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static void RegisterSectionMappings(FPropertyEditorModule& PropertyEditorModule);

private:
	void AddCallInEditorMethods(IDetailLayoutBuilder& DetailBuilder);
	TArray<FButtonEntry> GetButtons(TWeakObjectPtr<AModularBuildSystemActor> BS) const;
	
};
}
