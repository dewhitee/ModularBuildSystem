// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "MBSEditorObject.h"

class AModularBuildSystemActor;

namespace MBS
{
struct HProxy : public HHitProxy
{
	DECLARE_HIT_PROXY()

	UObject* RefObject;
	int32 Index;
	
	HProxy(UObject* InRefObject, int32 InIndex)
		: HHitProxy(HPP_UI), RefObject(InRefObject), Index(InIndex)
	{}
};

/**
 * 
 */
class MODULARBUILDSYSTEMEDITOR_API FEditorMode : public FEdMode
{
public:
	UMBSEditorObject* UISettings;
	TSharedPtr<FUICommandList> UICommandList;
	FSimpleMulticastDelegate OnToolChanged;
	const static FEditorModeID EM_MBS;
	
private:	
	TWeakObjectPtr<AModularBuildSystemActor> CurrentSelectedBuildSystem;
	int32 CurrentSelectedIndex = -1;
	
	FIntRect ViewRect;
	FViewMatrices ViewMatrices;

	struct FHighlightedActor
	{
		AStaticMeshActor* StaticMeshActor;
		UMaterialInterface* OriginalMaterial;
	};
	
	TArray<FHighlightedActor> HighlightedStaticMeshActors;
	TObjectPtr<AStaticMeshActor> HighlightedActor = nullptr;
	TObjectPtr<UMaterialInterface> HighlightedActorOriginalMaterial = nullptr;
	
public:
	FEditorMode();
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	//virtual bool Select(AActor* InActor, bool bInSelected) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

private:
	void BindCommands();
	void OnMouseMoveSectionHighlight(const FViewportCursorLocation& CursorLocation);
	void OnMouseMoveSystemHighlight(const FViewportCursorLocation& CursorLocation);
	void ResetHighlightedActor(FHighlightedActor& InHighlightedActor);
	static void ResetHighlightedActor(TObjectPtr<AStaticMeshActor>& InHighlightedActor, TObjectPtr<UMaterialInterface>& InHighlightedActorOriginalMaterial);
	void ResetHighlightedActor();
	void HighlightActor(AStaticMeshActor* InNewHighlightedActor, FHighlightedActor& InHighlightedActor);
	void HighlightActor(AStaticMeshActor* InNewHighlightedActor, TObjectPtr<AStaticMeshActor>& InHighlightedActor, TObjectPtr<UMaterialInterface>& InHighlightedActorOriginalMaterial);
	AActor* GetActorAtLocation(const FViewportCursorLocation& Location) const;

	void OnSetOperationsCommand() const;
	void OnSetSelectCommand() const;
};
}
