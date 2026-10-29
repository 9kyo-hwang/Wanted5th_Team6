// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWBossHitObjectSpawner.generated.h"

UCLASS()
class GTW_API AGTWBossHitObjectSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWBossHitObjectSpawner();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// @TEMP. 현재는 Level Blueprint에서 BeginPlay 시 호출해주자
	UFUNCTION(BlueprintCallable, Category="Boss|Gimmick")
	bool StartSpawning(AActor* InTarget);

	// 클리어 혹은 값 문제 등의 이유로 스포너 멈춰야 할 때
	UFUNCTION(BlueprintCallable, Category = "Boss|Gimmick")
	void StopSpawning();

private:
	void SpawnHitObject();
	void HandleContactEnded();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGTWBossHitObject> HitObjectClass;
	
	// HitObject가 보스 적중하면 Destroy() 호출해서 약참조가 나으려나?
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AGTWBossHitObject> HitObjectInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> TargetActorRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float RespawnTime;

	FTimerHandle SpawnTimerHandle;

	bool bIsSpawning;
};
