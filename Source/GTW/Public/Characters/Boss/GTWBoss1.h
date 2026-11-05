#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GTWBoss1.generated.h"

class ATriggerBox;
class UBoxComponent;
class UAnimMontage;
struct FCollisionQueryParams;

/** P2처럼 BT Task가 공격 함수를 호출한다. 수치/에셋은 BP, 이동 범위는 레벨 Trigger Box에서 편집한다. */
UCLASS()
class GTW_API AGTWBoss1 : public ACharacter
{
    GENERATED_BODY()
public:
    virtual void Tick(float DeltaSeconds) override;
    /** 레벨에 배치한 고정 Trigger Box. 위치/회전/Box Extent로 사각 이동 범위를 편집한다. */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Boss|Movement") TObjectPtr<ATriggerBox> MovementBoundsActor;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Movement", meta=(ClampMin="0")) float ChaseSpeed = 250.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Movement", meta=(ClampMin="0")) float StopDistance = 100.f;
    /** 개발자용 노란 원으로 표시하는 플레이어 주변 생성 반경. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="1")) float PlayerRadius = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="0.1")) float DropInterval = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="1")) float GroundSearchDepth = 500.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="0.1")) float GroundAttackScale = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss|Debug") bool bDrawDebug = true;

    /** BP_P1Boss의 Class Defaults에서 선택하는 공격 준비 몽타주. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Boss|Attack") TObjectPtr<UAnimMontage> DropAttack;
    /** 생성할 기믹 BP 목록. 에셋 선택은 BP_P1Boss에서만 관리한다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Attack") TArray<TSubclassOf<AActor>> AttackActorClasses;
    /** 한 번에 생성할 기믹 개수. 최대 재시도 횟수는 이 값의 10배다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="1", ClampMax="32")) int32 AttackActorCount = 3;
    /** 자체 수명이 없는 기믹에만 적용하는 정리 시간. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Attack", meta=(ClampMin="0.1", Units="s")) float AttackActorLifeSpan = 12.f;
    UFUNCTION(BlueprintCallable, Category="Boss|Attack") void PlayDropObstacles(APawn* TargetPlayer);

    UFUNCTION(BlueprintPure, Category="Boss|Attack") bool IsPreparingAttack() const;
    /** BP Service가 전달한 목표로 이동하고 플레이어를 바라본다. */
    UFUNCTION(BlueprintCallable, Category="Boss|Movement")
    void UpdatePursuit(APawn* TargetPlayer, const FVector& ChaseLocation, float DeltaSeconds);
    /** GameMode BP의 시작/종료 흐름으로만 실행한다. */
    UFUNCTION(BlueprintCallable, Category="Boss|Encounter") void StartEncounter();
    UFUNCTION(BlueprintPure, Category="Boss|Encounter") bool IsEncounterRunning() const { return bRunning; }
    /** 플레이어 위치를 사각형 내부의 추격 목표로 변환한다. */
    UFUNCTION(BlueprintPure, Category="Boss|Movement")
    FVector GetChaseLocation(const FVector& PlayerPosition) const;
    UFUNCTION(BlueprintCallable, Category="Boss|Encounter") void StopEncounter();
protected:
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void PrepareEncounter();
    FVector ConstrainPosition(const FVector& Position) const;
    bool PlayAttackAnim(UAnimMontage* Montage, APawn* TargetPlayer);
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void CancelPreparedAttack();
    void EnforceArenaBounds();
    void DrawArenaDebug() const;
    FVector GetMovementExtent() const;
    UBoxComponent* GetMovementBounds() const;
    void SpawnAttackActors(APawn* TargetPlayer);
    TArray<TSubclassOf<AActor>> GetAttackClasses() const;
    bool FindAttackSpawnLocation(APawn* TargetPlayer, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const;
    TWeakObjectPtr<APawn> PreparedTarget;

    UPROPERTY(Transient) TObjectPtr<UAnimMontage> PreparedMontage;
    bool bRunning = false;
    float BossHeight = 0.f;
    uint8 PreviousMovementMode = 0;
    uint8 PreviousCustomMode = 0;
    TArray<TWeakObjectPtr<AActor>> SpawnedAttackActors;

};
