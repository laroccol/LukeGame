// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGamePlayerState.h"
#include "LukeGameCharacter.h"
#include "LukeGamePlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ALukeGamePlayerState::ALukeGamePlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void ALukeGamePlayerState::Reset()
{
	Super::Reset();

	//PlayerStates persist across seamless travel.  Keep the same teams as previous match.
	//SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void ALukeGamePlayerState::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}

void ALukeGamePlayerState::ClientInitialize(AController* InController)
{
	Super::ClientInitialize(InController);
}

void ALukeGamePlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void ALukeGamePlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void ALukeGamePlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

void ALukeGamePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
}

int32 ALukeGamePlayerState::GetKills() const
{
	return NumKills;
}

int32 ALukeGamePlayerState::GetDeaths() const
{
	return NumDeaths;
}

float ALukeGamePlayerState::GetScore() const
{
	return Score;
}

int32 ALukeGamePlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 ALukeGamePlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

bool ALukeGamePlayerState::IsQuitter() const
{
	return bQuitter;
}

void ALukeGamePlayerState::ScoreKill(ALukeGamePlayerState* Victim, int32 Points)
{
	NumKills++;
}

void ALukeGamePlayerState::ScoreDeath(ALukeGamePlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
}

void ALukeGamePlayerState::InformAboutKill_Implementation(class ALukeGamePlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ALukeGamePlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ALukeGamePlayerController* TestPC = Cast<ALukeGamePlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				FUniqueNetIdRepl LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					//TestPC->OnKill();
				}
			}
		}
	}
}

void ALukeGamePlayerState::BroadcastDeath_Implementation(class ALukeGamePlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ALukeGamePlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		ALukeGamePlayerController* TestPC = Cast<ALukeGamePlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			//TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
		}
	}
}

void ALukeGamePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALukeGamePlayerState, NumKills);
	DOREPLIFETIME(ALukeGamePlayerState, NumDeaths);
}

FString ALukeGamePlayerState::GetShortPlayerName() const
{
	if (GetPlayerName().Len() > 10)
	{
		return GetPlayerName().Left(10) + "...";
	}
	return GetPlayerName();
}
