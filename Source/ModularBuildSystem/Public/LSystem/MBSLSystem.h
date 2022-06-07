// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MBSLSystem.generated.h"

class UMBSLSystemAlphabet;
class UMBSLSystemGrammarPreset;
class AModularBuildSystemActor;

UENUM(BlueprintType)
enum class EMBSLSystemRuleState : uint8
{
	None,
	SetClass,
	SetMesh
};

USTRUCT(BlueprintType)
struct FMBSLSystemRule
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName Symbol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FText Description;
};

USTRUCT(BlueprintType)
struct FMBSLSystemGrammar
{
	GENERATED_BODY()

	/**
	 * Initial state of an L-System.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Axiom;
	
	/**
	 * Symbols that are generated using iterations on Axiom with specified Rules.
	 * @see Axiom
	 * @see Rules
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Symbols;
	
	/**
	 * Map of user defined rules where Key is a single symbol and Value is a string that defines a rule for it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Rules;
	
	/**
	 * Array of symbols created on each iteration.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AdvancedDisplay))
	TArray<FString> SymbolsOnEachIteration;
	
	/**
	 * Definitions of all symbols available to the L-System.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(TitleProperty="Description", AdvancedDisplay))
	TMap<FName, FText> SymbolDefinitions;

	/**
	 * Definitions of all symbols available to the L-System.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMBSLSystemAlphabet> Alphabet;

	void Init();
	void SetSymbols(const int32 Iteration);
	FString TransformWithRule(const FName CurrentSymbol);
	
	FString ToString() const;
};

struct FMBSLSystemRunArgs
{
	TObjectPtr<AModularBuildSystemActor> BS;
	FTransform OutTransform;
	TSubclassOf<AActor> ClassToSpawn;
	TArray<TSubclassOf<AActor>> ClassesToSpawn;
	TObjectPtr<UStaticMesh> MeshToSet;
	TArray<UStaticMesh*> MeshesToSet;
	mutable TArray<AActor*> SpawnedActors;
	int32 LevelIndex;
};

USTRUCT(BlueprintType)
struct FMBSLSystemNextArgs
{
	GENERATED_BODY()

	FMBSLSystemNextArgs()
		: BS(nullptr)
		, MeshToSet(nullptr)
		, MeshesToSet(TArray<UStaticMesh*>())
		, OutSpawnedActor(nullptr)
		, bSaveTransform(false) {}

	FMBSLSystemNextArgs(FMBSLSystemRunArgs& RunArgs, FTransform& InSavedTransform, EMBSLSystemRuleState* InState)
		: BS(RunArgs.BS)
		, ClassToSpawn(&RunArgs.ClassToSpawn)
		, ClassesToSpawn(RunArgs.ClassesToSpawn)
		, MeshToSet(&RunArgs.MeshToSet)
		, MeshesToSet(RunArgs.MeshesToSet)
		, OutLocation(RunArgs.OutTransform.GetLocation())
		, OutRotation(RunArgs.OutTransform.GetRotation())
		, OutSpawnedActor(nullptr)
		, bSaveTransform(false)
		, SavedTransform(&InSavedTransform)
		, State(InState) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AModularBuildSystemActor> BS;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor>* ClassToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> ClassesToSpawn;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh>* MeshToSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMesh*> MeshesToSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	mutable FVector OutLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	mutable FRotator OutRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	mutable AActor* OutSpawnedActor;
	
	bool bSaveTransform;
	FTransform* SavedTransform;
	EMBSLSystemRuleState* State;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta=(DisplayName="L-System"))
class MODULARBUILDSYSTEM_API UMBSLSystem : public UObject
{
	GENERATED_BODY()

public:	
	UMBSLSystem();

private:	
	UPROPERTY(EditAnywhere, Category="L-System")
	FMBSLSystemGrammar Grammar;

	UPROPERTY(EditAnywhere, Category="L-System")
	int32 Iteration;

	UPROPERTY(VisibleAnywhere, Category="L-System")
	int32 CurrentSymbolIndex;

	UPROPERTY(EditAnywhere, Category="L-System")
	bool bPreserveLocation;

	UPROPERTY(EditAnywhere, Category="L-System")
	bool bPreserveRotation;

	UPROPERTY(VisibleAnywhere, Category="L-System")
	TArray<TSubclassOf<AActor>> AvailableClasses;
	
	UPROPERTY(VisibleAnywhere, Category="L-System")
	TArray<UStaticMesh*> AvailableMeshes;

	UPROPERTY(EditAnywhere, Category="L-System")
	TObjectPtr<UMBSLSystemGrammarPreset> GrammarPreset;

public:	
	void Run(FMBSLSystemRunArgs& Args);
	void SetGrammarFromPreset();
	
protected:
	virtual bool PreRun(FMBSLSystemRunArgs& Args) { return true; }
	virtual bool PostRun(FMBSLSystemRunArgs& Args) { return true; }
	
	virtual void PreSymbolConsumed(FMBSLSystemNextArgs& Args) {}
	virtual void PostSymbolConsumed(FMBSLSystemNextArgs& Args) {}
	
	UFUNCTION(BlueprintNativeEvent, Category="L-System")
	void Next(UPARAM(Ref) FMBSLSystemNextArgs& Args);
	virtual void Next_Implementation(FMBSLSystemNextArgs& Args);

	void Iterate();

	TCHAR GetCurrentSymbol() const;
	bool Is(const TCHAR OtherSymbol) const;
	
	// Rules
	virtual void L(FMBSLSystemNextArgs& Args);
	virtual void R(FMBSLSystemNextArgs& Args);
	virtual void F(FMBSLSystemNextArgs& Args);
	virtual void B(FMBSLSystemNextArgs& Args);
	virtual void C(FMBSLSystemNextArgs& Args);
	virtual void M(FMBSLSystemNextArgs& Args);
	virtual void W(FMBSLSystemNextArgs& Args);
	virtual void A(FMBSLSystemNextArgs& Args);
	virtual void S(FMBSLSystemNextArgs& Args);
	virtual void D(FMBSLSystemNextArgs& Args);
	virtual void Q(FMBSLSystemNextArgs& Args);
	virtual void NonAlphas(FMBSLSystemNextArgs& Args, const TCHAR Symbol);

private:
	static void SetState(FMBSLSystemNextArgs& Args, EMBSLSystemRuleState State);
	void SpawnActorOfClass(FMBSLSystemNextArgs& Args);
	void SetClassAtIndex(FMBSLSystemNextArgs& Args, int32 Index);
	void SetMeshAtIndex(FMBSLSystemNextArgs& Args, int32 Index);
	void ApplyRuleAtIndex(FMBSLSystemNextArgs& Args, int32 Index);
	void ApplyRuleSetClassAtIndex(FMBSLSystemNextArgs& Args, int32 Index);
	void ApplyRuleSetMeshAtIndex(FMBSLSystemNextArgs& Args, int32 Index);
	void ApplyRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args, const TFunction<void()> Rule);
	void LogRule(const FString& Prefix, const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args) const;
	void PreLogRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args) const;
	void PostLogRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args) const;
};
