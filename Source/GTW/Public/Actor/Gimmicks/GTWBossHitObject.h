// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossHitObject.generated.h"

class AGTWBossHitObject;

DECLARE_DELEGATE_OneParam(FOnHitBossDelegate, AGTWBossHitObject* /*this*/);

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
	bool Activate(AActor* InTarget);	// 날아가 맞출 대상(보스)을 인자로 받음
	void Cancel();						// 충돌, Tick 종료 후 제거

	FOnHitBossDelegate OnHitBoss;

private:
	UFUNCTION()
	void OnPlayerContact(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FlyToTarget(float DeltaTime);	// Tick에서 타겟을 향해 이동시키는 함수
	void OnHitTarget();

private:
	// RootComponent 겸 Player 접촉 감지 역할
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> ContactSphere;

	// 오브젝트 외관
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> VisualMesh;

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
};
