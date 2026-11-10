// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWGeometryCollectionTable.h"
#include "Engine/OverlapResult.h"
#include <GTWDebugHelper.h>

#include <GeometryCollection/GeometryCollectionComponent.h>
#include <Characters/Boss/GTWBossPhase2Character.h>

void AGTWGeometryCollectionTable::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	FVector Center; // 액터 중심
	FVector Extent; // 각 축의 반길이
	GetActorBounds(false, Center, Extent);

	// 충돌 정보를 저장할 변수
	FHitResult OutHit;
	TArray<FOverlapResult> OutHits;
	// 충돌 쿼리 파라미터
	FCollisionQueryParams CollisionParams;
	// 본인 충돌 무시
	CollisionParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByChannel(
		OutHits,							// 충돌 결과 저장 변수
		Center,								// 레이 시작지점
		FQuat::Identity,					// 회전: 기본값
		ECC_Visibility,						// 충돌 채널
		FCollisionShape::MakeBox(Extent),	// 충돌 형태
		CollisionParams						// 츙돌 쿼리 파라미터
	);

	// 디버그용 박스
	DrawDebugBox(
		GetWorld(),
		Center,
		Extent, 
		FColor::Green,
		false,
		-1.f
	);

	if (!TargetClass)
	{
		Debug::Print(TEXT("[AGTWGeometryCollectionTable] TargetClass가 초기화되지 않음!!"));
	}

	TSet<AActor*> ProcessedActors;
	for (const FOverlapResult& OverlapActor : OutHits)
	{
		AActor* Target = OverlapActor.GetActor();
		// 유효하지 않은 액터는 건너뛰기
		if (!IsValid(Target) || !Target->IsA(TargetClass.Get()) ||
			ProcessedActors.Contains(Target))
		{
			continue;
		}
		ProcessedActors.Add(Target);

		const FVector TargetCenter = Target->GetActorLocation();
		// 타깃 주위 검사 반경
		const float ReleaseRadius = 1000.f;

		UGeometryCollectionComponent* Geometry
			= Cast<UGeometryCollectionComponent>(this->GetGeometryCollectionComponent());

		if (IsValid(Geometry))
		{
			// 범위 안 조각을 Kinematic → Dynamic으로 전환
			Geometry->ApplyKinematicField(ReleaseRadius, TargetCenter);
		}

		// 디버그용 구체 
		DrawDebugSphere(
			GetWorld(),
			TargetCenter,
			ReleaseRadius,
			32,
			FColor::Red,
			false,
			-1.f
		);
	}
}