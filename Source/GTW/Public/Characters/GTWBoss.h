// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GTWBoss.generated.h"

UCLASS()
class GTW_API AGTWBoss : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGTWBoss();

	// 검기 실행 함수
	void PlaySwordWave();
	// 공 실행 함수
	void PlaySpawnBall();
	// 번개 실행 함수
	void PlayLightning();
protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> SwordAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> LightningAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> BallAttack;

private:
	// 어택 몽타주 실행
	void PlayAttackAnim(UAnimMontage* AttackMontage);


};
