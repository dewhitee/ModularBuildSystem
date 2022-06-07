// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMBSEditor, Log, All);

namespace MBS
{
class IEditorModuleListener
{
public:
	virtual ~IEditorModuleListener() = default;
	virtual void OnStartupModule() {}
	virtual void OnShutdownModule() {}
};
}

class FModularBuildSystemEditorModule : public IModuleInterface
{
protected:
	TArray<TSharedRef<MBS::IEditorModuleListener>> ModuleListeners;
	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
	
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
	TSharedPtr<FExtender> MenuExtender;
	
	static TSharedRef<FWorkspaceItem> MenuRoot;
	
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate, FName ExtensionHook,
		const TSharedPtr<FUICommandList>& CommandList = nullptr, EExtensionHook::Position Position = EExtensionHook::Before);

	void AddModuleListeners();

	void MakePulldownMenu(FMenuBarBuilder& MenuBarBuilder);
	void FillPulldownMenu(FMenuBuilder& MenuBuilder);

	static TSharedRef<FWorkspaceItem> GetMenuRoot() { return MenuRoot; }
	
	FORCEINLINE static FModularBuildSystemEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FModularBuildSystemEditorModule>("ModularBuildSystemEditor");
	}

	FORCEINLINE static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ModularBuildSystemEditor");
	}

private:
	void RegisterVisualizers();
	void RegisterCustomTypes();
	void RegisterSettings();
	void RegisterCustomizedDetails();
	void SetupMenuExtensions();
};
