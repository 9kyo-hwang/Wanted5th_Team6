// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossGimmickBase.generated.h"

class USceneComponent;

UENUM(BlueprintType)
enum class EGimmickFXTypes : uint8
{
	// 필요하면 추가
	Warning,	// 경고
	Body,		// 몸체
	Triggered,	// 
	Impact,		// 어디 닿았을 때
};

// 기믹들 이펙트 제어 통일하려고 도입한 베이스
UCLASS(Abstract, Blueprintable)
class GTW_API AGTWBossGimmickBase : public AActor
{
	GENERATED_BODY()

protected:
	void PlayBurstFX(EGimmickFXTypes Type, const FTransform& WorldTransform);
	void StartLoopingFX(EGimmickFXTypes Type, USceneComponent* AttachTo);
	void StopLoopingFX(EGimmickFXTypes Type);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Boss|Gimmick|FX")
	void OnPlayBurstFX(EGimmickFXTypes Type, const FTransform& WorldTransform);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Gimmick|FX")
	void OnStartLoopingFX(EGimmickFXTypes Type, USceneComponent* AttachTo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Gimmick|FX")
	void OnStopLoopingFX(EGimmickFXTypes Type);

private:
	TSet<EGimmickFXTypes> ActiveLoopingFX;
	bool bIsEndingPlay = false;
};
