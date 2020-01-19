// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LukeGameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "LukeCharacterMovementComponent.h"
#include "Engine.h"
#include "LukeGameWeapon.h"
#include "LukeGamePlayerController.h"
#include "LukeGameGameMode.h"
#include "LukeGameTypes.h"
#include "WeaponPickup.h"
#include "LukeGameHUD.h"
#include "LukeGameDamageType.h"
#include "LukeGamePlayerState.h"
#include "LukeCar.h"
#include "LukeGameInstance.h"
#include "LukeGameAttributeSet.h"
#include "GameplayAbilityBase.h"
#include "LukeAbilityTypes.h"

//////////////////////////////////////////////////////////////////////////
// ALukeGameCharacter

FOnLukeGameCharacterEquipWeapon ALukeGameCharacter::NotifyEquipWeapon;
FOnLukeGameCharacterUnEquipWeapon ALukeGameCharacter::NotifyUnEquipWeapon;

ALukeGameCharacter::ALukeGameCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULukeCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 90.f;
	BaseLookUpRate = 90.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CollectionSphereRadius = 200.0f;

	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetupAttachment(RootComponent);
	CollectionSphere->SetSphereRadius(CollectionSphereRadius);

	FOVAiming = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	RunningSpeedModifier = 2.0f;
	bWantsToRun = false;

	CrouchingSpeedModifier = 0.5f;
	bWantsToCrouch = false;

	JumpCount = 0;

	LowHealthPercentage = 0.3f;

	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AttributeSetComp = CreateDefaultSubobject<ULukeGameAttributeSet>(TEXT("AttributeSetComp"));

	ConcurrentShots = 0;
}

void ALukeGameCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		Health = GetMaxHealth();
		SpawnDefaultInventory();
	}
}

void ALukeGameCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALukeGameCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void ALukeGameCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	SetCurrentWeapon(CurrentWeapon);
}

void ALukeGameCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	AbilitySystem->RefreshAbilityActorInfo();
}

void ALukeGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

bool ALukeGameCharacter::IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer)
{
	Super::IsReplicationPausedForConnection(ConnectionOwnerNetViewer);

	return false;
}

void ALukeGameCharacter::OnReplicationPausedChanged(bool bIsReplicationPaused)
{
	GetMesh()->SetHiddenInGame(bIsReplicationPaused, true);
}

void ALukeGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentGameInstance = Cast<ULukeGameInstance>(GetGameInstance());

	AttributeSetComp->OnHealthChange.AddDynamic(this, &ALukeGameCharacter::OnHealthChanged);

	if (FOVCurve)
	{
		FOVAiming->AddInterpFloat(FOVCurve, InterpFunction, FName("Alpha"));
		FOVAiming->SetTimelineFinishedFunc(TimelineFinished);

		FOVAiming->SetLooping(false);
	}
}

void ALukeGameCharacter::Restart()
{
	AbilitySystem->RefreshAbilityActorInfo();
}

void ALukeGameCharacter::AquireAbility(TSubclassOf<UGameplayAbility> AbilityToAquire)
{
	if (AbilitySystem)
	{
		if (HasAuthority() && AbilityToAquire)
		{
			FGameplayAbilitySpecDef SpecDef = FGameplayAbilitySpecDef();
			SpecDef.Ability = AbilityToAquire;
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(SpecDef, 1);
			AbilitySystem->GiveAbility(AbilitySpec);
		}
		AbilitySystem->InitAbilityActorInfo(this, this);
	}
}

void ALukeGameCharacter::AquireAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToAquire)
{
	for (TSubclassOf<UGameplayAbility> AbilityItem : AbilitiesToAquire)
	{	
		AquireAbility(AbilityItem);
		TSubclassOf<UGameplayAbilityBase> AbilityBaseClass = *AbilityItem;
		if (AbilityBaseClass != nullptr)
		{
			AddAbilityToUI(AbilityBaseClass);
		}
	}
}

void ALukeGameCharacter::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (CurrentHealth <= 0.f)
	{
		Die(0.f, FDamageEvent(), nullptr, nullptr);
	}
	BP_OnHealthChanged(CurrentHealth, MaxHealth);
}

void ALukeGameCharacter::TimelineFloatReturn(float Value)
{
	FollowCamera->SetFieldOfView(FMath::Lerp(90.0f, 70.0f, Value));
}

void ALukeGameCharacter::OnTimelineFinished()
{
}

void ALukeGameCharacter::UndoRecoil()
{
	AddControllerPitchInput(0.1f);
	ConcurrentShots--;
	if (ConcurrentShots <= 0)
	{
		GetWorldTimerManager().ClearTimer(PitchTimer);
	}
}

bool ALukeGameCharacter::IsAlive() const
{
	return Health > 0;
}

int32 ALukeGameCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<ALukeGameCharacter>()->Health;
}

float ALukeGameCharacter::GetLowHealthPercentage() const
{
	return LowHealthPercentage;
}

float ALukeGameCharacter::GetHealth() const
{
	return Health;
}

float ALukeGameCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		BP_UpdateUI(Health);
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}

bool ALukeGameCharacter::CanDie(float KillingDamage, struct FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode<ALukeGameGameMode>() == NULL)
//		|| GetWorld()->GetAuthGameMode<ALukeGameGameMode>()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}

bool ALukeGameCharacter::Die(float KillingDamage, struct FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<ALukeGameGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<ALukeGameCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

void ALukeGameCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	TearOff();
	bIsDying = true;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		ALukeGamePlayerController* PC = Cast<ALukeGamePlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			ULukeGameDamageType* DamageType = Cast<ULukeGameDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback && PC->IsVibrationEnabled())
			{
				FForceFeedbackParameters FFParams;
				FFParams.Tag = "Damage";
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, FFParams);
			}
		}
	}

	// cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	if (GetNetMode() != NM_DedicatedServer && DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	// remove all weapons
	DestroyInventory();

	//StopAllAnimMontages();

	if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
		LowHealthWarningPlayer->Stop();
	}

	if (RunLoopAC)
	{
		RunLoopAC->Stop();
	}

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	float DeathAnimDuration = 3.933f; 
	GetMesh()->PlayAnimation(DeathAnim, false);

	// Ragdoll
	if (DeathAnimDuration > 0.f)
	{
		// Trigger ragdoll a little before the animation early so the character doesn't
		// blend back to its normal position.
		const float TriggerRagdollTime = DeathAnimDuration - 0.7f;

		// Enable blend physics so the bones are properly blending against the montage.
		GetMesh()->bBlendPhysics = true;

		// Use a local timer handle as we don't need to store it for later but we don't need to look for something to clear
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ALukeGameCharacter::SetRagdollPhysics, FMath::Max(0.1f, TriggerRagdollTime), false);
	}
	else
	{
		SetRagdollPhysics();
	}

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ALukeGameCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);

		// play the force feedback effect on the client player controller
		ALukeGamePlayerController* PC = Cast<ALukeGamePlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			ULukeGameDamageType* DamageType = Cast<ULukeGameDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback && PC->IsVibrationEnabled())
			{
				FForceFeedbackParameters FFParams;
				FFParams.Tag = "Damage";
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, FFParams);
			}
		}
	}

	if (DamageTaken > 0.f)
	{
		//ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}

	ALukeGamePlayerController* MyPC = Cast<ALukeGamePlayerController>(Controller);
	ALukeGameHUD* MyHUD = MyPC ? Cast<ALukeGameHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyWeaponHit(DamageTaken, DamageEvent, PawnInstigator);
	}

	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		ALukeGamePlayerController* InstigatorPC = Cast<ALukeGamePlayerController>(PawnInstigator->Controller);
		ALukeGameHUD* InstigatorHUD = InstigatorPC ? Cast<ALukeGameHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}

}


void ALukeGameCharacter::SetRagdollPhysics()
{
	DetachFromControllerPendingDestroy();
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}



void ALukeGameCharacter::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<ALukeGameCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	LastTakeHitTimeTimeout = TimeoutTime;
}

void ALukeGameCharacter::TornOff()
{
	SetLifeSpan(25.f);
}

FName ALukeGameCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

bool ALukeGameCharacter::CanFire() const
{
	return IsAlive();
}

bool ALukeGameCharacter::CanReload() const
{
	return true;
}

ALukeGameWeapon* ALukeGameCharacter::GetWeapon() const
{
	return CurrentWeapon;
}

void ALukeGameCharacter::OnRep_CurrentWeapon(ALukeGameWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void ALukeGameCharacter::SetCurrentWeapon(ALukeGameWeapon* NewWeapon, ALukeGameWeapon* LastWeapon)
{
	ALukeGameWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	//unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	//equip new one
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);

		NewWeapon->OnEquip(LastWeapon);
	}
}

void ALukeGameCharacter::SpawnDefaultInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ALukeGameWeapon* NewWeapon = GetWorld()->SpawnActor<ALukeGameWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	// equip first weapon in inventory
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}

void ALukeGameCharacter::AddWeapon(ALukeGameWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
	}

	if (Inventory.Num() == 1)
	{
		EquipWeapon(Weapon);
	}
}

void ALukeGameCharacter::RemoveWeapon(ALukeGameWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);

		if (Inventory.Num() == 0)
		{
			CurrentWeapon = NULL;
		}
	}
}

bool ALukeGameCharacter::DropCurrentWeapon_Validate()
{
	return true;
}

void ALukeGameCharacter::DropCurrentWeapon_Implementation()
{
	if (CurrentWeapon != NULL)
	{
		FActorSpawnParameters SpawnParams;
		FVector SpawnLocation = GetMesh()->GetBoneLocation("LeftHand");
		FRotator SpawnRotation = FRotator(0, 0, 0);
		AWeaponPickup* WeaponToDrop = GetWorld()->SpawnActor<AWeaponPickup>(CurrentWeapon->WeaponPickup, SpawnLocation, SpawnRotation, SpawnParams);

		RemoveWeapon(CurrentWeapon);
		

	}
}

ALukeGameWeapon* ALukeGameCharacter::FindWeapon(TSubclassOf<ALukeGameWeapon> WeaponClass)
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}

	return NULL;
}

void ALukeGameCharacter::EquipWeapon(ALukeGameWeapon* Weapon)
{
	if (Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

void ALukeGameCharacter::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			if (IsRunning())
			{
				SetRunning(false, false);

			}
			CurrentWeapon->StartFire();
		}
	}
}

void ALukeGameCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}

void ALukeGameCharacter::OnNextWeapon()
{
	ALukeGamePlayerController* MyPC = Cast<ALukeGamePlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			ALukeGameWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon);
		}
	}
}

void ALukeGameCharacter::OnPrevWeapon()
{
	ALukeGamePlayerController* MyPC = Cast<ALukeGamePlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			ALukeGameWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
			EquipWeapon(PrevWeapon);
		}
	}
}

void ALukeGameCharacter::OnReload()
{
	ALukeGamePlayerController* MyPC = Cast<ALukeGamePlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StartReload();
		}
	}
}

USkeletalMeshComponent* ALukeGameCharacter::GetPawnMesh() const
{
	return GetMesh();
}

int32 ALukeGameCharacter::GetInventoryCount() const
{
	return Inventory.Num();
}

ALukeGameWeapon* ALukeGameCharacter::GetInventoryWeapon(int32 index) const
{
	return Inventory[index];
}

bool ALukeGameCharacter::IsEnemyFor(AController* TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}

	ALukeGamePlayerState* TestPlayerState = Cast<ALukeGamePlayerState>(TestPC->PlayerState);
	ALukeGamePlayerState* MyPlayerState = Cast<ALukeGamePlayerState>(GetPlayerState());

	bool bIsEnemy = true;
	if (GetWorld()->GetGameState())
	{
		const ALukeGameGameMode* DefGame = GetWorld()->GetGameState()->GetDefaultGameMode<ALukeGameGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->CanDealDamage(TestPlayerState, MyPlayerState);
		}
	}

	return bIsEnemy;
}

void ALukeGameCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	if (TargetingSound)
	{
		UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
	}

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(bNewTargeting);
	}
}

void ALukeGameCharacter::OnStartTargeting()
{
	ULukeCharacterMovementComponent* MoveComp = Cast<ULukeCharacterMovementComponent>(GetCharacterMovement());
	ALukeGamePlayerController* MyPC = Cast<ALukeGamePlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);

		}
		SetTargeting(true);

		FOVAiming->Play();
	}
}

void ALukeGameCharacter::OnStopTargeting()
{
	SetTargeting(false);
	FOVAiming->Reverse();
}

float ALukeGameCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

bool ALukeGameCharacter::IsTargeting() const
{
	return bIsTargeting;
}

bool ALukeGameCharacter::ServerSetTargeting_Validate(bool bNewTargeting)
{
	return true;
}

void ALukeGameCharacter::ServerSetTargeting_Implementation(bool bNewTargeting)
{
	SetTargeting(bNewTargeting);
}

bool ALukeGameCharacter::ServerEquipWeapon_Validate(ALukeGameWeapon* Weapon)
{
	return true;
}

void ALukeGameCharacter::ServerEquipWeapon_Implementation(ALukeGameWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

void ALukeGameCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	if (IsCrouching())
	{
		SetCrouching(false);
	}

	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning, bToggle);
	}
}

void ALukeGameCharacter::SetCrouching(bool bNewCrouching)
{
	if (IsRunning() && bNewCrouching)
	{
		SetRunning(false, false);
	}

	bWantsToCrouch = bNewCrouching;
	
	if (Role < ROLE_Authority)
	{
		ServerSetCrouching(bNewCrouching);
	}
}

void ALukeGameCharacter::OnStartRunning()
{
	SetRunning(true, false);
}

void ALukeGameCharacter::OnStartCrouching()
{
	if (IsCrouching())
	{
		SetCrouching(false);
	}
	else {
		SetCrouching(true);
	}
}

void ALukeGameCharacter::OnStartRunningToggle()
{
	SetRunning(true, true);
}

void ALukeGameCharacter::OnStopRunning()
{
	SetRunning(false, false);
}

void ALukeGameCharacter::DamageCharacter()
{
	FPointDamageEvent PointDmg;
	PointDmg.ShotDirection = FVector(0, 0, 0);
	PointDmg.Damage = 15.0f;


	TakeDamage(PointDmg.Damage, PointDmg, nullptr, nullptr);
}

bool ALukeGameCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return (bWantsToRun || bWantsToRunToggled);
}

bool ALukeGameCharacter::IsCrouching() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return bWantsToCrouch;
}

float ALukeGameCharacter::GetMoveForwardValue() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return MoveForwardValue;
}

float ALukeGameCharacter::GetMoveRightValue() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return MoveRightValue;
}

bool ALukeGameCharacter::CanJumpInternal_Implementation() const
{
	bool bCanJump = Super::CanJumpInternal_Implementation();

	ULukeCharacterMovementComponent* MoveComp = Cast<ULukeCharacterMovementComponent>(GetCharacterMovement());
	if (!bCanJump && MoveComp)
	{
		bCanJump = MoveComp->CanJump();
	}

	return bCanJump;
}

void ALukeGameCharacter::Jump()
{
	Super::Jump();

	ULukeCharacterMovementComponent* MoveComp = Cast<ULukeCharacterMovementComponent>(GetCharacterMovement());

	SetCrouching(false);
	//SetRunning(false, false);

	JumpCount = MoveComp->JumpCount;
}

int32 ALukeGameCharacter::GetJumpCount() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return JumpCount;
}

void ALukeGameCharacter::SetJumpCount(int32 NewJumpCount)
{
	JumpCount = NewJumpCount;

	if (Role < ROLE_Authority)
	{
		ServerSetJumpCount(NewJumpCount);
	}
}

bool ALukeGameCharacter::ServerSetJumpCount_Validate(int32 NewJumpCount)
{
	return true;
}

void ALukeGameCharacter::ServerSetJumpCount_Implementation(int32 NewJumpCount)
{
	SetJumpCount(NewJumpCount);
}

FRotator ALukeGameCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool ALukeGameCharacter::ServerMoveForward_Validate(float Value)
{
	return true;
}

void ALukeGameCharacter::ServerMoveForward_Implementation(float Value)
{
	MoveForwardValue = Value;
}

bool ALukeGameCharacter::ServerMoveRight_Validate(float Value)
{
	return true;
}

void ALukeGameCharacter::ServerMoveRight_Implementation(float Value)
{
	MoveRightValue = Value;
}

void ALukeGameCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALukeGameCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ALukeGameCharacter, bWantsToCrouch, COND_SkipOwner);
	DOREPLIFETIME(ALukeGameCharacter, MoveForwardValue);
	DOREPLIFETIME(ALukeGameCharacter, MoveRightValue);
	DOREPLIFETIME(ALukeGameCharacter, JumpCount);
	DOREPLIFETIME(ALukeGameCharacter, Health);
	DOREPLIFETIME(ALukeGameCharacter, CurrentWeapon);
	DOREPLIFETIME(ALukeGameCharacter, CollectionSphereRadius);
	DOREPLIFETIME_CONDITION(ALukeGameCharacter, Inventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ALukeGameCharacter, LastTakeHitInfo, COND_Custom);

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(ALukeGameCharacter, bIsTargeting, COND_SkipOwner);
}

float ALukeGameCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}

float ALukeGameCharacter::GetCrouchingSpeedModifier() const
{
	return CrouchingSpeedModifier;
}

bool ALukeGameCharacter::ServerSetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void ALukeGameCharacter::ServerSetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	SetRunning(bNewRunning, bToggle);
}

bool ALukeGameCharacter::ServerSetCrouching_Validate(bool bNewCrouching)
{
	return true;
}

void ALukeGameCharacter::ServerSetCrouching_Implementation(bool bNewCrouching)
{
	SetCrouching(bNewCrouching);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALukeGameCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		ALukeGameWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

void ALukeGameCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
}

void ALukeGameCharacter::CollectPickups()
{
	ServerCollectPickups();
}

bool ALukeGameCharacter::ServerCollectPickups_Validate()
{
	return true;
}

void ALukeGameCharacter::ServerCollectPickups_Implementation()
{
	if (Role == ROLE_Authority) {

		TArray<AActor*> CollectActors;
		CollectionSphere->GetOverlappingActors(CollectActors);

		for (int i = 0; i < CollectActors.Num(); ++i) {
			ALukeGamePickup* const TestPickup = Cast<ALukeGamePickup>(CollectActors[i]);

			if (TestPickup != NULL && !TestPickup->IsPendingKill() && TestPickup->IsActive()) {
				
				if (AWeaponPickup * WeaponPickup = Cast<AWeaponPickup>(TestPickup))
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Weapon Is Good");
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					ALukeGameWeapon* NewWeapon = GetWorld()->SpawnActor<ALukeGameWeapon>(WeaponPickup->WeaponActual, SpawnInfo);
					AddWeapon(NewWeapon);
				}
				else {
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Weapon Is Not Good");
				}
				TestPickup->PickedUpBy(this);
				TestPickup->SetActive(false);
			}
		}
	}
}

void ALukeGameCharacter::EnterVehicle()
{
	if (GetNetMode() == NM_Client)
	{
		ServerEnterVehicle();
		return;
	}

	for (TActorIterator<ALukeCar> Vehicle(GetWorld()); Vehicle; ++Vehicle)
	{
		if (Vehicle->GetDistanceTo(this) < 500.0f)
		{
			Vehicle->EnterCharacter(this);
			break;
		}
	}
}

bool ALukeGameCharacter::ServerEnterVehicle_Validate()
{
	return true;
}

void ALukeGameCharacter::ServerEnterVehicle_Implementation()
{
	EnterVehicle();
}

void ALukeGameCharacter::AddAbilityToUI(TSubclassOf<UGameplayAbilityBase> AbilityToAdd)
{
	ALukeGamePlayerController* PlayerControllerBase = Cast<ALukeGamePlayerController>(GetController());
	if (PlayerControllerBase)
	{
		UGameplayAbilityBase* AbilityInstance = AbilityToAdd.Get()->GetDefaultObject<UGameplayAbilityBase>();
		if (AbilityInstance)
		{
			FGameplayAbilityInfo AbilityInfo = AbilityInstance->GetAbilityInfo();
			PlayerControllerBase->AddAbilityToUI(AbilityInfo);
		}
	}
}

void ALukeGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ALukeGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALukeGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ALukeGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &ALukeGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ALukeGameCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ALukeGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ALukeGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ALukeGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ALukeGameCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ALukeGameCharacter::OnStartRunning);
	PlayerInputComponent->BindAction("RunToggle", IE_Pressed, this, &ALukeGameCharacter::OnStartRunningToggle);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ALukeGameCharacter::OnStopRunning);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ALukeGameCharacter::OnStartCrouching);

	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &ALukeGameCharacter::StartWeaponFire);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Released, this, &ALukeGameCharacter::StopWeaponFire);

	PlayerInputComponent->BindAction("TakeDamage", IE_Pressed, this, &ALukeGameCharacter::DamageCharacter);

	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &ALukeGameCharacter::DropCurrentWeapon);

	PlayerInputComponent->BindAction("CollectPickups", IE_Pressed, this, &ALukeGameCharacter::CollectPickups);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ALukeGameCharacter::OnNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &ALukeGameCharacter::OnPrevWeapon);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &ALukeGameCharacter::OnStartTargeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &ALukeGameCharacter::OnStopTargeting);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ALukeGameCharacter::OnReload);

	PlayerInputComponent->BindAction("EnterVehicle", IE_Pressed, this, &ALukeGameCharacter::EnterVehicle);

	AbilitySystem->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "AbilityInput"));

}


void ALukeGameCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ALukeGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ALukeGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ALukeGameCharacter::TurnAtRate(float Rate)
{
	float Sensitivity = CurrentGameInstance->Sensitivity;
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * Sensitivity * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ALukeGameCharacter::LookUpAtRate(float Rate)
{

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * CurrentGameInstance->Sensitivity * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ALukeGameCharacter::MoveForward(float Value)
{
	MoveForwardValue = Value;

	if (Role < ROLE_Authority)
	{
		ServerMoveForward(Value);
	}

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ALukeGameCharacter::MoveRight(float Value)
{
	MoveRightValue = Value;

	if (Role < ROLE_Authority)
	{
		ServerMoveRight(Value);
	}

	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		ULukeCharacterMovementComponent* MoveComp = Cast<ULukeCharacterMovementComponent>(GetCharacterMovement());

		if (IsRunning() && Value > 0.0f && MoveComp->IsMovingForward())
		{
			YawRotation.Yaw -= 15.0f;
		}
		else if (IsRunning() && Value < 0.0f && MoveComp->IsMovingForward())
		{
			YawRotation.Yaw += 15.0f;
		}
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
