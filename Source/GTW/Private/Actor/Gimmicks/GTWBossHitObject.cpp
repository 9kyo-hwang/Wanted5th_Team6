// Fill out your copyright notice in the Description page of Project Settings.


#include <Actor/Gimmicks/GTWBossHitObject.h>

#include <Components/SphereComponent.h>
#include <GameFramework/PlayerController.h>
#include <NiagaraComponent.h>

AGTWBossHitObject::AGTWBossHitObject()
	: FlightSpeed(1000.0f)
	, TargetHeightOffset(88.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;	// 시작과 동시에 Tick은 불필요

	ContactSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ContactSphere"));
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 처음엔 충돌 꺼놓자
	ContactSphere->SetGenerateOverlapEvents(true);	// 오버랩 이벤트 ON
	
	SetRootComponent(ContactSphere);

	ObjectBodyFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ObjectBodyFX"));
	ObjectBodyFX->SetVisibility(false);
	ObjectBodyFX->SetAutoActivate(false);
	ObjectBodyFX->SetAutoDestroy(false);
	ObjectBodyFX->SetupAttachment(ContactSphere);
}

bool AGTWBossHitObject::SetTarget(AActor* InTarget)
{
	if (!IsValid(InTarget))
	{
		return false;
	}

	// @TODO: 만약 소환 위치에 이미 플레이어와 겹쳐있다면?

	TargetActorRef = InTarget;
	return true;
}

void AGTWBossHitObject::BeginPlay()
{
	Super::BeginPlay();

	if (FlightSpeed <= 0.0f)
	{
		return;
	}

	ContactSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ContactSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ThisClass::OnPlayerContact
	);

	ObjectBodyFX->SetVisibility(true);
	ObjectBodyFX->Activate();
}

void AGTWBossHitObject::OnPlayerContact(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OverlappingPawn->GetController());
	if (!PlayerController)
	{
		return;
	}

	// @TEMP: BP Character 가지고 검사하기?

	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 플레이어 탐지했으니 역할 끝
	SetActorTickEnabled(true);	// Tick 발동 -> 이동 시작
}

void AGTWBossHitObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FlyToTarget(DeltaTime);
}

void AGTWBossHitObject::FlyToTarget(float DeltaTime)
{
	AActor* Target = TargetActorRef.Get();
	if (!ensureMsgf(IsValid(Target), TEXT("[ERROR] %s lost its target during flight."), *GetName()))
	{
		Destroy();
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation() + FVector::UpVector * TargetHeightOffset;
	const FVector Direction = TargetLocation - CurrentLocation;

	const float RemainDistance = Direction.Size();	// 남은 거리
	const float ToMoveDistance = FlightSpeed * DeltaTime;	// 이동할 거리

	if (RemainDistance <= ToMoveDistance)
	{
		// bSweep: 충돌 영역을 "휩쓰는" 판정으로 하지 않도록(날아가는 도중 충돌 없도록)
		SetActorLocation(TargetLocation, false);
		OnHitTarget();
		return;
	}

	const FVector NextLocation = CurrentLocation + Direction.GetSafeNormal() * ToMoveDistance;
	SetActorLocation(NextLocation, false);
}

// Flying 상태에서 보스에게 도달했을 때
void AGTWBossHitObject::OnHitTarget()
{
	SetActorTickEnabled(false);

	// 보스 적중했음을 알림(맞춘 오브젝트 == 나)
	OnHitBoss.ExecuteIfBound(this);

	if (!IsActorBeingDestroyed())
	{
		ObjectBodyFX->Deactivate();
		ObjectBodyFX->SetVisibility(false);

		Destroy();
	}
}

void AGTWBossHitObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}
