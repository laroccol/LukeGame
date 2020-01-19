// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LukeCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class LUKEGAME_API ULukeCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	ULukeCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual float GetMaxSpeed() const override;

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerSetMoveDirection(const FVector& MoveDir);

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	FVector MoveDirection;

	bool IsMovingForward() const;

	///@brief Override DoJump to trigger the extra jumps.
	virtual bool DoJump(bool bReplayingMoves) override;
	///@return Whether or not the character can currently jump.
	bool CanJump();
	///@brief This is called whenever the character lands on the ground, and will be used to reset the jump counter.
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	UPROPERTY(Category = "Multijump", EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Max Multijump Count"))
		int32 MaxJumpCount;
	UPROPERTY(Category = "Multijump", BlueprintReadWrite, meta = (DisplayName = "Current jump count"))
		int32 JumpCount;
	
};
