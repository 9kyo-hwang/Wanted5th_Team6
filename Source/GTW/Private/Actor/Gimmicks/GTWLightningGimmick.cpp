// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWLightningGimmick.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWHitReactionReceiver.h>

#include <Engine/OverlapResult.h>

// Sets default values
AGTWLightningGimmick::AGTWLightningGimmick()
	: WarningDuration(1.f)	// BP에서 조정
	, VisualDuration(1.f)
	, StunTime(1.f)
	, StrikeRadius(10.f)
	, CurrentState(ELightningState::Ready)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WarningMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningMarker"));
	WarningMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 메시는 충돌 X
	WarningMarker->SetVisibility(false);									// 메시는 처음에 표시 X
	WarningMarker->SetupAttachment(Root);

	LightningVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightningVisual"));
	LightningVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 메시는 충돌 X
	LightningVisual->SetupAttachment(Root);
}

// 번개 액터 스폰 시 실행
void AGTWLightningGimmick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentState = ELightningState::Warning;
	WarningMarker->SetVisibility(true);
	LightningVisual->SetVisibility(false);

	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::Strike, WarningDuration);
}

// Warning 마커 지우고 -> 번개 표시하고 -> 판정
void AGTWLightningGimmick::Strike()
{
	if (CurrentState != ELightningState::Warning)
	{
		return;
	}

	CurrentState = ELightningState::Struck;
	WarningMarker->SetVisibility(false);
	LightningVisual->SetVisibility(true);

	GetWorldTimerManager().SetTimer(VisualTimerHandle, this, &ThisClass::Finish, VisualDuration);

	if (AActor* Target = nullptr; FindStrikeTarget(Target))
	{
		if (IGTWHitReactionReceiver* Receiver = Cast<IGTWHitReactionReceiver>(Target))
		{
			Debug::Print(TEXT("[Lightning] Player에게 Stun!"));
			Receiver->HitStunObject(StunTime);
		}
	}
}

// 번개 치는 범위 내에 플레이어가 있는지 검사 후 반환
bool AGTWLightningGimmick::FindStrikeTarget(AActor*& OutTarget)
{
	OutTarget = nullptr;

	UWorld* World = GetWorld();
	if (!World || StrikeRadius <= 0.f)
	{
		Debug::Print(TEXT("[Lightning] World가 없거나 StrikeRadius 값이 유효하지 않음"));
		return false;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);	// Pawn 쿼리 타입'만'

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(LightningStrike),	/*TraceTag*/
		false,								/*TraceComplex*/
		this								/*IgnoreActor*/
	);

	if (!World->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(StrikeRadius),
		QueryParams
	))
	{
		Debug::Print(FString::Printf(TEXT("[Lightning | %s] 현재 범위에 Pawn 타입 객체 없음"), *GetName()));
		return false;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!IsValid(Target))
		{
			continue;
		}

		if (Cast<IGTWHitReactionReceiver>(Target))
		{
			Debug::Print(FString::Printf(TEXT("[Lightning | %s] 플레이어 발견!"), *GetName()));
			OutTarget = Target;
			return true;
		}
	}

	Debug::Print(FString::Printf(TEXT("[Lightning | %s] 현재 범위에 플레이어(Receiver 구현체) 없음"), *GetName()));
	return false;
}

// 스폰한 번개 삭제
void AGTWLightningGimmick::Finish()
{
	if (CurrentState != ELightningState::Struck)
	{
		return;
	}

	Destroy();
}

// 타이머 핸들 정리
void AGTWLightningGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	GetWorldTimerManager().ClearTimer(VisualTimerHandle);

	Super::EndPlay(EndPlayReason);
}
