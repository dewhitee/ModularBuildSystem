// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorMode/MBSEditorMode.h"
#include "EditorMode/MBSEditorModeToolkit.h"
#include "ModularBuildSystemEditor.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "MBSToolCommands.h"
#include "Toolkits/ToolkitManager.h"
#include "ModularBuildSystemActor.h"
#include "Selection.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_HIT_PROXY(MBS::HProxy, HHitProxy);

const FEditorModeID MBS::FEditorMode::EM_MBS(TEXT("EM_MBS"));

MBS::FEditorMode::FEditorMode()
{
	UISettings = NewObject<UMBSEditorObject>(GetTransientPackage(), TEXT("UISettings"), RF_Transactional);
	//UISettings->SetParent(this);
}

void MBS::FEditorMode::Enter()
{
	FEdMode::Enter();
	if (!Toolkit.IsValid())
	{
		Toolkit = MakeShareable(new MBS::FEditorModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());

		UICommandList = Toolkit->GetToolkitCommands();
		BindCommands();
	}
}

void MBS::FEditorMode::Exit()
{
	// todo: fix unresolved external symbol
	FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
	Toolkit.Reset();
	FEdMode::Exit();
}

void MBS::FEditorMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	ViewRect = View->UnscaledViewRect;
	ViewMatrices = View->ViewMatrices;
	
	FEdMode::Render(View, Viewport, PDI);
}

void MBS::FEditorMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
}

bool MBS::FEditorMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy,
	const FViewportClick& Click)
{
	UE_LOG(LogMBSEditor, Warning, TEXT("HandleClick"));
	AActor* Actor = GetActorAtLocation(Click);
	UE_LOG(LogMBSEditor, Log, TEXT("Actor=%s"), Actor ? *Actor->GetActorLabel() : TEXT("nullptr"));
	if (UISettings && UISettings->GetSelectToolSelected())
	{
		UE_LOG(LogMBSEditor, Warning, TEXT("HitProxy"));
		if (USelection* Selection = GEditor->GetSelectedActors())
		{
			TArray<AActor*> SelectedActors;
			Selection->GetSelectedObjects(SelectedActors);
			if (!SelectedActors.IsEmpty() && !SelectedActors.Contains(Actor))
			{
				GEditor->SelectNone(true, true, true);
				
				/*for (const AActor* SelectedActor : SelectedActors)
				{
					UE_LOG(LogMBSEditor, Verbose, TEXT("SelectedActor=%s"), *SelectedActor->GetActorLabel());
					if (AActor* SelectionParent = SelectedActor->GetAttachParentActor())
					{
						UE_LOG(LogMBSEditor, Verbose, TEXT("SelectionParent=%s"), *SelectionParent->GetName());
						if (AModularBuildSystemActor* MBS = Cast<AModularBuildSystemActor>(SelectionParent))
						{
							if (GEditor->CanSelectActor(MBS, true, true, true))
							{
								UE_LOG(LogMBSEditor, Verbose, TEXT("Unselecting %s"), *SelectedActor->GetName());
								GEditor->SelectNone(true, true, true);
								UE_LOG(LogMBSEditor, Verbose, TEXT("Selecting %s"), *MBS->GetName());
								GEditor->SelectActor(MBS, true, true, true, true);
							}
							else
							{
								UE_LOG(LogMBSEditor, Warning, TEXT("Can't select %s"), *MBS->GetName());
							}
						}
						else
						{
							UE_LOG(LogMBSEditor, Warning, TEXT("SelectionParent is not MBS"));
						}
					}
					else
					{
						UE_LOG(LogMBSEditor, Warning, TEXT("SelectionParent=nullptr"));
					}
				}*/
			}

			if (Actor)
			{
				// If selected (clicked) actor is attached to the MBS actor
				if (AModularBuildSystemActor* MBS = Cast<AModularBuildSystemActor>(Actor->GetAttachParentActor()))
				{
					switch (UISettings->GetSelectionType())
					{
					case EMBSEditorSelectionType::Section: // Selecting only sections of an MBS actor
						GEditor->SelectActor(Actor, true, true, true);
						break;
					case EMBSEditorSelectionType::Side: // Selecting only single side of an MBS actor
						// Find which side actor is pointing to
						// TODO: implement
						break;
					case EMBSEditorSelectionType::System: // Selecting only a whole MBS actor
						GEditor->SelectActor(MBS, true, true, true);
						break;
					default: ;
					}
				}
				
			}

			Selection->GetSelectedObjects(SelectedActors);
			for (const AActor* SelectedActor : SelectedActors)
			{
				UE_LOG(LogMBSEditor, VeryVerbose, TEXT("PostClick: SelectedActor=%s"), *SelectedActor->GetActorLabel());
			}
		}
	}
	else
	{
		UE_LOG(LogMBSEditor, Error, TEXT("%s"), UISettings ? TEXT("Select tool is not selected!") : TEXT("UISettings was nullptr!"));
		return false;
	}

	return true;
}

bool MBS::FEditorMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 X, int32 Y)
{
	const FViewportCursorLocation& CursorLocation = ViewportClient->GetCursorWorldLocationFromMousePos();
	if (!UISettings->GetHighlightMaterial())
	{
		return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
	}

	switch (UISettings->GetSelectionType())
	{
		case EMBSEditorSelectionType::Section:
		{
			OnMouseMoveSectionHighlight(CursorLocation);
			break;
		}
		case EMBSEditorSelectionType::Side: break;
		case EMBSEditorSelectionType::System:
		{
			OnMouseMoveSystemHighlight(CursorLocation);
			break;
		}
		default: ;
	}
	
	return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
}

bool MBS::FEditorMode::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX,
	int32 InMouseY)
{
	return FEdMode::CapturedMouseMove(InViewportClient, InViewport, InMouseX, InMouseY);
}

void MBS::FEditorMode::BindCommands()
{
	const MBS::FToolCommands& Commands = MBS::FToolCommands::Get();
	UICommandList->MapAction(
		Commands.SetOperationsCommand,
		FExecuteAction::CreateRaw(this, &FEditorMode::OnSetOperationsCommand),
		FCanExecuteAction(),
		FIsActionChecked::CreateLambda([=] { return UISettings->GetOperationsToolSelected(); }));

	UICommandList->MapAction(
		Commands.SetSelectCommand,
		FExecuteAction::CreateRaw(this, &FEditorMode::OnSetSelectCommand),
		FCanExecuteAction(),
		FIsActionChecked::CreateLambda([=] { return UISettings->GetSelectToolSelected(); }));
}

void MBS::FEditorMode::OnMouseMoveSectionHighlight(const FViewportCursorLocation& CursorLocation)
{
	ResetHighlightedActor();
	
	AActor* HitActor = GetActorAtLocation(CursorLocation);
	UE_LOG(LogMBSEditor, VeryVerbose, TEXT("Hit.Actor=%s"), HitActor ? *HitActor->GetActorLabel() : TEXT("nullptr"));
	if (HitActor)
	{
		if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(HitActor))
		{
			HighlightActor(StaticMeshActor, HighlightedActor, HighlightedActorOriginalMaterial);
		}
	}
}

void MBS::FEditorMode::OnMouseMoveSystemHighlight(const FViewportCursorLocation& CursorLocation)
{
	for (auto& Highlighted : HighlightedStaticMeshActors)
	{
		ResetHighlightedActor(Highlighted);
	}
	
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		CursorLocation.GetOrigin(),
		CursorLocation.GetOrigin() + CursorLocation.GetDirection() * 1000000.f,
		ECollisionChannel::ECC_Visibility))
	{
		// Select all static mesh actors of MBS
		if (AActor* HitActor = Hit.GetActor())
		{
			if (const AModularBuildSystemActor* MBS = Cast<AModularBuildSystemActor>(HitActor))
			{
				// If system is found, then select all children of it
				TArray<AActor*> Children;
				MBS->GetAllChildActors(Children);
				for (const auto& Child : Children)
				{
					if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Child))
					{
						FHighlightedActor NewHighlightedActor;
						HighlightActor(StaticMeshActor, NewHighlightedActor);
					}
				}
			}
			else if (const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(HitActor))
			{
				// Find if parent actor is MBS
				AActor* ParentActor = StaticMeshActor->GetAttachParentActor();
				if (const AModularBuildSystemActor* ParentMBS = Cast<AModularBuildSystemActor>(ParentActor))
				{
					TArray<AActor*> Children;
					ParentMBS->GetAttachedActors(Children);
					
					UE_LOG(LogMBSEditor, Verbose, TEXT("ParentMBS=%s, Children.Num()=%d"),
						*ParentMBS->GetActorLabel(), Children.Num());
					
					for (const auto& Child : Children)
					{
						if (AStaticMeshActor* ChildStaticMeshActor = Cast<AStaticMeshActor>(Child))
						{
							UE_LOG(LogMBSEditor, VeryVerbose, TEXT("ChildStaticMeshActor=%s"), *ChildStaticMeshActor->GetActorLabel());
							FHighlightedActor NewHighlightedActor;
							HighlightActor(ChildStaticMeshActor, NewHighlightedActor);
						}
					}
				}
			}
		}
	}
}

void MBS::FEditorMode::ResetHighlightedActor(FHighlightedActor& InHighlightedActor)
{
	if (InHighlightedActor.StaticMeshActor && InHighlightedActor.StaticMeshActor->GetStaticMeshComponent() && InHighlightedActor.OriginalMaterial)
	{
		const int32 FoundIndex = HighlightedStaticMeshActors.IndexOfByPredicate([&](const FHighlightedActor& Highlighted)
		{
			return Highlighted.StaticMeshActor == InHighlightedActor.StaticMeshActor;
		});
		
		if (FoundIndex)
		{
			HighlightedStaticMeshActors.RemoveAt(FoundIndex);
			InHighlightedActor.StaticMeshActor->GetStaticMeshComponent()->SetMaterial(0, InHighlightedActor.OriginalMaterial);
			InHighlightedActor.StaticMeshActor = nullptr;
			InHighlightedActor.OriginalMaterial = nullptr;
		}
	}
}

void MBS::FEditorMode::ResetHighlightedActor(TObjectPtr<AStaticMeshActor>& InHighlightedActor,
	TObjectPtr<UMaterialInterface>& InHighlightedActorOriginalMaterial)
{
	if (InHighlightedActor && InHighlightedActor->GetStaticMeshComponent() && InHighlightedActorOriginalMaterial)
	{
		InHighlightedActor->GetStaticMeshComponent()->SetMaterial(0, InHighlightedActorOriginalMaterial);
		InHighlightedActor = nullptr;
		InHighlightedActorOriginalMaterial = nullptr;
	}
}

void MBS::FEditorMode::ResetHighlightedActor()
{
	ResetHighlightedActor(HighlightedActor, HighlightedActorOriginalMaterial);
}

void MBS::FEditorMode::HighlightActor(AStaticMeshActor* InNewHighlightedActor, FHighlightedActor& InHighlightedActor)
{
	if (UMaterialInterface* HighlightMaterial = UISettings->GetHighlightMaterial())
	{
		InHighlightedActor.OriginalMaterial = InNewHighlightedActor->GetStaticMeshComponent()->GetMaterial(0);
		InHighlightedActor.StaticMeshActor = InNewHighlightedActor;
		InHighlightedActor.StaticMeshActor->GetStaticMeshComponent()->SetMaterial(0, HighlightMaterial);
		HighlightedStaticMeshActors.Add(InHighlightedActor);
	}
}

void MBS::FEditorMode::HighlightActor(AStaticMeshActor* InNewHighlightedActor,
	TObjectPtr<AStaticMeshActor>& InHighlightedActor,
	TObjectPtr<UMaterialInterface>& InHighlightedActorOriginalMaterial)
{
	if (UMaterialInterface* HighlightMaterial = UISettings->GetHighlightMaterial())
	{
		InHighlightedActorOriginalMaterial = InNewHighlightedActor->GetStaticMeshComponent()->GetMaterial(0);
		InHighlightedActor = InNewHighlightedActor;
		InHighlightedActor->GetStaticMeshComponent()->SetMaterial(0, HighlightMaterial);
	}
}

AActor* MBS::FEditorMode::GetActorAtLocation(const FViewportCursorLocation& Location) const
{
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		Location.GetOrigin(),
		Location.GetOrigin() + Location.GetDirection() * 1000000.f,
		ECollisionChannel::ECC_Visibility))
	{
		// Find MBS actor under cursor (or a child of it)
		return Hit.GetActor();
	}
	return nullptr;
}

void MBS::FEditorMode::OnSetOperationsCommand() const
{
	UE_LOG(LogMBSEditor, Log, TEXT("Operations tool was set."));
	UISettings->SetSelectToolSelected(false);
	UISettings->SetOperationsToolSelected(true);
	OnToolChanged.Broadcast();
}

void MBS::FEditorMode::OnSetSelectCommand() const
{
	UE_LOG(LogMBSEditor, Log, TEXT("Select tool was set."));
	UISettings->SetSelectToolSelected(true);
	UISettings->SetOperationsToolSelected(false);
	OnToolChanged.Broadcast();
}
