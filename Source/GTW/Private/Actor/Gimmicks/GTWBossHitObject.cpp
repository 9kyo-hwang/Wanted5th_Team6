// Fill out your copyright notice in the Description page of Project Settings.


#include <Actor/Gimmicks/GTWBossHitObject.h>

#include <Components/SphereComponent.h>
#include <GameFramework/PlayerController.h>

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

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(ContactSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGTWBossHitObject::BeginPlay()
{
	Super::BeginPlay();

	ContactSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ThisClass::OnPlayerContact
	);
}

void AGTWBossHitObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FlyToTarget(DeltaTime);
}

bool AGTWBossHitObject::Activate(AActor* InTarget)
{
	if (!IsValid(InTarget) || FlightSpeed <= 0.0f)
	{
		return false;
	}

	// @TODO: 만약 소환 위치에 이미 플레이어와 겹쳐있다면?

	TargetActorRef = InTarget;

	// 활성화 되었으므로 QueryOnly로 충돌 판정 ON
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	return true;
}

// 플레이어가 공략 실패 or 보스 클리어 시(혹시나) or 비정상 상황인 경우 다 정리하는 역할
void AGTWBossHitObject::Cancel()
{
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);

	Destroy();
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

void AGTWBossHitObject::FlyToTarget(float DeltaTime)
{
	AActor* Target = TargetActorRef.Get();
	if (!ensureMsgf(IsValid(Target), TEXT("[ERROR] %s lost its target during flight."), *GetName()))
	{
		Cancel();
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

	Destroy();
}
