// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LukeAbilityTypes.h"
#include "LukeGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API ALukeGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ALukeGamePlayerController(const FObjectInitializer& ObjectInitializer);

	virtual void UnFreeze() override;

	bool IsGameInputAllowed() const;

	bool IsVibrationEnabled() const;

	void SetIsVibrationEnabled(bool bEnable);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,  Category = "PlayerController")
		void AddAbilityToUI(FGameplayAbilityInfo AbilityInfo);

protected:

	uint8 bIsVibrationEnabled : 1;
	
};
