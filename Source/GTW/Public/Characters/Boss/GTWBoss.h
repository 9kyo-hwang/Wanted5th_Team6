// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Interfaces/GTWBossAIInterface.h>
#include "GTWBoss.generated.h"

UCLASS()
class GTW_API AGTWBoss : public ACharacter, public IGTWBossAIInterface
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	AGTWBoss();

	virtual void StartCombat() override;
	virtual void StopCombat() override;

public:
	// 검기 실행 함수
	UFUNCTION()
	void PlaySwordWave();
	// 공 실행 함수
	void PlaySpawnBall();
	// 번개 실행 함수
	void PlayLightning();

	// 스플라인 갱신 함수(오버랩 트리거 BP에 할당)
	// 다음 스플라인 이동 시에만 호출. 
	UFUNCTION(BlueprintCallable, Category = Spline)
	void NextSpline();

	// 스플라인 실행 함수
	UFUNCTION(BlueprintCallable, Category = Spline)
	void StartSpline();

public:
	// 게임 오버 처리할 박스 콜리전 추가. 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Arrow)
	class UBoxComponent* DeathBox;

protected:
	// 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> SwordAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> LightningAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<class UAnimMontage> BallAttack;

	// 스플라인 3개 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spline)
	TArray<TObjectPtr<class AGTWStartMoveSplineActor>> BossSplines;
	
	// 투사체 액터 
	// 운석
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<class AGTWMeteorGimmick> MeteorClass;

	// 번개
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<class AGTWLightningGimmick> LightningClass;

	// 검기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<class AGTWSwordWaveGimmick> SwordWaveClass;


private:
	// 어택 몽타주 실행
	void PlayAttackAnim(UAnimMontage* AttackMontage);

private:
	// 현재 스플라인 번호
	int CurrentSplineNum = 0;

};
