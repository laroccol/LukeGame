// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeAbilityTypes.h"

FGameplayAbilityInfo::FGameplayAbilityInfo()
{
	CooldownDuration = 0.0f;
	AbilityName = FName("Default");
	UIMaterial = nullptr;
	AbilityBase = nullptr;
}

FGameplayAbilityInfo::FGameplayAbilityInfo(float InCooldownDuration, FName InAbilityName, UMaterialInstance * InUIMaterial, TSubclassOf<UGameplayAbilityBase> InAbilityBase)
{
	CooldownDuration = InCooldownDuration;
	AbilityName = InAbilityName;
	UIMaterial = InUIMaterial;
	AbilityBase = InAbilityBase;
}
