// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossGimmickBase.h"
#include "GTWLightningGimmick.generated.h"

UCLASS()
class GTW_API AGTWLightningGimmick : public AGTWBossGimmickBase
{
	GENERATED_BODY()
	
public:	
	AGTWLightningGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void Strike();
	bool TryStrikeTarget();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float StunTime;	// 플레이어 기절 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float StrikeRadius;	// 번개 공격 범위(원) 반지름

	FTimerHandle WarningTimerHandle;
};
