// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossHitObject.generated.h"

UENUM()
enum class EBossHitObjectState : uint8
{
	Inactive,			// 초기화 및 이벤트 연결 완료 전(접촉 안받도록)
	WaitingForContact,	// 맵에 소환되어 플레이어 접촉 대기
	Flying,				// 제한시간 안에 플레이어와 접촉해서 날아가는 중
	Finished,			// Hit or Expired or Cancel 시 진입
};

/*
* 보스 1페이즈에서, 플레이어와 부딪히면 보스에게 날아가 유효타를 입히는 오브젝트
*/
UCLASS()
class GTW_API AGTWBossHitObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWBossHitObject();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// @TEMP: 임시로 BP에서 함수 호출 가능하도록. Event BeginPlay(이벤트 바인딩) -> Activate(타겟 세팅)
	UFUNCTION(BlueprintCallable, Category="Boss|Hit Object")
	bool Activate(AActor* InTarget);	// 날아가 맞출 대상(보스)을 인자로 받음
	void Cancel();						// 타이머, 충돌, Tick 종료 후 제거

private:
	// Player가 오브젝트와 충돌했을 때 발동할 OnComponentBeginOverlap Delegate Callback
	UFUNCTION()
	void OnPlayerContact(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 제한 시간 내에 플레이어가 접촉하지 못했을 때 발동할 Callback
	void OnContactExpired();

	// Tick에서 타겟을 향해 이동시키는 함수
	void FlyToTarget(float DeltaTime);
	void OnHitTarget();

private:
	// RootComponent 겸 Player 접촉 감지 역할
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USphereComponent> ContactSphere;

	// 오브젝트 외관
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> VisualMesh;

	// 오브젝트 소환 후 플레이어가 맞힐 수 있는 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float ContactLifetime;

	// 날아가는 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float FlightSpeed;

	// @TEMP: 타겟 적중 위치가 발이라 위로 올리기 위한 오프셋
	// 나중에 보스 쪽에 Socket을 두고 그쪽으로 날려버려?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true", Units="cm"))
	float TargetHeightOffset;

	// 날아가서 명중시킬 대상 보스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> TargetActorRef;

	// 소환 후 유지시간
	FTimerHandle ContactLifetimeTimerHandle;	

	EBossHitObjectState CurrentState;
};
