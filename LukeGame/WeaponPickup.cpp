// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"

void AWeaponPickup::PickedUpBy(APawn* Pawn)
{
	Super::PickedUpBy(Pawn);

	if (Role == ROLE_Authority)
	{
		SetLifeSpan(0.001f);
	}
}
