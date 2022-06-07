// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSActorDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "MBSFunctionLibrary.h"
#include "MBSPresetManagerInterface.h"
#include "ModularBuildSystemActor.h"
#include "ModularBuildSystemEditor.h"
#include "Interior/MBSInterior.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "MBSActorDetails"

void MBS::FActorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	//RegisterSectionMappings(PropertyModule);
	AddCallInEditorMethods(DetailBuilder);
}

void MBS::FActorDetails::RegisterSectionMappings(FPropertyEditorModule& PropertyEditorModule)
{
	const FName ClassName = AModularBuildSystemActor::StaticClass()->GetFName();
	
	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "Generator", LOCTEXT("Generator", "Generator"));
		Section->AddCategory("Generator");
	}

	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "Interior", LOCTEXT("Interior", "Interior"));
		Section->AddCategory("Interior");
	}

	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "MBS", LOCTEXT("MBS", "MBS"));
		Section->AddCategory("ModularBuildSystem");
		Section->AddCategory("Bounds");
	}

	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "Presets", LOCTEXT("Presets", "Presets"));
		Section->AddCategory("Presets");
	}

	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "Operations", LOCTEXT("Operations", "Operations"));
		Section->AddCategory("Selection");
		Section->AddCategory("Merge");
		Section->AddCategory("Stretch");
		Section->AddCategory("Display");
	}

	{
		const TSharedRef<FPropertySection> Section = PropertyEditorModule.FindOrCreateSection(ClassName, "Config", LOCTEXT("Config", "Config"));
		Section->AddCategory("Config");
	}
}

void MBS::FActorDetails::AddCallInEditorMethods(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}

	TWeakObjectPtr<AModularBuildSystemActor> BS = Cast<AModularBuildSystemActor>(Objects[0].Get());
	TArray<FButtonEntry> Buttons = GetButtons(BS);
	
	struct FCategoryEntry // @see FCategoryEntry in ObjectDetails.cpp (AddCallInEditorMethods method of FObjectDetails class)
	{
		FName CategoryName;
		FName RowTag;
		TSharedPtr<SWrapBox> WrapBox;
		FTextBuilder FunctionSearchText;

		FCategoryEntry(const FName InCategoryName)
			: CategoryName(InCategoryName)
		{
			WrapBox = SNew(SWrapBox).UseAllottedSize(true);
		}
	};
	
	FName ActiveCategory;
	TArray<FCategoryEntry, TInlineAllocator<8>> CategoryList;
	
	for (FButtonEntry& Button : Buttons)
	{
		const FText ButtonCaption = FText::FromString(FName::NameToDisplayString(Button.ButtonName.ToString(), false));
		if (Button.CategoryName != ActiveCategory)
		{
			ActiveCategory = Button.CategoryName;
			CategoryList.Emplace(ActiveCategory);
		}
		FCategoryEntry& CategoryEntry = CategoryList.Last();
		
		CategoryEntry.WrapBox->AddSlot()
		.Padding(0.f, 0.f, 5.f, 3.f)
		[
			SNew(SButton)
			.Text(ButtonCaption)
			.OnClicked_Lambda(Button.OnClick)
			.ToolTipText(Button.Tooltip)
		];

		CategoryEntry.RowTag = Button.ButtonName;
		CategoryEntry.FunctionSearchText.AppendLine(ButtonCaption);
		CategoryEntry.FunctionSearchText.AppendLine(Button.Tooltip);
	}

	for (FCategoryEntry& Entry : CategoryList)
	{
		IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory(Entry.CategoryName);
		CategoryBuilder.AddCustomRow(Entry.FunctionSearchText.ToText())
		.RowTag(Entry.RowTag)
		[
			Entry.WrapBox.ToSharedRef()
		];
	}
}

TArray<MBS::FActorDetails::FButtonEntry> MBS::FActorDetails::GetButtons(TWeakObjectPtr<AModularBuildSystemActor> BS) const
{
	TArray<FButtonEntry> Buttons;

	auto OnInit = [BS]
	{
		if (BS.IsValid())
		{
			BS->Init();
		}
		return FReply::Handled();
	};
	
	auto OnBuild = [BS]
	{
		if (BS.IsValid())
		{
			BS->SetBuildModeActivated(!BS->IsBuildModeActivated());

			UE_LOG(LogMBSEditor, Log, TEXT("%s: %s"), *BS->GetName(),
				(BS->IsBuildModeActivated() ? TEXT("Building mode activated") : TEXT("Building mode deactivated")));
		}
		return FReply::Handled();
	};
	
	auto OnClear = [BS]
	{
		if (BS.IsValid())
		{
			UMBSFunctionLibrary::ForEachLevel(BS->GetAllLevels(), [&](FModularLevel& InLevel) { InLevel = FModularLevel(); });
		}
		return FReply::Handled();
	};

	auto OnReset = [BS]
	{
		if (BS.IsValid())
		{
			BS->ResetBuildSystem();
		}
		return FReply::Handled();
	};

	auto OnFixLevelIds = [BS]
	{
		if (BS.IsValid())
		{
			UE_LOG(LogMBSEditor, Log, TEXT("%s: Fixing level ids"), *BS->GetName());
			for (FModularLevel* Level : BS->GetAllLevels())
			{
				BS->LevelInitializer.FixLevelId(Level);
			}
		}
		return FReply::Handled();
	};
	
	Buttons.Emplace(FButtonEntry("ModularBuildSystem", "Init", TEXT("Initializes all modular levels."), OnInit));
	Buttons.Emplace(FButtonEntry("ModularBuildSystem", "Build", TEXT("Activates the build mode for this modular build system actor.\n@see bBuildModeIsActivated for more info about build mode."), OnBuild));
	Buttons.Emplace(FButtonEntry("ModularBuildSystem", "Clear", TEXT("Empties (set to default) all modular levels of this modular build system actor."), OnClear));
	Buttons.Emplace(FButtonEntry("ModularBuildSystem", "Reset", TEXT("Clears all arrays of modular sections, including instanced modular sections and modular section actors, destroying all actors that were spawned and instanced static mesh components associated with each modular level."), OnReset));
	Buttons.Emplace(FButtonEntry("ModularBuildSystem", "FixLevelIds", TEXT("Fixes all invalid IDs of this MBS actor modular levels. Does nothing if all IDs are valid."), OnFixLevelIds));
	
	auto OnGenerateInterior = [BS]
	{
		UE_LOG(LogMBSEditor, Log, TEXT("%s: Generating interior."), *BS->GetActorLabel());
		if (const TWeakObjectPtr<UMBSInterior> Interior = BS->GetInterior(); Interior.IsValid())
		{
			Interior->GenerateInterior(BS.Get());
		}
		else
		{
			UE_LOG(LogMBSEditor, Error, TEXT("%s: Can't generate interior as Interior object is not provided."), *BS->GetActorLabel());
		}
		return FReply::Handled();
	};

	auto OnResetInterior = [BS]
	{
		UE_LOG(LogMBSEditor, Log, TEXT("%s: Resetting interior."), *BS->GetActorLabel());
		if (const TWeakObjectPtr<UMBSInterior> Interior = BS->GetInterior(); Interior.IsValid())
		{
			Interior->ResetInterior();
		}
		else
		{
			UE_LOG(LogMBSEditor, Error, TEXT("%s: Can't reset interior as Interior object is not provided."), *BS->GetActorLabel());
		}
		return FReply::Handled();
	};
		
	auto OnUpdateInterior = [BS]
	{
		UE_LOG(LogMBSEditor, Log, TEXT("%s: Updating interior."), *BS->GetActorLabel());
		if (const TWeakObjectPtr<UMBSInterior> Interior = BS->GetInterior(); Interior.IsValid())
		{
			Interior->UpdateInterior();
		}
		return FReply::Handled();
	};
		
	auto OnApplyInteriorPresets = [BS]
	{
		UE_LOG(LogMBSEditor, Log, TEXT("%s: Applying interior presets."), *BS->GetActorLabel());
		if (const TWeakObjectPtr<UMBSInterior> Interior = BS->GetInterior(); Interior.IsValid())
		{
			Interior->ApplyInteriorPreset();
		}
		return FReply::Handled();
	};
		
	Buttons.Emplace(FButtonEntry("Interior", "GenerateInterior", TEXT("Generates interior if Interior object is provided."), OnGenerateInterior));
	Buttons.Emplace(FButtonEntry("Interior", "ResetInterior", TEXT("Destroys all interior actors that were spawned by this MBS actor Interior object."), OnResetInterior));
	Buttons.Emplace(FButtonEntry("Interior", "UpdateInterior", TEXT("Updates interior."), OnUpdateInterior));
	Buttons.Emplace(FButtonEntry("Interior", "ApplyInteriorPresets", TEXT("Applies interior presets."), OnApplyInteriorPresets));
	
	auto OnResetStretchCoefficients = [BS]
	{
		UE_LOG(LogMBSEditor, Log, TEXT("%s: Resetting stretch coefficients."), *BS->GetActorLabel());
		BS->StretchManager.ResetScaleCoefficients();
		return FReply::Handled();
	};

	Buttons.Emplace(FButtonEntry("Stretch", "ResetStretchCoefficients", TEXT(""), OnResetStretchCoefficients));

	auto OnLoadPreset = [BS]
	{
		if (BS.IsValid())
		{
			IMBSPresetManagerInterface::Execute_LoadPreset(BS->GetPresetManager().GetObject());
		}
		return FReply::Handled();
	};

	auto OnSavePreset = [BS]
	{
		if (BS.IsValid())
		{
			if (!BS->GetPresetManager().GetObject())
			{
				UE_LOG(LogMBSEditor, Error, TEXT("%s: Preset manager is not set!"), *BS->GetActorLabel());
			}
			IMBSPresetManagerInterface::Execute_SavePreset(BS->GetPresetManager().GetObject());
		}
		return FReply::Handled();
	};

	Buttons.Emplace(FButtonEntry("Presets", "LoadPreset", TEXT("Loads MBS actor preset."), OnLoadPreset));
	Buttons.Emplace(FButtonEntry("Presets", "SavePreset", TEXT("Saves all modular levels of this MBS actor into a preset."), OnSavePreset));
	
	return Buttons;
}

#undef LOCTEXT_NAMESPACE
