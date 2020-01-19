// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeCharacterMovementComponent.h"
#include "LukeGameCharacter.h"

ULukeCharacterMovementComponent::ULukeCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	bUseAccelerationForPaths = true;

	bUseControllerDesiredRotation = true;
	RotationRate = FRotator(0.0f, 10000.0f, 0.0f);

	MaxJumpCount = 2;
}

float ULukeCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const ALukeGameCharacter* LukeGameCharacter = Cast<ALukeGameCharacter>(PawnOwner);
	if (LukeGameCharacter)
	{
		if (LukeGameCharacter->IsRunning() && IsMovingForward())
		{
			MaxSpeed *= LukeGameCharacter->GetRunningSpeedModifier();
		}

		if (LukeGameCharacter->IsCrouching())
		{
			MaxSpeed *= LukeGameCharacter->GetCrouchingSpeedModifier();
		}
	}

	return MaxSpeed;
}

bool ULukeCharacterMovementComponent::ServerSetMoveDirection_Validate(const FVector& MoveDir)
{
	return true;
}

void ULukeCharacterMovementComponent::ServerSetMoveDirection_Implementation(const FVector& MoveDir)
{
	MoveDirection = MoveDir;
}

void ULukeCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	//Store movement vector
	if (PawnOwner->IsLocallyControlled())
	{
		MoveDirection = PawnOwner->GetLastMovementInputVector();
		if (GetNetMode() == ENetMode::NM_Client)
		{
			ServerSetMoveDirection(MoveDirection);
		}
	}
}

bool ULukeCharacterMovementComponent::IsMovingForward() const
{
	if (!PawnOwner)
	{
		return false;
	}

	FVector Forward = PawnOwner->GetActorForwardVector();
	FVector MoveDirection = Velocity.GetSafeNormal();

	Forward.Z = 0.0f;
	MoveDirection.Z = 0.0f;

	float VelocityDot = FVector::DotProduct(Forward, MoveDirection);
	return VelocityDot > 0.4f;
}

bool ULukeCharacterMovementComponent::CanJump()
{
	return (IsMovingOnGround() || JumpCount < MaxJumpCount) && CanEverJump();
}

bool ULukeCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (Super::DoJump(bReplayingMoves))
	{
		JumpCount++;

		if (JumpCount > 1)
		{
			//Calculate lateral speed to use in adjusting trajectory in midair
			FVector LateralVelocity = Velocity;
			LateralVelocity.Z = 0.0f;//Don't care about vertical velocity
			float LateralSpeed = LateralVelocity.Size();

			//Average the actual velocity with the target velocity
			FVector NewVelocity = MoveDirection * LateralSpeed;
			NewVelocity.Z = 0.0f;
			NewVelocity += LateralVelocity;
			NewVelocity *= 0.5f;

			Velocity = NewVelocity;
			Velocity.Z = JumpZVelocity;
		}

		return true;
	}

	return false;
}

void ULukeCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	JumpCount = 0;

	ALukeGameCharacter* Player = Cast<ALukeGameCharacter>(GetOwner());
	Player->SetJumpCount(0);

	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

