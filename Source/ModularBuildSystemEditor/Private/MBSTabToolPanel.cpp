// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSTabToolPanel.h"

#include "Widgets/Layout/SScrollBox.h"

void MBS::STabToolPanel::Construct(const FArguments& InArgs)
{
	Tool = InArgs._Tool;
	if (Tool.IsValid())
	{
		// Actions with Tool object
	}

	constexpr float OuterPadding = 5.f;
	constexpr float InnerPadding = 15.f;

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(OuterPadding)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FColor(192, 192, 192, 255))
			.Padding(InnerPadding)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("This is a tab example.")))
			]
		]
		+ SScrollBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(OuterPadding)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FColor(192, 192, 192, 255))
			.Padding(InnerPadding)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("This is another text")))
			]
		]
	];
}
