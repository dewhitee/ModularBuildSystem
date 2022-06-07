// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSMenuTool.h"

#include "MBSToolCommands.h"

#define LOCTEXT_NAMESPACE "MBSMenuTool"

MBS::FMenuTool::FMenuTool()
{
}

MBS::FMenuTool::~FMenuTool()
{
}

void MBS::FMenuTool::OnStartupModule()
{
	CommandList = MakeShareable(new FUICommandList);
	MBS::FToolCommands::Register();
	MapCommands();
	FModularBuildSystemEditorModule::Get().AddMenuExtension(
		FMenuExtensionDelegate::CreateRaw(this, &MBS::FMenuTool::MakeMenuEntry),
		FName("Section_1"),
		CommandList);
}

void MBS::FMenuTool::OnShutdownModule()
{
	MBS::FToolCommands::Unregister();
}

void MBS::FMenuTool::MakeMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(MBS::FToolCommands::Get().MenuCommand1);
	MenuBuilder.AddSubMenu(
		FText::FromString("Sub Menu"),
		FText::FromString("This is test sub menu"),
		FNewMenuDelegate::CreateSP(this, &MBS::FMenuTool::MakeSubMenu));
}

void MBS::FMenuTool::MakeSubMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(MBS::FToolCommands::Get().MenuCommand2);
	MenuBuilder.AddMenuEntry(MBS::FToolCommands::Get().MenuCommand3);
}

void MBS::FMenuTool::MapCommands()
{
	const auto& Commands = MBS::FToolCommands::Get();
	CommandList->MapAction(
		Commands.MenuCommand1,
		FExecuteAction::CreateSP(this, &MBS::FMenuTool::MenuCommand1),
		FCanExecuteAction());

	CommandList->MapAction(
		Commands.MenuCommand2,
		FExecuteAction::CreateSP(this, &MBS::FMenuTool::MenuCommand2),
		FCanExecuteAction());

	CommandList->MapAction(
		Commands.MenuCommand3,
		FExecuteAction::CreateSP(this, &MBS::FMenuTool::MenuCommand3),
		FCanExecuteAction());
}

void MBS::FMenuTool::MenuCommand1()
{
	UE_LOG(LogMBSEditor, Log, TEXT("MenuCommand1"));
}

void MBS::FMenuTool::MenuCommand2()
{
	UE_LOG(LogMBSEditor, Log, TEXT("MenuCommand2"));
}

void MBS::FMenuTool::MenuCommand3()
{
	UE_LOG(LogMBSEditor, Log, TEXT("MenuCommand3"));
}

#undef LOCTEXT_NAMESPACE
