// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWLightningGimmick.generated.h"

UENUM(BlueprintType)
enum class ELightningState : uint8
{
	Ready,
	Warning,
	Struck,
	Finished
};

UCLASS()
class GTW_API AGTWLightningGimmick : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWLightningGimmick();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Strike();
	void Finish();

protected:
	// VFX, SFX 실행을 위한 BP 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category="Boss|Gimmick|Lightning")
	void OnPlayLightningEffects();

private:
	bool FindStrikeTarget(AActor*& OutTarget);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> WarningMarker;	// 예고 지점 표시할 표식.

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> LightningVisual;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float VisualDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float StunTime;	// 플레이어 기절 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float StrikeRadius;	// 번개 공격 범위(원) 반지름

	ELightningState CurrentState;
	FTimerHandle WarningTimerHandle;
	FTimerHandle VisualTimerHandle;
};
