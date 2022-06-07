// Fill out your copyright notice in the Description page of Project Settings.


#include "MBSGeneratorProperty.h"

#include "List/ModularBuildSystemActorList.h"
#include "List/ModularBuildSystemMeshList.h"
#include "ModularBuildSystem.h"

bool MBS::FGenPropertyValidator::Validate()
{
	bool bResult = true;
	Errors.Empty();
	for (const auto& Property : Properties)
	{
		if (!IsValid(Property.Key))
		{
			const FText& ErrorText = FText::FromString(FString::Printf(TEXT("%s: %s"), *Property.Value, *Text.ToString()));
			UE_LOG(LogMBSProperty, Error, TEXT("%s"), *ErrorText.ToString());
			Errors.Add(ErrorText);
			bResult &= false;
		}
	}
	return bResult;
}

void UMBSGeneratorProperty::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		const auto StructProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
		const auto StructTail = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
		const FString StructProp2 = PropertyChangedEvent.Property->GetName();
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();
		const FName StructName = StructProperty->GetFName();
		UE_LOG(LogGenerator, Log, TEXT("%s: StructName=%s, PropertyName=%s, StructTail=%s, StructProp2=%s"), *GetName(),
			*StructName.ToString(), *PropertyName.ToString(), *StructTail->GetName(), *StructProp2);
		if (!Data.Resolution
			&& (PropertyName == GET_MEMBER_NAME_CHECKED(UMBSGeneratorProperty, Data.MeshList)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UMBSGeneratorProperty, Data.ActorList)
			|| PropertyName == "MeshList"
			|| PropertyName == "ActorList"))
		{
			if (Data.bUseActorList)
			{
				if (Data.ActorList && !Data.ActorList->Elements.IsEmpty())
				{
					TArray<UModularSectionResolution*> Keys;
					Data.ActorList->Elements.GetKeys(Keys);
					Data.Resolution = Keys[0];
				}
			}
			else
			{
				if (Data.MeshList && !Data.MeshList->Elements.IsEmpty())
				{
					TArray<UModularSectionResolution*> Keys;
					Data.MeshList->Elements.GetKeys(Keys);
					Data.Resolution = Keys[0];
				}
			}
		}
	}
	UObject::PostEditChangeChainProperty(PropertyChangedEvent);
}
