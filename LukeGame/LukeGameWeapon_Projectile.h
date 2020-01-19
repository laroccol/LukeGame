// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LukeGameWeapon.h"
#include "GameFramework/DamageType.h"
#include "LukeGameWeapon_Projectile.generated.h"

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class ALukeGameProjectile> ProjectileClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float ProjectileLife;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

/**
 * 
 */
UCLASS(Abstract)
class LUKEGAME_API ALukeGameWeapon_Projectile : public ALukeGameWeapon
{
	GENERATED_BODY()

public:

	ALukeGameWeapon_Projectile(const FObjectInitializer& ObjectInitializer);

	/** apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::ERocket;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
		FProjectileWeaponData ProjectileConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
		void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
	
};
