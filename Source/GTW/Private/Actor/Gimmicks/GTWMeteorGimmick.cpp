// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWMeteorGimmick.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWHitReactionReceiver.h>

#include <Components/SphereComponent.h>
#include <Engine/OverlapResult.h>

AGTWMeteorGimmick::AGTWMeteorGimmick()
	: WarningDuration(1.f)
	, FallHeight(1000.f)
	, FallSpeed(500.f)
	, GroundedDuration(3.f)
	, KnockbackRadius(100.f)
	, KnockbackPower(100.f)
	, KnockbackInterval(0.1f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeteorCollision = CreateDefaultSubobject<USphereComponent>(TEXT("MeteorCollision"));
	MeteorCollision->SetGenerateOverlapEvents(true);
	MeteorCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeteorCollision->SetupAttachment(Root);
}

// 소환과 동시에 마커 표시 -> 시간 경과 시 떨어지기 시작
void AGTWMeteorGimmick::BeginPlay()
{
	Super::BeginPlay();
	
	if (CurrentState != EMeteorState::Ready) 
	{
		return;
	}

	CurrentState = EMeteorState::Warning;

	StartLoopingFX(EGimmickFXTypes::Warning, Root);

	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::Falling, WarningDuration);
}

void AGTWMeteorGimmick::Falling()
{
	if (CurrentState != EMeteorState::Warning)
	{
		return;
	}

	CurrentState = EMeteorState::Falling;

	GroundLocation = MeteorCollision->GetComponentLocation();	// 월드 공간의 컴포넌트 위치
	MeteorCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 떨어질 때는 여전히 충돌이 없어야 함

	// 컴포넌트는 애초부터 땅에 있으니, Height만큼 높은 위치를 시작으로 지정
	const FVector StartLocation = GroundLocation + FVector::UpVector * FallHeight;
	MeteorCollision->SetWorldLocation(StartLocation);

	StartLoopingFX(EGimmickFXTypes::Body, MeteorCollision);

	// Interval만큼 knockback 함수를 "반복해서" 호출
	GetWorldTimerManager().SetTimer(KnockbackTimerHandle, this, &ThisClass::Knockback, KnockbackInterval, true);
	
	SetActorTickEnabled(true);	// 이동 시작
}

void AGTWMeteorGimmick::Knockback()
{
	UWorld* World = GetWorld();
	if (!World || KnockbackRadius <= 0.f)
	{
		Debug::Print(TEXT("[Meteor] World가 없거나 넉백 범위가 유효하지 않음"));
		return;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);	// Pawn 쿼리 타입'만'

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(MeteorKnockback),	/*TraceTag*/
		false,								/*TraceComplex*/
		this								/*IgnoreActor*/
	);

	const FVector MeteorLocation = MeteorCollision->GetComponentLocation();

#if 0
	DrawDebugSphere(
		World,
		MeteorLocation,
		KnockbackRadius,
		16,
		FColor::Red,
		false,
		GroundedDuration,
		0,
		2.f
	);
#endif

	if (!World->OverlapMultiByObjectType(
		Overlaps,
		MeteorLocation,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(KnockbackRadius),
		QueryParams
	))
	{
		//Debug::Print(FString::Printf(TEXT("[Meteor | %s] 현재 범위에 Pawn 타입 객체 없음"), *GetName()));
		return;
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
			//Debug::Print(FString::Printf(TEXT("[Meteor | %s] 플레이어 발견!"), *GetName()));
			
			// XY축 방향만 계산
			FVector KnockbackDirection = (Target->GetActorLocation() - MeteorLocation).GetSafeNormal2D();
			if (KnockbackDirection.IsNearlyZero())
			{
				// 0도 ~ 360도 중 한 각도에 대해 cos(x성분), sin(y성분)
				const float Angle = FMath::FRandRange(0, 2.f * PI);
				KnockbackDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
			}

			Receiver->HitKnockbackObject(
				KnockbackDirection,
				KnockbackPower
			);

			return;
		}
	}

	//Debug::Print(FString::Printf(TEXT("[Meteor | %s] 현재 범위에 플레이어(Receiver 구현체) 없음"), *GetName()));
}

void AGTWMeteorGimmick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState != EMeteorState::Falling)
	{
		return;
	}

	const FVector CurrentLocation = MeteorCollision->GetComponentLocation();
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, GroundLocation, DeltaTime, FallSpeed);	// 고정 간격으로 보간해서 이동처리
	MeteorCollision->SetWorldLocation(NewLocation);

	if (NewLocation.Equals(GroundLocation, 0.1f))
	{
		Grounded();
	}
}

// 메테오가 땅 위치에 도달했다면
void AGTWMeteorGimmick::Grounded()
{
	if (CurrentState != EMeteorState::Falling)
	{
		return;
	}

	CurrentState = EMeteorState::Grounded;
	SetActorTickEnabled(false);
	StopLoopingFX(EGimmickFXTypes::Warning);
	
	MeteorCollision->SetWorldLocation(GroundLocation);
	MeteorCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);	// @TODO: Pawn Block으로 설정해줘야 함

	PlayBurstFX(EGimmickFXTypes::Impact, MeteorCollision->GetComponentToWorld());

	// Finish에 진입하기 전까지 계속 넉백 판정을 일으킴
	GetWorldTimerManager().SetTimer(GroundedTimerHandle, this, &ThisClass::Finish, GroundedDuration, false);
}

void AGTWMeteorGimmick::Finish()
{
	if (CurrentState != EMeteorState::Grounded)
	{
		return;
	}

	CurrentState = EMeteorState::Finished;
	StopLoopingFX(EGimmickFXTypes::Body);
	Destroy();
}

void AGTWMeteorGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	GetWorldTimerManager().ClearTimer(KnockbackTimerHandle);
	GetWorldTimerManager().ClearTimer(GroundedTimerHandle);
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}
