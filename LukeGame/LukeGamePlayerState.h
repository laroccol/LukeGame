// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LukeGamePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API ALukeGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ALukeGamePlayerState(const FObjectInitializer& ObjectInitializer);

	// Begin APlayerState interface
	/** clear scores */
	virtual void Reset() override;

	/**
	 * Set the team
	 *
	 * @param	InController	The controller to initialize state with
	 */
	virtual void ClientInitialize(class AController* InController) override;

	virtual void UnregisterPlayerWithSession() override;

	// End APlayerState interface

	/** player killed someone */
	void ScoreKill(ALukeGamePlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(ALukeGamePlayerState* KilledBy, int32 Points);

	/** get number of kills */
	int32 GetKills() const;

	/** get number of deaths */
	int32 GetDeaths() const;

	/** get number of points */
	float GetScore() const;

	/** get number of bullets fired this match */
	int32 GetNumBulletsFired() const;

	/** get number of rockets fired this match */
	int32 GetNumRocketsFired() const;

	/** get whether the player quit the match */
	bool IsQuitter() const;

	/** gets truncated player name to fit in death log and scoreboards */
	FString GetShortPlayerName() const;

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
		void InformAboutKill(class ALukeGamePlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ALukeGamePlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
		void BroadcastDeath(class ALukeGamePlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ALukeGamePlayerState* KilledPlayerState);

	//We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);

	/** Set whether the player is a quitter */
	void SetQuitter(bool bInQuitter);

	virtual void CopyProperties(class APlayerState* PlayerState) override;
protected:

	/** number of kills */
	UPROPERTY(Transient, Replicated)
		int32 NumKills;

	/** number of deaths */
	UPROPERTY(Transient, Replicated)
		int32 NumDeaths;

	/** number of bullets fired this match */
	UPROPERTY()
		int32 NumBulletsFired;

	/** number of rockets fired this match */
	UPROPERTY()
		int32 NumRocketsFired;

	/** whether the user quit the match */
	UPROPERTY()
		uint8 bQuitter : 1;
};
