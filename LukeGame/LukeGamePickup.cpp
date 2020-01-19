// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGamePickup.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

ALukeGamePickup::ALukeGamePickup() {
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = false;

	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);

	if (Role == ROLE_Authority) {
		bIsActive = true;
	}
}

void ALukeGamePickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALukeGamePickup, bIsActive);
}

void ALukeGamePickup::OnRep_IsActive() {

}

bool ALukeGamePickup::IsActive() {
	return bIsActive;
}

void ALukeGamePickup::SetActive(bool NewPickupState) {
	if (Role == ROLE_Authority) {
		bIsActive = NewPickupState;
	}
}

void ALukeGamePickup::WasCollected_Implementation() {
	//UE_LOG(LogClass, Log, TEXT("ALukeGamePickup::WasCollected_Implementation %s"), GetName());
}

void ALukeGamePickup::PickedUpBy(APawn* Pawn) {
	if (Role == ROLE_Authority) {
		PickupInstigator = Pawn;
		ClientOnPickedUpBy(Pawn);
	}
}

void ALukeGamePickup::ClientOnPickedUpBy_Implementation(APawn* Pawn) {
	PickupInstigator = Pawn;
	WasCollected();
}

