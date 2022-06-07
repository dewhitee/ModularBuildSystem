// Fill out your copyright notice in the Description page of Project Settings.


#include "LSystem/MBSLSystem.h"

#include "LSystem/MBSLSystemGrammarPreset.h"
#include "ModularBuildSystem.h"
#include "ModularBuildSystemActor.h"
#include "ModularSectionResolution.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "LSystem/MBSLSystemAlphabet.h"

#ifndef START_RULE
#define START_RULE(Symbol) \
if (Is((#Symbol)[0])) { Symbol(Args); }
#endif

#ifndef RULE
#define RULE(Symbol) \
else if (Is((#Symbol)[0])) { Symbol(Args); }
#endif

void FMBSLSystemGrammar::Init()
{
	Symbols = "";
	SymbolsOnEachIteration.Empty();
}

void FMBSLSystemGrammar::SetSymbols(const int32 Iteration)
{
	Init();
	FString IteratedSymbols = Axiom;
	// Transforming on each iteration
	for (int32 It = 0; It < Iteration; It++)
	{
		Symbols = "";
		SymbolsOnEachIteration.Add(IteratedSymbols);
		for (int32 i = 0; i < IteratedSymbols.Len(); i++)
		{
			const FString& TransformedSymbols = TransformWithRule(static_cast<FName>(FString::Chr(IteratedSymbols[i])));
			Symbols.Append(TransformedSymbols);
			UE_LOG(LogMBSLSystem, Verbose, TEXT("SetSymbols: IteratedSymbols=%s\n, TransformedSymbols='%s'\n, i=%d\n, It=%d\n, Iteration=%d\n, Symbols=%s\n________\n"),
				*FString::Printf(TEXT("[%s]"), *IteratedSymbols), *TransformedSymbols, i, It, Iteration, *ToString());
		}
		IteratedSymbols = Symbols;
	}
}

FString FMBSLSystemGrammar::TransformWithRule(const FName CurrentSymbol)
{
	for (const auto& Rule : Rules)
	{
		if (Rule.Key == CurrentSymbol)
		{
			// Rule found - transforming current symbol into definition of a rule
			if (Rule.Value.StartsWith("="))
			{
				const FString& OutString = Rule.Value.RightChop(1);
				UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("STARTS WITH '=' (Rule.Value='%s') OutString='%s'"), *Rule.Value, *OutString);
				return OutString;
			}
			UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("OutString='%s'"), *Rule.Value);
			return Rule.Value;
		}
	}
	return CurrentSymbol.ToString();
}

FString FMBSLSystemGrammar::ToString() const
{
	return FString::Printf(TEXT("[%s]"), *Axiom);
}

TCHAR UMBSLSystem::GetCurrentSymbol() const
{
	if (Grammar.Symbols.IsValidIndex(CurrentSymbolIndex))
	{
		return Grammar.Symbols[CurrentSymbolIndex];
	}
	return '?';
}

bool UMBSLSystem::Is(const TCHAR OtherSymbol) const
{
	UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: GetCurrentSymbol()=%c, OtherSymbol=%c; Result=%s"),
		*GetName(), GetCurrentSymbol(), OtherSymbol, GetCurrentSymbol() == static_cast<TCHAR>(OtherSymbol) ? TEXT("true") : TEXT("false"));
	return GetCurrentSymbol() == static_cast<TCHAR>(OtherSymbol);
}

void UMBSLSystem::L(FMBSLSystemNextArgs& Args)
{
	ApplyRule('L', "Go left by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.Y -= UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::R(FMBSLSystemNextArgs& Args)
{
	ApplyRule('R', "Go right by DefaultSectionSize", Args, [&]
	{
		//Args.OutLocation += UKismetMathLibrary::GetRightVector(Args.OutRotation) * UModularSectionResolution::DefaultSectionSize;
		Args.OutLocation.Y += UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::F(FMBSLSystemNextArgs& Args)
{
	ApplyRule('F', "Forward by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.X += UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::B(FMBSLSystemNextArgs& Args)
{
	ApplyRule('B', "Backward by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.X -= UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::C(FMBSLSystemNextArgs& Args)
{
	ApplyRule('C', "Set rule state to SetClass", Args, [&]
	{
		SetState(Args, EMBSLSystemRuleState::SetClass);
	});
}

void UMBSLSystem::M(FMBSLSystemNextArgs& Args)
{
	ApplyRule('M', "Set rule state to SetMesh", Args, [&]
	{
		SetState(Args, EMBSLSystemRuleState::SetMesh);
	});
}

void UMBSLSystem::W(FMBSLSystemNextArgs& Args)
{
	ApplyRule('W', "Forward by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.X += UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::A(FMBSLSystemNextArgs& Args)
{
	ApplyRule('A', "Go left by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.Y -= UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::S(FMBSLSystemNextArgs& Args)
{
	ApplyRule('S', "Backward by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.X -= UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::D(FMBSLSystemNextArgs& Args)
{
	ApplyRule('D', "Go right by DefaultSectionSize", Args, [&]
	{
		Args.OutLocation.Y += UModularSectionResolution::DefaultSectionSize;
	});
}

void UMBSLSystem::Q(FMBSLSystemNextArgs& Args)
{
	ApplyRule('Q', "Spawn actor of class as a part of a room", Args, [&]
	{
		SpawnActorOfClass(Args);
	});
}

void UMBSLSystem::NonAlphas(FMBSLSystemNextArgs& Args, const TCHAR Symbol)
{
	switch (Symbol)
	{
	case '[':
		ApplyRule('[', "Save current transform", Args, [&]
		{
			*Args.SavedTransform = FTransform(Args.OutRotation, Args.OutLocation);
			Args.bSaveTransform = false;
		});
		break;
	case ']':
		ApplyRule(']', "Retrieve last saved transform", Args, [&]
		{
			Args.OutLocation = Args.SavedTransform->GetLocation();
			Args.OutRotation = Args.SavedTransform->GetRotation().Rotator();
			Args.bSaveTransform = true;
		});
		break;
	case '+':
		ApplyRule('+', "Turn right", Args, [&]
		{
			Args.OutRotation.Yaw -= 90.f;
		});
		break;
	case '-':
		ApplyRule('-', "Turn left", Args, [&]
		{
			Args.OutRotation.Yaw += 90.f;
		});
		break;
	case '.':
		ApplyRule('.', "Placeholder", Args, [&] { return; });
		break;
	case '0': ApplyRuleAtIndex(Args, 0); break;
	case '1': ApplyRuleAtIndex(Args, 1); break;
	case '2': ApplyRuleAtIndex(Args, 2); break;
	case '3': ApplyRuleAtIndex(Args, 3); break;
	case '4': ApplyRuleAtIndex(Args, 4); break;
	case '5': ApplyRuleAtIndex(Args, 5); break;
	case '6': ApplyRuleAtIndex(Args, 6); break;
	case '7': ApplyRuleAtIndex(Args, 7); break;
	case '8': ApplyRuleAtIndex(Args, 8); break;
	case '9': ApplyRuleAtIndex(Args, 9); break;
	case '!':
		ApplyRule('!', "Spawn actor of specified class with static mesh (if is of StaticMeshActor class)", Args, [&]
		{
			SpawnActorOfClass(Args);
		});
		break;
	default:
		UE_LOG(LogMBSLSystem, Warning, TEXT("%s: No rule found for symbol %c"), *GetName(), Symbol);
		break;
	}
}

void UMBSLSystem::SetState(FMBSLSystemNextArgs& Args, EMBSLSystemRuleState State)
{
	*Args.State = State;
}

void UMBSLSystem::SpawnActorOfClass(FMBSLSystemNextArgs& Args)
{
	Args.OutSpawnedActor = Args.BS->GetWorld()->SpawnActor<AActor>(
		*Args.ClassToSpawn,
		Args.OutLocation,
		Args.OutRotation);
	
	AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Args.OutSpawnedActor);
	if (StaticMeshActor && Args.MeshToSet)
	{
		StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(*Args.MeshToSet);
	}

	if (Args.OutSpawnedActor)
	{
		Args.BS->AttachActor(Args.OutSpawnedActor, false);
	}
	else
	{
		UE_LOG(LogMBSLSystem, Error, TEXT("%s: Args.OutSpawnedActor is nullptr!"), *GetName());
	}
}

void UMBSLSystem::SetClassAtIndex(FMBSLSystemNextArgs& Args, int32 Index)
{
	if (Args.ClassesToSpawn.IsValidIndex(Index) && Args.ClassesToSpawn[Index] != nullptr)
	{
		*Args.ClassToSpawn = Args.ClassesToSpawn[Index];
		UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: Class at index=%d was set; Args.ClassToSpawn=%s"), *GetName(), Index, *(*Args.ClassToSpawn)->GetName());
	}
	else if (!Args.ClassesToSpawn.IsValidIndex(Index))
	{
		UE_LOG(LogMBSLSystem, Error, TEXT("%s: Index=%d is not valid, Args.ClassesToSpawn.Num()=%d"), *GetName(), Index, Args.ClassesToSpawn.Num());
	}
	else if (Args.ClassesToSpawn[Index] == nullptr)
	{
		UE_LOG(LogMBSLSystem, Error, TEXT("%s: Class is nullptr at index=%d"), *GetName(), Index);
	}
}

void UMBSLSystem::SetMeshAtIndex(FMBSLSystemNextArgs& Args, int32 Index)
{
	if (Args.MeshesToSet.IsValidIndex(Index) && Args.MeshesToSet[Index] != nullptr)
	{
		*Args.MeshToSet = Args.MeshesToSet[Index];
		UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: Mesh at index=%d was set; Args.MeshToSet=%s"), *GetName(), Index, *(*Args.MeshToSet)->GetName());
	}
	else if (!Args.MeshesToSet.IsValidIndex(Index))
	{
		UE_LOG(LogMBSLSystem, Error, TEXT("%s: Index=%d is not valid, Args.MeshesToSet.Num()=%d"), *GetName(), Index, Args.MeshesToSet.Num());
	}
	else if (Args.MeshesToSet[Index] == nullptr)
	{
		UE_LOG(LogMBSLSystem, Error, TEXT("%s: Mesh is nullptr at index=%d"), *GetName(), Index);
	}
}

void UMBSLSystem::ApplyRuleAtIndex(FMBSLSystemNextArgs& Args, int32 Index)
{
	// Switch between rules depending on a current state
	switch (*Args.State)
	{
	case EMBSLSystemRuleState::None:
		break;
	case EMBSLSystemRuleState::SetClass:
		ApplyRuleSetClassAtIndex(Args, Index);
		break;
	case EMBSLSystemRuleState::SetMesh:
		ApplyRuleSetMeshAtIndex(Args, Index);
		break;
	default: ;
	}
}

void UMBSLSystem::ApplyRuleSetClassAtIndex(FMBSLSystemNextArgs& Args, int32 Index)
{
	ApplyRule(FString::FromInt(Index)[0], FString::Printf(TEXT("[SetClass state] Set actor class of type-%d"), Index), Args, [&]
	{
		SetClassAtIndex(Args, Index);
		SetState(Args, EMBSLSystemRuleState::None);
	});
}

void UMBSLSystem::ApplyRuleSetMeshAtIndex(FMBSLSystemNextArgs& Args, int32 Index)
{
	ApplyRule(FString::FromInt(Index)[0], FString::Printf(TEXT("[SetMesh state] Set static mesh of type-%d"), Index), Args, [&]
	{
		SetMeshAtIndex(Args, Index);
		SetState(Args, EMBSLSystemRuleState::None);
	});
}

void UMBSLSystem::ApplyRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args,
	const TFunction<void()> Rule)
{
	PreLogRule(RuleSymbol, Desc, Args);
	Rule();
	PostLogRule(RuleSymbol, Desc, Args);
}

void UMBSLSystem::LogRule(const FString& Prefix, const TCHAR RuleSymbol, const FString& Desc,
	const FMBSLSystemNextArgs& Args) const
{
	UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: %s '%c' rule - %s\n; Params: \n\tArgs.OutLocation=%s, \n\tArgs.OutRotation=%s,"
		"\n\tArgs.OutSpawnedActor=%s, \n\tArgs.ClassToSpawn=%s, \n\tArgs.MeshToSet=%s, \n\tArgs.SavedTransform.Location=%s,"
		"\n\tArgs.SavedTransform.Rotation=%s, \n\tArgs.State=%s, \n\t_"),
		*GetName(),
		*Prefix,
		RuleSymbol,
		*Desc,
		*Args.OutLocation.ToCompactString(),
		*Args.OutRotation.ToString(),
		Args.OutSpawnedActor ? *Args.OutSpawnedActor->GetName() : TEXT("nullptr"),
		Args.ClassToSpawn ? *(*Args.ClassToSpawn)->GetName() : TEXT("None"),
		Args.MeshToSet ? *(*Args.MeshToSet)->GetName() : TEXT("nullptr"),
		*Args.SavedTransform->GetLocation().ToCompactString(),
		*Args.SavedTransform->GetRotation().ToString(),
		*UEnum::GetValueAsString(*Args.State));
}

void UMBSLSystem::PreLogRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args) const
{
	LogRule(TEXT("(PRE)"), RuleSymbol, Desc, Args);
}

void UMBSLSystem::PostLogRule(const TCHAR RuleSymbol, const FString& Desc, const FMBSLSystemNextArgs& Args) const
{
	LogRule(TEXT("(POST)"), RuleSymbol, Desc, Args);
}

UMBSLSystem::UMBSLSystem()
{
	/*Grammar.SymbolDefinitions = {
		{"L", FText::FromName("Go left")},
		{"R", FText::FromName("Go right")},
		{"F", FText::FromName("Go forward")},
		{"B", FText::FromName("Go backward")},
		{"W", FText::FromName("[DEPRECATED] Forward by DefaultSectionSize")},
		{"A", FText::FromName("[DEPRECATED] Go left by DefaultSectionSize")},
		{"S", FText::FromName("[DEPRECATED] Backward by DefaultSectionSize")},
		{"D", FText::FromName("[DEPRECATED] Go right by DefaultSectionSize")},
		{"Q", FText::FromName("[DEPRECATED] Spawn actor of class as a part of a room")},
		{"C", FText::FromName("Set rule state to SetClass")},
		{"M", FText::FromName("Set rule state to SetMesh")},
		{"+", FText::FromName("Turn right")},
		{"-", FText::FromName("Turn left")},
		{"0", FText::FromName("Set mesh or actor class of type-0")},
		{"1", FText::FromName("Set mesh or actor class of type-1")},
		{"2", FText::FromName("Set mesh or actor class of type-2")},
		{"3", FText::FromName("Set mesh or actor class of type-3")},
		{"4", FText::FromName("Set mesh or actor class of type-4")},
		{"5", FText::FromName("Set mesh or actor class of type-5")},
		{"6", FText::FromName("Set mesh or actor class of type-6")},
		{"7", FText::FromName("Set mesh or actor class of type-7")},
		{"8", FText::FromName("Set mesh or actor class of type-8")},
		{"9", FText::FromName("Set mesh or actor class of type-9")},
		{"!", FText::FromName("Spawn actor as a part of a room")},
	};*/
}

void UMBSLSystem::Run(FMBSLSystemRunArgs& Args)
{
	UE_LOG(LogMBSLSystem, Log, TEXT("%s: === Running L-System with %s grammar axiom."), *GetName(), *Grammar.ToString());
	PreRun(Args);

	// Iterating to construct symbols string using specified rules
	Iterate();

	// Use MBS actor world location
	Args.OutTransform.SetLocation(Args.BS->GetActorLocation());
	Args.OutTransform.SetRotation(Args.BS->GetActorRotation().Quaternion());
	Args.OutTransform.AddToTranslation(FVector(0.f, 0.f, Args.LevelIndex * UModularSectionResolution::DefaultSectionSize));

	// Use location and rotation relative to the MBS actor
	//Args.OutTransform.SetLocation(FVector(0.f, 0.f, 0.f));
	//Args.OutTransform.SetRotation(FQuat::Identity);
	
	FTransform SavedTransform;
	EMBSLSystemRuleState State;

	AvailableClasses = Args.ClassesToSpawn;
	AvailableMeshes = Args.MeshesToSet;
	
	for (CurrentSymbolIndex = 0; CurrentSymbolIndex < Grammar.Symbols.Len(); CurrentSymbolIndex++)
	{
		FMBSLSystemNextArgs NextArgs(Args, SavedTransform, &State);
		PreSymbolConsumed(NextArgs);
		Next(NextArgs);
		PostSymbolConsumed(NextArgs);

		if (bPreserveLocation || NextArgs.bSaveTransform)
		{
			Args.OutTransform.SetLocation(NextArgs.OutLocation);
		}

		if (bPreserveRotation || NextArgs.bSaveTransform)
		{
			Args.OutTransform.SetRotation(NextArgs.OutRotation.Quaternion());
		}

		UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: Args.OutSpawnedActor=%s"), *GetName(),
			NextArgs.OutSpawnedActor ? *NextArgs.OutSpawnedActor->GetName() : TEXT("nullptr"));
		
		if (NextArgs.OutSpawnedActor)
		{
			UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: SpawnedStaticMeshActor=%s"), *GetName(), *NextArgs.OutSpawnedActor->GetName());
			Args.SpawnedActors.Add(NextArgs.OutSpawnedActor);
		}
	}
	
	PostRun(Args);
	UE_LOG(LogMBSLSystem, Log, TEXT("%s: === L-System finished."), *GetName());
}

void UMBSLSystem::SetGrammarFromPreset()
{
	if (GrammarPreset)
	{
		Grammar.Axiom = GrammarPreset->Axiom;
		Grammar.Rules = GrammarPreset->Rules;

		if (GrammarPreset->Alphabet)
		{
			Grammar.SymbolDefinitions = GrammarPreset->Alphabet->SymbolDefinitions;
		}
	}
}

void UMBSLSystem::Next_Implementation(FMBSLSystemNextArgs& Args)
{
	UE_LOG(LogMBSLSystem, VeryVerbose, TEXT("%s: Consuming next grammar symbol: %c"), *GetName(), GetCurrentSymbol());
	START_RULE(L)
	RULE(R)
	RULE(F)
	RULE(B)
	RULE(C)
	RULE(M)
	//RULE(W)
	//RULE(A)
	//RULE(S)
	//RULE(D)
	//RULE(Q)
	else
	{
		NonAlphas(Args, GetCurrentSymbol());
	}
}

void UMBSLSystem::Iterate()
{
	Grammar.SetSymbols(Iteration);
}

#ifdef START_RULE
#undef START_RULE
#endif

#ifdef RULE
#undef RULE
#endif
