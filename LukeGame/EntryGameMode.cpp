// Fill out your copyright notice in the Description page of Project Settings.


#include "EntryGameMode.h"
#include "LukeGame.h"
#include "Engine.h"
#include "EngineGlobals.h"
#include "LukeGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEntryGameMode::AEntryGameMode()
	: Super()
{

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/MenuPlayerController"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}