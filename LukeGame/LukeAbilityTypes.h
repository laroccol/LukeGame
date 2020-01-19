// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LukeAbilityTypes.generated.h"

class UGameplayAbilityBase;
class UMaterialInstance;

USTRUCT(BlueprintType)
struct FGameplayAbilityInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityInfo")
		float CooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityInfo")
		FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityInfo")
		UMaterialInstance* UIMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityInfo")
		TSubclassOf<UGameplayAbilityBase> AbilityBase;

	FGameplayAbilityInfo();
	FGameplayAbilityInfo(float InCooldownDuration, FName InAbilityName, UMaterialInstance* InUIMaterial, TSubclassOf<UGameplayAbilityBase> InAbilityBase);
};
