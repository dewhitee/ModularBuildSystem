// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace MBS
{
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FToolCommands : public TCommands<FToolCommands>
{
public:
	// Dropdown menu
	TSharedPtr<FUICommandInfo> MenuCommand1;
	TSharedPtr<FUICommandInfo> MenuCommand2;
	TSharedPtr<FUICommandInfo> MenuCommand3;

	// Editor mode actions
	TSharedPtr<FUICommandInfo> SetOperationsCommand;
	TSharedPtr<FUICommandInfo> SetSelectCommand;
	
	FToolCommands();
	virtual ~FToolCommands() override;
	virtual void RegisterCommands() override;
};
}
