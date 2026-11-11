// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Interfaces/GTWBossAIInterface.h>
#include "GTWBossPhase2Character.generated.h"

class UAnimInstance;
class UAnimMontage;
class AGTWStartMoveSplineActor;
class AGTWMeteorGimmick;
class AGTWLightningGimmick;
class AGTWSwordWaveGimmick;
class UBoxComponent;

USTRUCT(BlueprintType)
struct FGTWBossGimmickSpawnSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Gimmick|Spawn", meta=(ClampMin="0.0", Units = "cm"))
	float ForwardDistance = 0.f;

	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn", meta = (ClampMin = "0.0", Units = "cm"))
	float HorizontalRandomRange = 0.f;
};

UCLASS()
class GTW_API AGTWBossPhase2Character : public ACharacter, public IGTWBossAIInterface
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	AGTWBossPhase2Character();

	virtual void StartCombat() override;
	virtual void StopCombat() override;

public:
	void PlaySwordWaveMontage();
	void PlayMeteorMontage();
	void PlayLightningMontage();

	// 스플라인 갱신 함수(오버랩 트리거 BP에 할당)
	// 다음 스플라인 이동 시에만 호출. 
	UFUNCTION(BlueprintCallable, Category = Spline)
	void NextSpline();

	// 스플라인 실행 함수
	UFUNCTION(BlueprintCallable, Category = Spline)
	void StartSpline();

private:
	// 어택 몽타주 실행
	void PlayAttackMontage(UAnimMontage* Montage);
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void SpawnSwordWaveGimmick();
	void SpawnMeteorGimmick();
	void SpawnLightningGimmick();

	FVector CalculateGimmickSpawnLocation(const FGTWBossGimmickSpawnSettings& Settings) const;

protected:
	// 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<UAnimMontage> SwordAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<UAnimMontage> LightningAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	TObjectPtr<UAnimMontage> MeteorAttack;

	// 스플라인 3개 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spline)
	TArray<TObjectPtr<AGTWStartMoveSplineActor>> BossSplines;
	
	// 투사체 액터 
	// 운석
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<AGTWMeteorGimmick> MeteorClass;

	UPROPERTY(EditAnywhere, Category="Gimmick|Spawn")
	FGTWBossGimmickSpawnSettings MeteorSpawnSettings;

	// 번개
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<AGTWLightningGimmick> LightningClass;
	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn")
	FGTWBossGimmickSpawnSettings LightningSpawnSettings;

	// 검기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GimmickActor)
	TSubclassOf<AGTWSwordWaveGimmick> SwordWaveClass;
	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn")
	FGTWBossGimmickSpawnSettings SwordWaveSpawnSettings;

	// 게임 오버 처리할 박스 콜리전 추가. 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Arrow)
	TObjectPtr<UBoxComponent> DeathBox;

private:
	bool bCombatActive = false;

	// 현재 스플라인 번호
	int32 CurrentSplineNum = 0;

	// ABP 클래스
	TSubclassOf<UAnimInstance> ABPClass;
};
