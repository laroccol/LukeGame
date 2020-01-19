// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Engine/Canvas.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "LukeGameDamageType.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API ULukeGameDamageType : public UDamageType
{
	GENERATED_BODY()

public:

	ULukeGameDamageType(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = HUD)
		FCanvasIcon KillIcon;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UForceFeedbackEffect* HitForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UForceFeedbackEffect* KilledForceFeedback;
	
};
