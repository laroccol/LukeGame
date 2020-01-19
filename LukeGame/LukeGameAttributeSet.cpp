// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGameAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

ULukeGameAttributeSet::ULukeGameAttributeSet()
{
	Health = 200.0f;
	MaxHealth = 200.0f;
}

void ULukeGameAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
	if (Data.EvaluatedData.Attribute.GetUProperty() == FindFieldChecked<UProperty>(ULukeGameAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(ULukeGameAttributeSet, Health)))
	{
		UE_LOG(LogTemp, Warning, TEXT("I took damage my health is now %f"), Health.GetCurrentValue());
		OnHealthChange.Broadcast(Health.GetCurrentValue(), MaxHealth.GetCurrentValue());
	}
}

void ULukeGameAttributeSet::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME( UMyAttributeSet, MyAttribute); Chances are this is how you would ordinarily do it, however in the case of attributes this'll lead to confusing and annoying replication errors, usually involving clientside ability prediction.
	//DOREPLIFETIME_CONDITION_NOTIFY(ULukeGameAttributeSet, Health, COND_None, REPNOTIFY_Always); //This is how it is done properly for attributes. 
}

