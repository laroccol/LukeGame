// Fill out your copyright notice in the Description page of Project Settings.


#include "LukeGameLazer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "LukeGameCharacter.h"
#include "TimerManager.h"

// Sets default values
ALukeGameLazer::ALukeGameLazer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USphereComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	LazerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LazerMesh"));
	LazerMesh->SetupAttachment(RootComponent);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 4500.0f;

	LazerEnd = CreateDefaultSubobject<USphereComponent>(TEXT("LazerEnd"));
	LazerEnd->SetupAttachment(CameraBoom);

	LazerEnd->OnComponentBeginOverlap.AddDynamic(this, &ALukeGameLazer::OnOverlapBegin);
	LazerEnd->OnComponentEndOverlap.AddDynamic(this, &ALukeGameLazer::OnOverlapEnd);

	BeamOriginalLength = 0.0f;

	bReplicates = true;

}

// Called when the game starts or when spawned
void ALukeGameLazer::BeginPlay()
{
	Super::BeginPlay();

	FVector SocketLocation = LazerMesh->GetSocketLocation(FName("end"));
	FVector ActorLocation = GetActorLocation();

	BeamOriginalLength = UKismetMathLibrary::Vector4_Size(SocketLocation - ActorLocation);
	UE_LOG(LogTemp, Warning, TEXT("Original Beam Length: %d"), BeamOriginalLength);

	GetWorldTimerManager().SetTimer(DamageDelay, this, &ALukeGameLazer::DealDamage, 0.05f, true);
}

// Called every frame
void ALukeGameLazer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ALukeGameLazer::DealDamage_Validate()
{
	return true;
}

void ALukeGameLazer::DealDamage_Implementation()
{
	for (ALukeGameCharacter* Character : OverlappingActors)
	{
		if (Character)
		{
			UE_LOG(LogTemp, Warning, TEXT("Character was Valid"));
			FPointDamageEvent PointDmg;
			PointDmg.ShotDirection = FVector(0, 0, 0);
			PointDmg.Damage = DAMAGE_PER_TICK;

			Character->TakeDamage(PointDmg.Damage, PointDmg, nullptr, nullptr);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Character was not Valid"));
		}

	}
}

void ALukeGameLazer::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ALukeGameCharacter* Character = Cast<ALukeGameCharacter>(OtherActor);
	if (Character)
	{
		if (Character != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
		{
			OverlappingActors.AddUnique(Character);
		}
	}
}

void ALukeGameLazer::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ALukeGameCharacter* Character = Cast<ALukeGameCharacter>(OtherActor);
	if (Character)
	{
		OverlappingActors.Remove(Character);
	}
}

