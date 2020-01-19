// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityBase.h"

FGameplayAbilityInfo UGameplayAbilityBase::GetAbilityInfo()
{
	UGameplayEffect* AbilityCooldownEffect = GetCooldownGameplayEffect();
	if (AbilityCooldownEffect)
	{
		float CooldownDuration = 0;
		AbilityCooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);

		return FGameplayAbilityInfo(CooldownDuration, AbilityName, UIMaterial, GetClass());
	}

	return FGameplayAbilityInfo();
}
