// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LukeGameGameMode.generated.h"

class ALukeGamePlayerState;

UCLASS(minimalapi)
class ALukeGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALukeGameGameMode();

	virtual bool CanDealDamage(class ALukeGamePlayerState* DamageInstigator, class ALukeGamePlayerState* DamagedPlayer) const;
};



