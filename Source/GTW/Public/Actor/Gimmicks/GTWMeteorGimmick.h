// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWMeteorGimmick.generated.h"

UENUM(BlueprintType)
enum class EMeteorState : uint8
{
	Ready,
	Warning,
	Falling,
	Grounded,
	Finished
};

UCLASS()
class GTW_API AGTWMeteorGimmick : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWMeteorGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Falling();
	void Grounded();

private:
	void Knockback();
	void Finish();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> WarningMarker;	// 예고 지점 표시할 표식.

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> MeteorCollision;				// 운석 충돌체

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> MeteorVisual;	// 운석 메시

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;	// 마커 표시 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float FallHeight;		// 낙하 시작 높이. 얘가 필요할까? 조금 더 고민해보자.

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float FallSpeed;		// 운석 낙하 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float GroundedDuration;	// 운석이 땅에 박힌 후 지속 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float KnockbackRadius;	// 운석의 넉백 판정 범위

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float KnockbackPower;	// 운석의 넉백 힘

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float KnockbackInterval;	// 넉백 간격

	EMeteorState CurrentState = EMeteorState::Ready;
	FTimerHandle WarningTimerHandle;
	FTimerHandle GroundedTimerHandle;
	FTimerHandle KnockbackTimerHandle;
	FVector GroundLocation = FVector::ZeroVector;
};
