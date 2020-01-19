// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "LukeGameAttributeSet.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChange, float, Health, float, MaxHealth);

/**
 * 
 */
UCLASS()
class LUKEGAME_API ULukeGameAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	ULukeGameAttributeSet();

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
		FGameplayAttributeData Health;

	UFUNCTION()
		void OnRep_MyAttribute()
		{
			GAMEPLAYATTRIBUTE_REPNOTIFY(ULukeGameAttributeSet, Health);
			UE_LOG(LogTemp, Warning, TEXT("Updated Health"));
		}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
		FGameplayAttributeData MaxHealth;

	void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

	FOnHealthChange OnHealthChange;
	
};
