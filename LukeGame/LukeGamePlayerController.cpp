// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGamePlayerController.h"
#include "LukeGameCharacter.h"

ALukeGamePlayerController::ALukeGamePlayerController(const FObjectInitializer& ObjectInitializer)
{

}

bool ALukeGamePlayerController::IsGameInputAllowed() const
{
	return true;
}

void ALukeGamePlayerController::UnFreeze()
{
	ServerRestartPlayer();
}

bool ALukeGamePlayerController::IsVibrationEnabled() const
{
	return bIsVibrationEnabled;
}

void ALukeGamePlayerController::SetIsVibrationEnabled(bool bEnable)
{
	bIsVibrationEnabled = bEnable;
}
