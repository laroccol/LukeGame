// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LukeGamePickup.h"
#include "WeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API AWeaponPickup : public ALukeGamePickup
{
	GENERATED_BODY()

public:

	void PickedUpBy(APawn* Pawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, noclear, Category = "Class Types")
		TSubclassOf<class ALukeGameWeapon> WeaponActual;
	
};
