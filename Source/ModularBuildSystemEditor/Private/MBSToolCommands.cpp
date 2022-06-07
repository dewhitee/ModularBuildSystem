// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSToolCommands.h"

#define LOCTEXT_NAMESPACE "MBSToolCommands"

MBS::FToolCommands::FToolCommands()
	: TCommands<MBS::FToolCommands>(
		TEXT("MBSTool"),
		FText::FromString("MBS Tool"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{
}

MBS::FToolCommands::~FToolCommands()
{
}

void MBS::FToolCommands::RegisterCommands()
{
	UI_COMMAND(MenuCommand1, "Menu Command 1", "Test Menu Command 1.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MenuCommand2, "Menu Command 2", "Test Menu Command 2.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MenuCommand3, "Menu Command 3", "Test Menu Command 3.", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(SetOperationsCommand, "Operations", "Common operations", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(SetSelectCommand, "Select", "Select single MBS actor", EUserInterfaceActionType::ToggleButton, FInputChord());
	//UI_COMMAND(SetSelectAll, "SelectAll", "Select all MBS actors", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
