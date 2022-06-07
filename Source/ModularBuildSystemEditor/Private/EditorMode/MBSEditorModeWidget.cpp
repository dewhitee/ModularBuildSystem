// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorMode/MBSEditorModeWidget.h"

#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "MBSFunctionLibrary.h"
#include "House/HouseBuildSystemActor.h"
#include "MBSToolCommands.h"
#include "ModularBuildSystemActor.h"
#include "ModularBuildSystemEditor.h"
#include "ObjectEditorUtils.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/Private/Widgets/Views/SListPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "MBSTool"

void MBS::SEditorModeWidget::Construct(const FArguments& InArgs)
{
	constexpr float Padding = 5.f;
	//const FSlateBrush* Brush = FEditorStyle::GetBrush("ToolPanel.GroupBorder");
	//const FMargin ModeToolbarMargin = FMargin(4, 0, 0, 0);

	// Create defaults panel
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SEditorModeWidget::SetIsVisible));

	if (MBS::FEditorMode* EdMode = GetEditorMode())
	{
		DetailsPanel->SetObject(EdMode->UISettings);
		EdMode->OnToolChanged.AddSP(this, &MBS::SEditorModeWidget::RefreshDetailsPanel);
	}
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		//.AutoHeight()
		.Padding(0, 0, 0, 5)
		[
			SAssignNew(ErrorText, SErrorText)
		]
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SScaleBox)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("COMMON OPERATIONS")))
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("All to build mode ON")))
					.OnClicked_Static(&MBS::SEditorModeWidget::AllToBuildModeOn)
				]
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("All to build mode OFF")))
					.OnClicked_Static(&MBS::SEditorModeWidget::AllToBuildModeOff)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Regenerate all")))
					.OnClicked_Static(&MBS::SEditorModeWidget::RegenerateAllBuildSystems)
				]
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Merge all")))
					.OnClicked_Static(&MBS::SEditorModeWidget::MergeAllBuildSystems)
				]
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Unmerge all")))
					.OnClicked_Static(&MBS::SEditorModeWidget::UnmergeAllBuildSystems)
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SScaleBox)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("TOTAL")))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0)
			[
				SNew(SBorder)
				.Padding(Padding)
				.Content()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(0)
						[
							SNew(STextBlock)
							.Text(this, &MBS::SEditorModeWidget::GetTotalBuildSystemCount)
						]
						+ SHorizontalBox::Slot()
						.Padding(0)
						[
							SNew(STextBlock)
							.Text(this, &MBS::SEditorModeWidget::GetTotalHouseCount)
						]
					]
					+ SVerticalBox::Slot()
					.Padding(Padding)
					[
						SNew(SBorder)
						.Padding(0)
						.ContentScale(0.1)
						[
							SNew(SVerticalBox)
						]
					]
					+ SVerticalBox::Slot()
					.Padding(0)
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(0)
						[
							SNew(STextBlock)
							// TODO: Get total merged count
							.Text(this, &MBS::SEditorModeWidget::GetTotalMergedCount)
						]
						+ SHorizontalBox::Slot()
						.Padding(0)
						[
							SNew(STextBlock)
							// TODO: Get total unmerged count
							.Text(this, &MBS::SEditorModeWidget::GetTotalUnmergedCount)
						]
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.Padding(0)
		[
			DetailsPanel.ToSharedRef()
		]
	];
	
	//UpdateModeToolBar();
}

MBS::FEditorMode* MBS::SEditorModeWidget::GetEditorMode()
{
	return static_cast<MBS::FEditorMode*>(GLevelEditorModeTools().GetActiveMode(MBS::FEditorMode::EM_MBS));
}

void MBS::SEditorModeWidget::SetParentTab(TSharedRef<SDockTab>& InDockTab)
{
	ParentTab = InDockTab;
	InDockTab->SetLabel(TabName);
	InDockTab->SetTabIcon(TabIcon);
}

FText MBS::SEditorModeWidget::GetActiveToolName() const
{
	FText OutText;
	if (IsOperationsTool())
	{
		OutText = LOCTEXT("MBSToolName_Test", "Test");
	}
	else if (IsSelectTool())
	{
		OutText = LOCTEXT("MBSToolName_Select", "Select");
	}
	return OutText;
}

FText MBS::SEditorModeWidget::GetActiveToolMessage() const
{
	FText OutText;
	if (IsOperationsTool())
	{
		OutText = LOCTEXT("MBSToolMessage_Test", "Test tool message tooltip.");
	}
	else if (IsSelectTool())
	{
		OutText = LOCTEXT("MBSToolMessage_Select", "Click to select ModularBuildSystemActor.");
	}
	return OutText;
}

void MBS::SEditorModeWidget::CustomizeToolBarPalette(FToolBarBuilder& ToolBarBuilder)
{
	ToolBarBuilder.AddToolBarButton(MBS::FToolCommands::Get().SetOperationsCommand);
	ToolBarBuilder.AddToolBarButton(MBS::FToolCommands::Get().SetSelectCommand);
}

FReply MBS::SEditorModeWidget::AllToBuildModeOn()
{
	UMBSFunctionLibrary::AllToBuildModeOn(GetEditorWorld());
	return FReply::Handled();
}

FReply MBS::SEditorModeWidget::AllToBuildModeOff()
{
	UMBSFunctionLibrary::AllToBuildModeOff(GetEditorWorld());
	return FReply::Handled();
}

FReply MBS::SEditorModeWidget::AllToSectionReloadModeNone()
{
	UMBSFunctionLibrary::AllToSectionReloadModeNone(GetEditorWorld());
	return FReply::Handled();
}

FReply MBS::SEditorModeWidget::RegenerateAllBuildSystems()
{
	UMBSFunctionLibrary::RegenerateAllBuildSystems(GetEditorWorld());
	return FReply::Handled();
}

FReply MBS::SEditorModeWidget::MergeAllBuildSystems()
{
	UMBSFunctionLibrary::MergeAllBuildSystems(GetEditorWorld());
	return FReply::Handled();
}

FReply MBS::SEditorModeWidget::UnmergeAllBuildSystems()
{
	UMBSFunctionLibrary::UnmergeAllBuildSystems(GetEditorWorld());
	return FReply::Handled();
}

TSharedRef<SWidget> MBS::SEditorModeWidget::BuildToolBar()
{
	FVerticalToolBarBuilder Toolbar(GetEditorMode()->UICommandList, FMultiBoxCustomization::None);
	Toolbar.SetLabelVisibility(EVisibility::Collapsed);
	Toolbar.SetStyle(&FEditorStyle::Get(), "MBSEditorToolbar");
	{
		Toolbar.AddToolBarButton(MBS::FToolCommands::Get().SetOperationsCommand);
		Toolbar.AddToolBarButton(MBS::FToolCommands::Get().SetSelectCommand);
	}

	return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.Padding(0)
				.BorderImage(FEditorStyle::GetBrush("NoBorder"))
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				[
					Toolbar.MakeWidget()
				]
			]
		];
}

bool MBS::SEditorModeWidget::IsOperationsTool()
{
	return GetEditorMode()->UISettings->GetOperationsToolSelected();
}

bool MBS::SEditorModeWidget::IsSelectTool()
{
	return GetEditorMode()->UISettings->GetSelectToolSelected();
}

FText MBS::SEditorModeWidget::GetTotalBuildSystemCount() const
{
	return FText::FromString(
		FString::Printf(TEXT("Total build system count: %d"), GetCountOf(AModularBuildSystemActor::StaticClass())));
}

FText MBS::SEditorModeWidget::GetTotalHouseCount() const
{
	return FText::FromString(
		FString::Printf(TEXT("Total house count: %d"), GetCountOf(AHouseBuildSystemActor::StaticClass())));
}

FText MBS::SEditorModeWidget::GetTotalMergedCount() const
{
	const int32 Count = GetCountOf<AModularBuildSystemActor>(
		AModularBuildSystemActor::StaticClass(), [](const AModularBuildSystemActor* Actor) -> bool
		{
			return Actor->IsMerged();
		});
	return FText::FromString(FString::Printf(TEXT("Total merged count: %d"), Count));
}

FText MBS::SEditorModeWidget::GetTotalUnmergedCount() const
{
	const int32 Count = GetCountOf<AModularBuildSystemActor>(
		AModularBuildSystemActor::StaticClass(), [](const AModularBuildSystemActor* Actor) -> bool
		{
			return !Actor->IsMerged();
		});
	return FText::FromString(FString::Printf(TEXT("Total unmerged count: %d"), Count));
}

void MBS::SEditorModeWidget::RefreshDetailsPanel() const
{
	UE_LOG(LogMBSEditor, Verbose, TEXT("RefreshDetailsPanel"));
	if (DetailsPanel)
	{
		DetailsPanel->ForceRefresh();
	}
}

bool MBS::SEditorModeWidget::SetIsVisible(const FPropertyAndParent& InPropertyAndParent) const
{
	UE_LOG(LogMBSEditor, Verbose, TEXT("SetIsVisible"));
	const FName CategoryName = FObjectEditorUtils::GetCategoryFName(&InPropertyAndParent.Property);

	UE_LOG(LogMBSEditor, Verbose, TEXT("CategoryName=%s"), *CategoryName.ToString());
	if (CategoryName == TEXT("Select"))
	{
		return IsSelectTool();
	}
	if (CategoryName == TEXT("Operations"))
	{
		return IsOperationsTool();
	}

	return true;
}

int32 MBS::SEditorModeWidget::GetCountOf(TSubclassOf<AActor> Class)
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetEditorWorld(), Class, OutActors);
	return OutActors.Num();
}

UWorld* MBS::SEditorModeWidget::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

#undef LOCTEXT_NAMESPACE
