// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWLaunchPadGimmick.h"

#include <Components/BoxComponent.h>
#include <GameFramework/Character.h>

// Sets default values
AGTWLaunchPadGimmick::AGTWLaunchPadGimmick()
	: LifeSpan(3.f)
	, LaunchVelocity(500.f)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetupAttachment(Root);
}

void AGTWLaunchPadGimmick::BeginPlay()
{
	Super::BeginPlay();

	Mesh->SetVisibility(true);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerEnter);
	SetLifeSpan(LifeSpan);
}

void AGTWLaunchPadGimmick::OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character || !Character->IsPlayerControlled())
	{
		return;
	}

	Character->LaunchCharacter(
		FVector::UpVector * LaunchVelocity,
		false,						// XY override. 기존 수평 속도 유지
		true						// Z override. 수직 방향 속도 변수로 분리
	);
}
