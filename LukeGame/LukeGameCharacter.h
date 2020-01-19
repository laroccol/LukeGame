// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LukeGameTypes.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "LukeGameCharacter.generated.h"

class USoundCue;
class ULukeGameAttributeSet;
class UGameplayAbilityBase;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLukeGameCharacterEquipWeapon, ALukeGameCharacter*, ALukeGameWeapon* /* new */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLukeGameCharacterUnEquipWeapon, ALukeGameCharacter*, ALukeGameWeapon* /* old */);

UENUM(BlueprintType)
enum class AbilityInput : uint8
{
	UseAbility1 UMETA(DisplayName = "Use Spell 1"), //This maps the first ability(input ID should be 0 in int) to the action mapping(which you define in the project settings) by the name of "UseAbility1". "Use Spell 1" is the blueprint name of the element.
	UseAbility2 UMETA(DisplayName = "Use Spell 2"), //Maps ability 2(input ID 1) to action mapping UseAbility2. "Use Spell 2" is mostly used for when the enum is a blueprint variable.
	UseAbility3 UMETA(DisplayName = "Use Spell 3"),
	UseAbility4 UMETA(DisplayName = "Use Spell 4"),
	WeaponAbility UMETA(DisplayName = "Use Weapon"), //This finally maps the fifth ability(here designated to be your weaponability, or auto-attack, or whatever) to action mapping "WeaponAbility".

		//You may also do something like define an enum element name that is not actually mapped to an input, for example if you have a passive ability that isn't supposed to have an input. This isn't usually necessary though as you usually grant abilities via input ID,
		//which can be negative while enums cannot. In fact, a constant called "INDEX_NONE" exists for the exact purpose of rendering an input as unavailable, and it's simply defined as -1.
		//Because abilities are granted by input ID, which is an int, you may use enum elements to describe the ID anyway however, because enums are fancily dressed up ints.
};

UCLASS(config=Game)
class ALukeGameCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Battery, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* CollectionSphere;

	class UTimelineComponent* FOVAiming;
public:
	ALukeGameCharacter(const FObjectInitializer& ObjectInitializer);

	UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystem;
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Abilities)
		TSubclassOf<class UGameplayAbility> Ability;

	virtual void PostInitializeComponents() override;

	virtual void BeginDestroy() override;

	virtual void Destroyed() override;

	virtual void PawnClientRestart() override;

	virtual void PossessedBy(class AController* InController) override;

	virtual void OnRep_PlayerState() override;

	virtual bool IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer) override;

	virtual void OnReplicationPausedChanged(bool bIsReplicationPaused) override;

	virtual void BeginPlay() override;

	virtual void Restart() override;

	FORCEINLINE class USphereComponent* GetCollectionSphere() const { return CollectionSphere; }

	class ULukeGameInstance* CurrentGameInstance;

	//Abilities
	//-----------------------------------------------------------------------------------------------

	/** Our ability system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
		class UAbilitySystemComponent* AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
		ULukeGameAttributeSet* AttributeSetComp;

	UFUNCTION(BlueprintCallable, Category = "Ability")
		void AquireAbility(TSubclassOf<UGameplayAbility> AbilityToAquire);

	UFUNCTION(BlueprintCallable, Category = "Ability")
		void AquireAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToAquire);

	UFUNCTION()
		void OnHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (DisplayName = "OnHealthChanged"))
		void BP_OnHealthChanged(float CurrentHealth, float MaxHealth);


	//Aiming
	//-----------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "TimeLine")
		class UCurveFloat* FOVCurve;

	FOnTimelineFloat InterpFunction{};

	FOnTimelineEvent TimelineFinished{};

	UFUNCTION()
		void TimelineFloatReturn(float Value);

	UFUNCTION()
		void OnTimelineFinished();

	FTimerHandle PitchTimer;

	void UndoRecoil();

	int ConcurrentShots;


	//void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	//Health
	//---------------------------------------------------------------------------------

	bool IsAlive() const;

	int32 GetMaxHealth() const;

	float GetLowHealthPercentage() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Health", meta = (DisplayName = "UpdateUI"))
		void BP_UpdateUI(float CurrentHealth);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health)
		uint32 bIsDying : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Health)
		float Health;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetHealth() const;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	//virtual void Suicide();

	//virtual void KilledBy(class APawn* EventInstigator);

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	void SetRagdollPhysics();

	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	virtual void TornOff();

	//virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	//Weapon
	//-------------------------------------------------------------------------------

	FName GetWeaponAttachPoint() const;
	 
	bool CanFire() const;

	LUKEGAME_API static FOnLukeGameCharacterEquipWeapon NotifyEquipWeapon;

	LUKEGAME_API static FOnLukeGameCharacterUnEquipWeapon NotifyUnEquipWeapon;

	bool CanReload() const;

	/** get currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		class ALukeGameWeapon* GetWeapon() const;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
		class ALukeGameWeapon* CurrentWeapon;

	/** current weapon rep handler */
	UFUNCTION()
		void OnRep_CurrentWeapon(class ALukeGameWeapon* LastWeapon);

	/** updates current weapon */
	void SetCurrentWeapon(class ALukeGameWeapon* NewWeapon, class ALukeGameWeapon* LastWeapon = NULL);

	void SpawnDefaultInventory();

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		TArray<TSubclassOf<class ALukeGameWeapon> > DefaultInventoryClasses;

	UPROPERTY(Transient, Replicated)
		TArray<class ALukeGameWeapon*> Inventory;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
		struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	void AddWeapon(class ALukeGameWeapon* Weapon);

	void RemoveWeapon(class ALukeGameWeapon* Weapon);

	UFUNCTION(reliable, server, WithValidation)
		void DropCurrentWeapon();

	class ALukeGameWeapon* FindWeapon(TSubclassOf<class ALukeGameWeapon> WeaponClass);

	void EquipWeapon(class ALukeGameWeapon* Weapon);

	void StartWeaponFire();

	void StopWeaponFire();

	void OnNextWeapon();

	void OnPrevWeapon();

	void OnReload();

	USkeletalMeshComponent* GetPawnMesh() const;

	int32 GetInventoryCount() const;

	class ALukeGameWeapon* GetInventoryWeapon(int32 index) const;

	bool IsEnemyFor(AController* TestPC) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, noclear, Category = "ClassTypes")
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, noclear, Category = "Class Types")
		UParticleSystem* ImpactEffect;

	void SetTargeting(bool bNewTargeting);

	void OnStartTargeting();

	void OnStopTargeting();

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		float GetTargetingSpeedModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		bool IsTargeting() const;


	//virtual void Tick(float DeltaSeconds) override;

	//Movement
	//-------------------------------------------------------------------------------

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	void SetCrouching(bool bNewCrouching);

	/** player pressed run action */
	void OnStartRunning();

	/** player pressed toggled run action */
	void OnStartRunningToggle();

	void OnStartCrouching();

	/** player released run action */
	void OnStopRunning();

	void OnStopCrouching();

	void DamageCharacter();

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetRunningSpeedModifier() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetCrouchingSpeedModifier() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsCrouching() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetMoveForwardValue() const;

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerMoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetMoveRightValue() const;

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerMoveRight(float Value);

	bool CanJumpInternal_Implementation() const override;

	void Jump() override;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		int32 GetJumpCount() const;

	void SetJumpCount(int32 NewJumpCount);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerSetJumpCount(int32 NewJumpCount);

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		FRotator GetAimOffsets() const;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		float RunningSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		float CrouchingSpeedModifier;

	/** current running state */
	UPROPERTY(Transient, Replicated)
		uint8 bWantsToRun : 1;

	/** from gamepad running is toggled */
	uint8 bWantsToRunToggled : 1;

	UPROPERTY(Transient, Replicated)
		uint8 bWantsToCrouch : 1;


	/** update targeting state */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerSetRunning(bool bNewRunning, bool bToggle);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerSetCrouching(bool bNewCrouching);

	//Weapon
	//------------------------------------------------------------
	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		FName WeaponAttachPoint;

	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipWeapon(class ALukeGameWeapon* NewWeapon);

	uint8 bWantsToFire : 1;

	void DestroyInventory();

	UFUNCTION()
		void OnRep_LastTakeHitInfo();

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		float TargetingSpeedModifier;

	UPROPERTY(Transient, Replicated)
		uint8 bIsTargeting : 1;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		USoundCue* TargetingSound;

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerSetTargeting(bool bNewTargeting);

	//Pickups
	//---------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Spawning")
		void CollectPickups();

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerCollectPickups();

	//Sounds and Effects
	//----------------------------------------------------------------

	UPROPERTY(Transient)
		TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** animation played on death */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		UAnimationAsset* DeathAnim;

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		USoundCue* DeathSound;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		class UParticleSystem* RespawnFX;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		class USoundCue* RespawnSound;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		USoundCue* LowHealthSound;

	/** sound played when running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		USoundCue* RunLoopSound;

	/** sound played when stop running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		USoundCue* RunStopSound;

	/** used to manipulate with run loop sound */
	UPROPERTY()
		UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
		UAudioComponent* LowHealthWarningPlayer;


	//Health
	//------------------------------------------------------------
	
	float LowHealthPercentage;

	//Vehicle
	//------------------------------------------------------------

	void EnterVehicle();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEnterVehicle();

	void AddAbilityToUI(TSubclassOf<UGameplayAbilityBase> AbilityToAdd);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	UPROPERTY(Replicated)
		float MoveForwardValue;

	UPROPERTY(Replicated)
		float MoveRightValue;

	UPROPERTY(Replicated)
		int32 JumpCount;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
		float CollectionSphereRadius;
};

