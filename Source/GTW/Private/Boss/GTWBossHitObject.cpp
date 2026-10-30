// Fill out your copyright notice in the Description page of Project Settings.


#include <Characters/Boss/GTWBossHitObject.h>
#include <Components/SphereComponent.h>
#include <GameFramework/Character.h>

// Sets default values
AGTWBossHitObject::AGTWBossHitObject()
	: ContactLifetime(10.f)
	, FlightSpeed(1000.f)
	, TargetHeightOffset(88.f)
	, CurrentState(EBossHitObjectState::Inactive)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;	// 시작과 동시에 Tick은 불필요

	ContactSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ContactSphere"));
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 처음엔 충돌 꺼놓자
	ContactSphere->SetGenerateOverlapEvents(true);	// 오버랩 이벤트 ON
	// @TODO: 얘 전용 충돌 프로파일을 하나 둘까?
	
	SetRootComponent(ContactSphere);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(ContactSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGTWBossHitObject::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 감지
	ContactSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ThisClass::OnPlayerContact
	);
}

void AGTWBossHitObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState != EBossHitObjectState::Flying)
	{
		return;
	}

	FlyToTarget(DeltaTime);
}

bool AGTWBossHitObject::Activate(AActor* InTarget)
{
	// @TODO: 테스트는 Level BP에서, 추후 Spawner를 두자.

	if (CurrentState != EBossHitObjectState::Inactive)
	{
		return false;
	}

	if (!IsValid(InTarget) || FlightSpeed <= 0.f || ContactLifetime <= 0.f)
	{
		return false;
	}

	// @TODO: 만약 소환 위치에 이미 플레이어와 겹쳐있다면?

	TargetActorRef = InTarget;
	CurrentState = EBossHitObjectState::WaitingForContact;

	// 소환 유지 시간 타이머 ON
	GetWorldTimerManager().SetTimer(
		ContactLifetimeTimerHandle,
		this,
		&ThisClass::OnContactExpired,
		ContactLifetime,
		false
	);

	// 활성화 되었으므로 QueryOnly로 충돌 판정 ON
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	return true;
}

// 플레이어가 공략 실패 or 보스 클리어 시(혹시나) or 비정상 상황인 경우 다 정리하는 역할
void AGTWBossHitObject::Cancel()
{
	if (CurrentState == EBossHitObjectState::Finished)
	{
		return;
	}

	// 여기서는 아무런 통지를 날리지 않음(Spawner 측에서 다 정리해야 하며, 개별 오브젝트는 스스로의 정리만 담당)

	CurrentState = EBossHitObjectState::Finished;
	GetWorldTimerManager().ClearTimer(ContactLifetimeTimerHandle);
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);
	Destroy();
}

void AGTWBossHitObject::OnPlayerContact(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CurrentState != EBossHitObjectState::WaitingForContact)
	{
		return;
	}

	// @TODO: 우리가 사용할 BP_{} Player로 캐스팅 필요
	if (ACharacter* Player = Cast<ACharacter>(OtherActor))
	{
		CurrentState = EBossHitObjectState::Flying;
		GetWorldTimerManager().ClearTimer(ContactLifetimeTimerHandle);
		ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 플레이어 탐지했으니 역할 끝
		SetActorTickEnabled(true);	// 이때부터 Tick이 발동하여 이동 시작

		// Spawner에게 재생성 통지
		OnContactEnded.ExecuteIfBound();
	}
}

void AGTWBossHitObject::OnContactExpired()
{
	if (CurrentState != EBossHitObjectState::WaitingForContact)
	{
		return;
	}

	CurrentState = EBossHitObjectState::Finished;
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Spawner에게 재생성 통지
	OnContactEnded.ExecuteIfBound();

	Destroy();
}

void AGTWBossHitObject::FlyToTarget(float DeltaTime)
{
	AActor* Target = TargetActorRef.Get();
	if (!ensureMsgf(IsValid(Target), TEXT("[ERROR] %s lost its target during flight."), *GetName()))
	{
		// 사실 여기 들어오면 비정상 상황
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
	check(CurrentState == EBossHitObjectState::Flying);
	CurrentState = EBossHitObjectState::Finished;
	SetActorTickEnabled(false);

	// @TODO: Target한테 명중 이벤트 전달

	Destroy();
}
