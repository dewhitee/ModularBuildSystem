// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ModularSection.h"
#include "ModularSectionResolution.h"
#include "MBSGeneratorBase.generated.h"

class IModularBuildSystemInterface;
struct FModularLevel;
class UMBSGeneratorProperty;
class UMBSIndexCalculation;
class AModularBuildSystemActor;
class UModularBuildSystemListBase;
class UModularBuildSystemMeshList;
class UModularBuildSystemActorList;
class UModularSectionResolution;

USTRUCT(BlueprintType)
struct FMBSGeneratorPropertyData
{
	GENERATED_BODY()

	FMBSGeneratorPropertyData();

	UPROPERTY(VisibleAnywhere, Category = "Generator")
	FName DebugPropertyName = FName(NAME_None);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
	bool bUseActorList = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator", meta = (EditCondition = "!bUseActorList"))
	TObjectPtr<UModularBuildSystemMeshList> MeshList = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator", meta = (EditCondition = "bUseActorList"))
	TObjectPtr<UModularBuildSystemActorList> ActorList = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
	TObjectPtr<UModularSectionResolution> Resolution = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Generator")
	TObjectPtr<UMBSIndexCalculation> IndexCalculation = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Generator", meta = (InlineEditConditionToggle))
	bool bOverridePivotLocation = false;

	UPROPERTY(EditDefaultsOnly, Category = "Generator", meta = (EditCondition = "bOverridePivotLocation"))
	EModularSectionPivotLocation PivotLocationOverride;

	// TODO: Add functionality
	UPROPERTY(EditDefaultsOnly, Category = "Generator", meta = (InlineEditConditionToggle))
	bool bOverrideResolutionSnapMode = false;

	// TODO: Add functionality
	UPROPERTY(EditDefaultsOnly, Category="Generator", meta=(EditCondition="bOverrideResolutionSnapMode"))
	EModularSectionResolutionSnapMode SnapModeOverride;
	
	/**
	 * @brief Gets static mesh of certain resolution from mesh list at specified index.
	 * @param AtIndex Index of mesh list element.
	 * @param OverrideResolution (Optional) Custom modular section resolution.
	 * If nullptr - default Resolution property will be used.
	 * @return Static mesh from mesh list.
	 */
	UStaticMesh* GetMesh(int32 AtIndex, UModularSectionResolution* OverrideResolution = nullptr) const;

	/**
	 * @brief Gets static mesh of certain resolution from mesh list at random index.
	 * @param OverrideResolution (Optional) Custom modular section resolution.
	 * @return Random static mesh from mesh list.
	 */
	UStaticMesh* GetRandomMesh(UModularSectionResolution* OverrideResolution = nullptr) const;

	/**
	 * @brief Gets actor class of certain resolution from actor list at specified index. 
	 * @param AtIndex Index of actor list element.
	 * @param OverrideResolution (Optional) Custom modular section resolution. 
	 * @return Actor class from actor list.
	 */
	TSubclassOf<AActor> GetActorClass(int32 AtIndex, UModularSectionResolution* OverrideResolution = nullptr) const;
	
	/**
	 * @brief Gets actor class of certain resolution from actor list at random index. 
	 * @param OverrideResolution (Optional) Custom modular section resolution. 
	 * @return Random actor class from actor list. 
	 */
	TSubclassOf<AActor> GetRandomActorClass(UModularSectionResolution* OverrideResolution = nullptr) const;

	int32 GetIndex(const struct FMBSIndexCalculationArgs& Args, int32 Default = 0) const;

	// TODO: Add IsSet method returning bool, that validates if all necessary data of this structure is set
	bool IsSet() const;
};

/**
 * Not used because container class can't use this class fields in metadata specifiers.
 */
USTRUCT(BlueprintType)
struct FMBSGeneratorDataToggle
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Generator")
	bool bSupported = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator", meta=(EditCondition = bSupported))
	bool bEnabled = false;
};

/**
 * Deprecated. Should not be used anymore.
 */
USTRUCT(BlueprintType)
struct FMBSGeneratorInstanceProperty
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator")
	FVector Offset = {};
};

// TODO: Rename and use as optional GP data or remove in favor of UMBSProperty
/**
 * Deprecated. Should not be used anymore - use UMBSGeneratorProperty derived classes instead.
 */
USTRUCT(BlueprintType)
struct FMBSGeneratorProperty_Old
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Generator")
	bool bSupported = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Generator",
		meta=(EditCondition = "bSupported", EditConditionHides))
	bool bEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator",
		meta = (EditCondition = "bSupported", EditConditionHides))
	FMBSGeneratorPropertyData Data;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Generator",
	meta = (EditCondition = "bEnabled", EditConditionHides))
	FMBSGeneratorInstanceProperty Params;
};

/**
 * Base class of all generators from ModularBuildSystem plugin.
 */
UCLASS(Abstract)
class MODULARBUILDSYSTEM_API UMBSGeneratorBase : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Sets build system pointer of this generator. Should be overridden by subclasses.
	 * @param InBuildSystemPtr Pointer to modular build system derived instance.
	 * @return True if build system pointer was set and successfully casted to required subclass.
	 */
	virtual bool SetBuildSystemPtr(TScriptInterface<IModularBuildSystemInterface> InBuildSystemPtr)
	{
		unimplemented();
		return false;
	};

	/**
	 * Gets pointer to a modular build system of this generator.
	 * @return Pointer to a modular build system actor.
	 */
	UFUNCTION(BlueprintCallable, Category=Generator)
	virtual TScriptInterface<IModularBuildSystemInterface> GetBuildSystemPtr() const
	{
		unimplemented();
		return nullptr;
	}

protected:
	/**
	 * Prints all post-generation information to the console.
	 */
	virtual void LogGenerationSummary() const;

	virtual bool CheckProperties() const;
	virtual bool CheckBounds() const;
	virtual bool CheckLists() const;
	virtual bool CheckResolutions() const;

	virtual bool CanGenerate() const;

	void CheckPropertyFull(const FName InPropertyName, const UMBSGeneratorProperty* InProperty, const bool bSupported, bool& bOutResult) const;
	void CheckProperty(const FName InPropertyName, const UMBSGeneratorProperty* InProperty, const bool bSupported, bool& bOutResult) const;

	struct FCheckPropertyEntry
	{
		FName PropertyName;
		UMBSGeneratorProperty* Property;
		bool bIsSupported;
	};
	void CheckProperty(const TArray<FCheckPropertyEntry>& CheckPropertyEntries, bool& bOutResult) const;

	void CheckList(const FName InMeshListName, const UModularBuildSystemListBase* InMeshList, const bool bIsSupported,
		bool& bOutResult) const;

	struct FCheckListEntry
	{
		FName MeshListName;
		UModularBuildSystemListBase* MeshList;
		bool bIsSupported;
	};
	void CheckList(const TArray<FCheckListEntry>& CheckListEntries, bool& bOutResult) const;
	
	void CheckResolution(const FName InResolutionName, const UModularSectionResolution* InResolution,
		const bool bIsSupported, bool& bOutResult) const;

	struct FCheckResolutionEntry
	{
		FName ResolutionName;
		UModularSectionResolution* Resolution;
		bool bIsSupported;
	};
	void CheckResolution(const TArray<FCheckResolutionEntry>& ResolutionEntries, bool& bOutResult) const;

	static EModularSectionPivotLocation GetPivotLocation(const FMBSGeneratorPropertyData& LevelData,
		const FModularLevel& Level);
};
