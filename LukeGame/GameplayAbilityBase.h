// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LukeAbilityTypes.h"
#include "GameplayAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API UGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityBase")
		UMaterialInstance* UIMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityBase")
		FName AbilityName;

	UFUNCTION(BlueprintCallable, Category = "AbilityBase")
		FGameplayAbilityInfo GetAbilityInfo();
	
};
