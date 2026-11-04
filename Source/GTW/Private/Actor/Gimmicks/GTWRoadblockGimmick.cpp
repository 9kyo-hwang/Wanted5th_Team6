// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWRoadblockGimmick.h"

#include <Characters/Player/GTWPlayerCharacter.h>

#include <Components/BoxComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Components/ArrowComponent.h>
#include <Components/SphereComponent.h>
#include <GameFramework/PlayerController.h>

// Sets default values
AGTWRoadblockGimmick::AGTWRoadblockGimmick()
	: WarningDuration(1.f)	// BP에서 조정
	, FlightDuration(1.f)	// BP에서 조정
	, CurrentState(ERoadblockState::Waiting)
	, FlightElapsedTime(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Root);

	LauncherMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LauncherMesh"));
	LauncherMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 메시는 충돌 X
	LauncherMesh->SetupAttachment(Root);
	
	LaunchPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LaunchPoint"));
	LaunchPoint->SetupAttachment(Root);
	
	TargetPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("TargetPoint"));
	TargetPoint->SetupAttachment(Root);

	WarningMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningMarker"));
	WarningMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 메시는 충돌 X
	WarningMarker->SetVisibility(false);									// 메시는 처음에 표시 X
	WarningMarker->SetupAttachment(TargetPoint);

	BlockCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BlockCollision"));
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 처음에 충돌 없고, 타겟 위치에 도달하면 ON
	BlockCollision->SetupAttachment(Root);

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);			// 메시는 충돌 X
	BlockMesh->SetupAttachment(BlockCollision);
}

// Overlap 연결
void AGTWRoadblockGimmick::BeginPlay()
{
	Super::BeginPlay();

	BlockMesh->SetVisibility(false);	// 게임 시작 시 길막 구체 Visible OFF
	
	FlightStartLocation = LaunchPoint->GetComponentLocation();
	FlightTargetLocation = TargetPoint->GetComponentLocation();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerEnter);
}

void AGTWRoadblockGimmick::OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CurrentState != ERoadblockState::Waiting)
	{
		return;
	}

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
		
	CurrentState = ERoadblockState::Warning;
	Warning();
}

// 경고 마커 표시 및 발사 준비
void AGTWRoadblockGimmick::Warning()
{
	if (CurrentState != ERoadblockState::Warning)
	{
		return;
	}

	// 마커 표시(이미 TargetPoint 자손으로 세팅해둬서 위치는 그대로)
	WarningMarker->SetVisibility(true);

	// WarningDuration 동안 표시한 후, 발사 수행
	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::Launch, WarningDuration);
}

// 발사
void AGTWRoadblockGimmick::Launch()
{
	if (CurrentState != ERoadblockState::Warning)
	{
		return;
	}

	// 충돌체 세팅
	BlockCollision->SetWorldLocation(FlightStartLocation);
	BlockMesh->SetVisibility(true);
	FlightElapsedTime = 0.f;

	// 이동 시작
	CurrentState = ERoadblockState::Flying;
	SetActorTickEnabled(true);
}

// 길막 액터 이동
void AGTWRoadblockGimmick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState != ERoadblockState::Flying)
	{
		return;
	}

	FlightElapsedTime += DeltaTime;

	// 진행도: ElapsedTime / Duration
	const float Alpha = FMath::Clamp(FlightElapsedTime / FlightDuration, 0.f, 1.f);
	const FVector NewLocation = FMath::Lerp(FlightStartLocation, FlightTargetLocation, Alpha);
	BlockCollision->SetWorldLocation(NewLocation);

	if (FlightElapsedTime >= FlightDuration)
	{
		Finish();
	}
}

// 도착, 마커 제거 및 충돌 활성화
void AGTWRoadblockGimmick::Finish()
{
	if (CurrentState != ERoadblockState::Flying)
	{
		return;
	}

	BlockCollision->SetWorldLocation(FlightTargetLocation);
	SetActorTickEnabled(false);
	CurrentState = ERoadblockState::Blocked;
	WarningMarker->SetVisibility(false);
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);	// 무슨 타입이 맞을까
}

// 정리
void AGTWRoadblockGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}
