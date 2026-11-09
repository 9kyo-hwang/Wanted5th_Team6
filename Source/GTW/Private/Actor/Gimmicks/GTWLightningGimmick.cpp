// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWLightningGimmick.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWHitReactionReceiver.h>
#include <Particles/ParticleSystemComponent.h>

#include <Engine/OverlapResult.h>


// Sets default values
AGTWLightningGimmick::AGTWLightningGimmick()
	: WarningDuration(1.f)	// BP에서 조정
	, StunTime(1.f)
	, StrikeRadius(100.f)
	, CurrentState(ELightningState::Ready)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WarningFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WarningFX"));
	WarningFX->SetVisibility(false);
	WarningFX->SetAutoActivate(false);
	WarningFX->SetupAttachment(Root);

	LightningFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LightningFX"));
	LightningFX->SetAutoActivate(false);
	LightningFX->SetupAttachment(Root);
}

// 번개 액터 스폰 시 실행
void AGTWLightningGimmick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentState = ELightningState::Warning;
	
	WarningFX->SetVisibility(true);
	WarningFX->Activate();
	
	LightningFX->OnSystemFinished.AddDynamic(this, &ThisClass::Finish);
	LightningFX->SetVisibility(false);

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
	
	WarningFX->SetVisibility(false);
	WarningFX->Deactivate();

	TryStrikeTarget();

	if (!LightningFX->Template)
	{
		Debug::Print(TEXT("[Lightning::Strike] 번개 FX가 설정이 안돼있어 재생 X"));
		Finish(LightningFX);
		return;
	}

	LightningFX->SetVisibility(true);
	LightningFX->Activate();
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

#if defined(ENABLE_DRAW_DEBUG)
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

// 스폰한 번개 삭제
void AGTWLightningGimmick::Finish(UParticleSystemComponent* InParticleSystemComponent)
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

	Super::EndPlay(EndPlayReason);
}
