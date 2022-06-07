// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MBSEditorMode.h"
#include "StatusBarSubsystem.h"

class AModularBuildSystemActor;

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API SEditorModeWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SEditorModeWidget) {}
	SLATE_END_ARGS();

private:
	TWeakPtr<SDockTab> ParentTab;
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<IDetailsView> DetailsPanel;

	FText TabName;
	const FSlateBrush* TabIcon = nullptr;

	FStatusBarMessageHandle StatusBarMessageHandle;
	
public:
	void Construct(const FArguments& InArgs);
	static MBS::FEditorMode* GetEditorMode();
	void SetParentTab(TSharedRef<SDockTab>& InDockTab);
	FText GetActiveToolName() const;
	FText GetActiveToolMessage() const;
	void CustomizeToolBarPalette(FToolBarBuilder& ToolBarBuilder);

private:
	static FReply AllToBuildModeOn();
	static FReply AllToBuildModeOff();
	static FReply AllToSectionReloadModeNone();
	static FReply RegenerateAllBuildSystems();
	static FReply MergeAllBuildSystems();
	static FReply UnmergeAllBuildSystems();

	TSharedRef<SWidget> BuildToolBar();
	static bool IsOperationsTool();
	static bool IsSelectTool();
	
	FText GetTotalBuildSystemCount() const;
	FText GetTotalHouseCount() const;
	FText GetTotalMergedCount() const;
	FText GetTotalUnmergedCount() const;

	void RefreshDetailsPanel() const;
	bool SetIsVisible(const FPropertyAndParent& InPropertyAndParent) const;

	static int32 GetCountOf(TSubclassOf<AActor> Class);

	template<class T>
	static int32 GetCountOf(TSubclassOf<T> Class, TFunction<bool(T*)> ForEach)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetEditorWorld(), Class, OutActors);
		int32 OutCount = 0;
		for (int32 i = 0; i < OutActors.Num(); i++)
		{
			if (ForEach(Cast<T>(OutActors[i])))
			{
				OutCount++;
			}
		}
		return OutCount;
	}
	static UWorld* GetEditorWorld();
};
}
