// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularBuildSystemEditor.h"

#include "AssetToolsModule.h"
#include "ComponentVisualizer.h"
#include "Visualizer/HouseVisualizationComponent.h"
#include "Visualizer/HouseVisualizer.h"
#include "IAssetTools.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "MBSActorDetails.h"
#include "Visualizer/MBSActorVisualizationComponent.h"
#include "Visualizer/MBSActorVisualizer.h"
#include "EditorMode/MBSEditorModeTool.h"
#include "MBSMenuTool.h"
#include "Preset/MBSPresetData.h"
#include "Config/MBSSettings.h"
#include "MBSTabTool.h"
#include "ModularBuildSystemActor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FModularBuildSystemEditorModule"

DEFINE_LOG_CATEGORY(LogMBSEditor);

TSharedRef<FWorkspaceItem> FModularBuildSystemEditorModule::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root"));

void FModularBuildSystemEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(LogMBSEditor, Warning, TEXT("ModularBuildSystemEditor module has been loaded."));
	if (GUnrealEd)
	{
		RegisterVisualizers();

		// Setup menu extensions
		if (!IsRunningCommandlet())
		{
			SetupMenuExtensions();
		}

		// Register custom asset types
		RegisterCustomTypes();

		RegisterSettings();
		RegisterCustomizedDetails();
	}
	else
	{
		UE_LOG(LogMBSEditor, Error, TEXT("GUnrealEd was nullptr."));
	}
}

void FModularBuildSystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UMBSActorVisualizationComponent::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UHouseVisualizationComponent::StaticClass()->GetFName());

		for (int32 i = 0; i < ModuleListeners.Num(); i++)
		{
			ModuleListeners[i]->OnShutdownModule();
		}
	}
}

void FModularBuildSystemEditorModule::AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate,
	FName ExtensionHook, const TSharedPtr<FUICommandList>& CommandList, EExtensionHook::Position Position)
{
	MenuExtender->AddMenuExtension(ExtensionHook, Position, CommandList, ExtensionDelegate);
}

void FModularBuildSystemEditorModule::AddModuleListeners()
{
	ModuleListeners.Add(MakeShareable(new MBS::FMenuTool));
	ModuleListeners.Add(MakeShareable(new MBS::FTabTool));
	ModuleListeners.Add(MakeShareable(new MBS::FEditorModeTool));
}

void FModularBuildSystemEditorModule::MakePulldownMenu(FMenuBarBuilder& MenuBarBuilder)
{
	MenuBarBuilder.AddPullDownMenu(
		FText::FromString("MBS"),
		FText::FromString("Open the MBS menu"),
		FNewMenuDelegate::CreateRaw(this, &FModularBuildSystemEditorModule::FillPulldownMenu),
		"MBS",
		FName(TEXT("MBSMenu")));
}

void FModularBuildSystemEditorModule::FillPulldownMenu(FMenuBuilder& MenuBuilder)
{
	// Frame for tools to fill in
	MenuBuilder.BeginSection("MBSSection", FText::FromString("Section 1"));
	MenuBuilder.AddMenuSeparator(FName("Section_1"));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("MBSSection", FText::FromString("Section 2"));
	MenuBuilder.AddMenuSeparator(FName("Section_2"));
	MenuBuilder.EndSection();
}

void FModularBuildSystemEditorModule::RegisterVisualizers()
{
	// Make new instances of the visualizers
	const TSharedPtr<FComponentVisualizer> MBSVisualizer = MakeShareable(new MBS::FActorVisualizer());
	const TSharedPtr<FComponentVisualizer> HouseVisualizer = MakeShareable(new MBS::FHouseVisualizer());

	// Register it to our specific component class
	GUnrealEd->RegisterComponentVisualizer(UMBSActorVisualizationComponent::StaticClass()->GetFName(), MBSVisualizer);
	GUnrealEd->RegisterComponentVisualizer(UHouseVisualizationComponent::StaticClass()->GetFName(), HouseVisualizer);
		
	MBSVisualizer->OnRegister();
	HouseVisualizer->OnRegister();

	UE_LOG(LogMBSEditor, Warning, TEXT("UMBSActorVisualizationComponent component visualizer has been registered."));
	UE_LOG(LogMBSEditor, Warning, TEXT("UHouseVisualizationComponent component visualizer has been registered."));
}

void FModularBuildSystemEditorModule::RegisterCustomTypes()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Add custom category
	const EAssetTypeCategories::Type Category =
		AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("MBS")), FText::FromString("ModularBuildSystem"));

	// Register custom assets with MBS category
	const TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FMBSPresetDataActions(Category));
	AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());

	// Save to unregister later
	CreatedAssetTypeActions.Add(Action);
}

void FModularBuildSystemEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		TSharedPtr<ISettingsContainer> ProjectSettingsContainer = SettingsModule->GetContainer("Project");
		ProjectSettingsContainer->DescribeCategory(
			"MBSCategory",
			FText::FromString("MBS Category"),
			FText::FromString("MBS settings description here"));

		SettingsModule->RegisterSettings(
			"Project",
			"MBSCategory",
			"MBSSettings",
			FText::FromString("MBS Settings"),
			FText::FromString("Configure MBS Settings"),
			GetMutableDefault<UMBSSettings>());
	}
}

void FModularBuildSystemEditorModule::RegisterCustomizedDetails()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(
		AModularBuildSystemActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&MBS::FActorDetails::MakeInstance));
	
	MBS::FActorDetails::RegisterSectionMappings(PropertyEditorModule);
}

void FModularBuildSystemEditorModule::SetupMenuExtensions()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension("Window", EExtensionHook::After, nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FModularBuildSystemEditorModule::MakePulldownMenu));
	LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);

	// Add module listeners
	AddModuleListeners();
	for (int32 i = 0; i < ModuleListeners.Num(); i++)
	{
		ModuleListeners[i]->OnStartupModule();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModularBuildSystemEditorModule, ModularBuildSystemEditor)