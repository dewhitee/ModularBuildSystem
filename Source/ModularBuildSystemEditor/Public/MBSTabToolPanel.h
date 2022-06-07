// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace MBS
{

class FTabTool;
/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API STabToolPanel : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STabToolPanel) {}
	SLATE_ARGUMENT(TWeakPtr<FTabTool>, Tool);
	SLATE_END_ARGS()

protected:
	TWeakPtr<FTabTool> Tool;

public:	
	void Construct(const FArguments& InArgs);
	
};
}
