// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "LukeGamePickup.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API ALukeGamePickup : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ALukeGamePickup();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Pickup")
		bool IsActive();

	UFUNCTION(BlueprintCallable, Category = "Pickup")
		void SetActive(bool NewPickupState);

	UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
		void WasCollected();
	virtual void WasCollected_Implementation();

	UFUNCTION(BlueprintAuthorityOnly, Category = "Pickup")
		virtual void PickedUpBy(APawn* Pawn);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
		bool bIsActive;

	UFUNCTION()
		virtual void OnRep_IsActive();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
		APawn* PickupInstigator;
private:
	UFUNCTION(NetMulticast, Unreliable)
		void ClientOnPickedUpBy(APawn* Pawn);
	
};
