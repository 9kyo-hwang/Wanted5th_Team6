// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossGimmickBase.h"
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
class GTW_API AGTWMeteorGimmick : public AGTWBossGimmickBase
{
	GENERATED_BODY()
	
public:	
	AGTWMeteorGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void Falling();
	void Grounded();
	void Knockback();
	void Finish();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> MeteorCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float FallHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	float FallSpeed;

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
