// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LukeGameSaveGame.generated.h"

class ALukeGameAbility;

/**
 * 
 */
UCLASS()
class LUKEGAME_API ULukeGameSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	ULukeGameSaveGame();
};
