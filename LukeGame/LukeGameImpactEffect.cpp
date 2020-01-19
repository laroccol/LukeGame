// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGameImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ALukeGameImpactEffect::ALukeGameImpactEffect(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAutoDestroyWhenFinished = true;
}

void ALukeGameImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	// show particles
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	// play sound
	USoundBase* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (DefaultDecal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(DefaultDecal.DecalMaterial, FVector(1.0f, DefaultDecal.DecalSize, DefaultDecal.DecalSize),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			DefaultDecal.LifeSpan);
	}

}

UParticleSystem* ALukeGameImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* ImpactFX = NULL;

	switch (SurfaceType)
	{
	case LUKEGAME_SURFACE_Concrete:	ImpactFX = ConcreteFX; break;
	case LUKEGAME_SURFACE_Dirt:		ImpactFX = DirtFX; break;
	case LUKEGAME_SURFACE_Water:		ImpactFX = WaterFX; break;
	case LUKEGAME_SURFACE_Metal:		ImpactFX = MetalFX; break;
	case LUKEGAME_SURFACE_Wood:		ImpactFX = WoodFX; break;
	case LUKEGAME_SURFACE_Grass:		ImpactFX = GrassFX; break;
	case LUKEGAME_SURFACE_Glass:		ImpactFX = GlassFX; break;
	case LUKEGAME_SURFACE_Flesh:		ImpactFX = FleshFX; break;
	default:						ImpactFX = DefaultFX; break;
	}


	return ImpactFX;
}

USoundBase* ALukeGameImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundBase* ImpactSound = NULL;

	switch (SurfaceType)
	{
	case LUKEGAME_SURFACE_Concrete:	ImpactSound = ConcreteSound; break;
	case LUKEGAME_SURFACE_Dirt:		ImpactSound = DirtSound; break;
	case LUKEGAME_SURFACE_Water:		ImpactSound = WaterSound; break;
	case LUKEGAME_SURFACE_Metal:		ImpactSound = MetalSound; break;
	case LUKEGAME_SURFACE_Wood:		ImpactSound = WoodSound; break;
	case LUKEGAME_SURFACE_Grass:		ImpactSound = GrassSound; break;
	case LUKEGAME_SURFACE_Glass:		ImpactSound = GlassSound; break;
	case LUKEGAME_SURFACE_Flesh:		ImpactSound = FleshSound; break;
	default:						ImpactSound = DefaultSound; break;
	}

	return ImpactSound;
}

