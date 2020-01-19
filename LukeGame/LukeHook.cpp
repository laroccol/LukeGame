// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeHook.h"
#include "LukeGameCharacter.h"
#include "CableComponent.h"

// Sets default values
ALukeHook::ALukeHook(const FObjectInitializer& ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);

	// Use a sphere as a simple collision representation.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Set the sphere's collision radius.
	CollisionComponent->InitSphereRadius(32.0f);

	CollisionComponent->SetCollisionProfileName(FName("BlockAllDynamic"));

	CollisionComponent->OnComponentHit.AddDynamic(this, &ALukeHook::OnSphereHit);
	// Set the root component to be the collision component.
	RootComponent = CollisionComponent;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	Mesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObject(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (MeshObject.Succeeded())
	{
		Mesh->SetStaticMesh(MeshObject.Object);
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -25.0f));
		Mesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
		Mesh->SetCollisionProfileName(FName("BlockAllDynamic"));
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->InitialSpeed = 3500.0f;
	ProjectileMovementComponent->MaxSpeed = 3500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> HookCurveObject(TEXT("CurveFloat'/Game/Blueprints/HookCurve.HookCurve'"));
	if (HookCurveObject.Succeeded())
	{
		HookCurve = HookCurveObject.Object;
	}

	HookTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	ActorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ActorTimeline"));

	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));

	ActorInterpFunction.BindUFunction(this, FName("ActorTimelineFloatReturn"));
	ActorTimelineFinished.BindUFunction(this, FName("OnActorTimelineFinished"));

	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	Cable->bAttachStart = true;
	Cable->bAttachEnd = true;
	Cable->bEnableStiffness = true;
	Cable->SetupAttachment(this->GetRootComponent());

}

// Called when the game starts or when spawned
void ALukeHook::BeginPlay()
{
	Super::BeginPlay();

	if (HookCurve)
	{
		HookTimeline->AddInterpFloat(HookCurve, InterpFunction, FName("Alpha"));
		HookTimeline->SetTimelineFinishedFunc(TimelineFinished);

		HookTimeline->SetLooping(false);

		ActorTimeline->AddInterpFloat(HookCurve, ActorInterpFunction, FName("Alpha"));
		ActorTimeline->SetTimelineFinishedFunc(ActorTimelineFinished);

		ActorTimeline->SetLooping(false);
	}

	Location = CollisionComponent->GetComponentLocation();

	GetWorldTimerManager().SetTimer(AttachTimer, this, &ALukeHook::AttachCableToOwner, 0.1f, false);

	GetWorldTimerManager().SetTimer(HookTimer, this, &ALukeHook::ThrowHook, 0.3f, false);
	
}

void ALukeHook::ThrowHook()
{
	HookTimeline->Play();
}

void ALukeHook::TimelineFloatReturn(float Value)
{
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	FVector NewLocation = FMath::Lerp(CollisionComponent->GetComponentLocation(), Location, Value);
	CollisionComponent->SetWorldLocation(NewLocation);
}

void ALukeHook::OnTimelineFinished()
{
	Destroy();
}

void ALukeHook::ActorTimelineFloatReturn(float Value)
{
	if (HitCharacter)
	{
		FVector NewLocation = FMath::Lerp(HitCharacter->GetActorLocation(), Location, Value);
		HitCharacter->GetMovementComponent()->StopActiveMovement();
		HitCharacter->SetActorLocation(NewLocation);
	}
}

void ALukeHook::OnActorTimelineFinished()
{
	Destroy();
}

void ALukeHook::OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Mesh->SetVisibility(false, false);
	HookTimeline->Play();

	HitCharacter = Cast<ALukeGameCharacter>(OtherActor);
	if (HitCharacter && HitCharacter != GetOwner())
	{
		ActorTimeline->Play();
	}
	else
	{
		Destroy();
	}
}

void ALukeHook::AttachCableToOwner()
{
	ALukeGameCharacter* HookOwner = Cast<ALukeGameCharacter>(GetOwner());
	if (HookOwner)
	{
		Cable->SetAttachEndTo(HookOwner, FName("None"), FName("None"));
	}
}

