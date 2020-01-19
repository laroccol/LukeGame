// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine.h"
#include "LukeHook.generated.h"

class UCableComponent;

UCLASS()
class LUKEGAME_API ALukeHook : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALukeHook(const FObjectInitializer& ObjectInitializer);

	void ThrowHook();

	UPROPERTY(EditAnywhere, Category = "TimeLine")
		class UCurveFloat* HookCurve;

	FOnTimelineFloat InterpFunction{};

	FOnTimelineEvent TimelineFinished{};

	FOnTimelineFloat ActorInterpFunction{};

	FOnTimelineEvent ActorTimelineFinished{};

	UFUNCTION()
		void TimelineFloatReturn(float Value);

	UFUNCTION()
		void OnTimelineFinished();

	UFUNCTION()
		void ActorTimelineFloatReturn(float Value);

	UFUNCTION()
		void OnActorTimelineFinished();

	UFUNCTION()
		void OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	class ALukeGameCharacter* HitCharacter;

	void AttachCableToOwner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle HookTimer;

	FTimerHandle AttachTimer;

private:
	
	FVector Location;

public:

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Movement)
		UProjectileMovementComponent* ProjectileMovementComponent;


	UPROPERTY(VisibleAnywhere, Category = Projectile)
		UStaticMeshComponent* Mesh;

	class UTimelineComponent* HookTimeline;

	class UTimelineComponent* ActorTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = cable, meta = (AllowPrivateAccess = "true"))
		UCableComponent* Cable;


};
