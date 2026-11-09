// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWLightningGimmick.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWHitReactionReceiver.h>

#include <Engine/OverlapResult.h>

AGTWLightningGimmick::AGTWLightningGimmick()
	: WarningDuration(1.f)	// BP에서 조정
	, StunTime(1.f)
	, StrikeRadius(100.f)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

// 번개 액터 스폰 시 실행
void AGTWLightningGimmick::BeginPlay()
{
	Super::BeginPlay();
	
	StartLoopingFX(EGimmickFXTypes::Warning, Root);

	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::Strike, WarningDuration);
}

// Warning 마커 지우고 -> 번개 표시하고 -> 판정
void AGTWLightningGimmick::Strike()
{
	StopLoopingFX(EGimmickFXTypes::Warning);

	PlayBurstFX(EGimmickFXTypes::Impact, Root->GetComponentToWorld());
	
	TryStrikeTarget();

	Destroy();
}

// 번개 치는 범위 내에 플레이어가 있으면 타격하는 API
bool AGTWLightningGimmick::TryStrikeTarget()
{
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

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(
		World,
		GetActorLocation(),
		StrikeRadius,
		/*Segments*/ 16,
		/*Color*/ FColor::Red,
		/*PersistentLines*/ false,
		/*LifeTime*/ 2.f,
		/*DepthPriority*/ 0,
		/*Thickness*/ 2.f
	);
#endif

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

		if (IGTWHitReactionReceiver* Receiver = Cast<IGTWHitReactionReceiver>(Target))
		{
			Debug::Print(FString::Printf(TEXT("[Lightning | %s] Player에게 Stun!"), *GetName()));
			Receiver->HitStunObject(StunTime);
			return true;
		}
	}

	Debug::Print(FString::Printf(TEXT("[Lightning | %s] 현재 범위에 플레이어(Receiver 구현체) 없음"), *GetName()));
	return false;
}

// 타이머 핸들 정리
void AGTWLightningGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);

	Super::EndPlay(EndPlayReason);
}
