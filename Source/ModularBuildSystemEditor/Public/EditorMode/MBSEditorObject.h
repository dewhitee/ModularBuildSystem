// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MBSEditorObject.generated.h"

namespace MBS
{
class FEditorMode;
}

UENUM(BlueprintType)
enum class EMBSEditorSelectionType : uint8
{
	Section,
	Side,
	System,
};

/**
 * 
 */
UCLASS()
class MODULARBUILDSYSTEMEDITOR_API UMBSEditorObject : public UObject
{
	GENERATED_BODY()

	MBS::FEditorMode* ParentMode;
	
	UPROPERTY(EditAnywhere, NonTransactional, Category="Select")
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	UPROPERTY(EditAnywhere, NonTransactional, Category="Select")
	EMBSEditorSelectionType SelectionType;
	
	bool bOperationsToolSelected	= false;
	bool bSelectToolSelected		= false;

public:
	bool GetOperationsToolSelected() const { return bOperationsToolSelected; }
	bool GetSelectToolSelected() const { return bSelectToolSelected; }
	
	void SetOperationsToolSelected(bool bSelected) { bOperationsToolSelected = bSelected; }
	void SetSelectToolSelected(bool bSelected) { bSelectToolSelected = bSelected; }
	
	void SetParent(MBS::FEditorMode* ModeParent) { ParentMode = ModeParent; }

	UMaterialInterface* GetHighlightMaterial() const { return HighlightMaterial; }
	EMBSEditorSelectionType GetSelectionType() const { return SelectionType; }
};
