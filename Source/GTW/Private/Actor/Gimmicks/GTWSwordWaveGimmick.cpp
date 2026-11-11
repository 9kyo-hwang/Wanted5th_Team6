// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWSwordWaveGimmick.h"

#include <Interfaces/GTWHitReactionReceiver.h>
#include <GTWDebugHelper.h>

#include <Components/BoxComponent.h>
#include <GameFramework/ProjectileMovementComponent.h>


// Sets default values
AGTWSwordWaveGimmick::AGTWSwordWaveGimmick()
	: WarningDuration(1.f)
	, MovementSpeed(100.f)
	, LifeSpan(5.f)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WaveCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WaveCollision"));
	WaveCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaveCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	WaveCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);	// Pawn만 Overlap
	WaveCollision->SetupAttachment(Root);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(WaveCollision);
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bSweepCollision = true;
}

void AGTWSwordWaveGimmick::BeginPlay()
{
	Super::BeginPlay();
	
	WaveCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerEnter);

	if (WarningDuration <= 0.f)
	{
		OnEndWarning();
	}
	else
	{
		GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::OnEndWarning, WarningDuration);
	}	
}

void AGTWSwordWaveGimmick::OnEndWarning()
{
	StartLoopingFX(EGimmickFXTypes::Body, WaveCollision);

	WaveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileMovement->Velocity = GetActorForwardVector() * MovementSpeed;
	ProjectileMovement->Activate();

	SetLifeSpan(LifeSpan);
}

void AGTWSwordWaveGimmick::OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Debug::Print(TEXT("[SwordWave] 플레이어가 검기에 닿았음"));
	if (IGTWHitReactionReceiver* Player = Cast<IGTWHitReactionReceiver>(OtherActor))
	{
		// 넉백을 호출하지만, 실제로는 넉백 없이 랙돌만 발생시키는 목적
		Player->HitKnockbackObject(FVector::RightVector, 0.f);
	}
}

void AGTWSwordWaveGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);

	Super::EndPlay(EndPlayReason);
}
